#include "assembler.h"
#include "codigoIntermedio.h"
#include "tablaSimbolos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Stack-slot backend for Linux x86-64 System V; the TAC remains unchanged. */
#define ASM_LIMIT 100

typedef struct {
  const char *name;
  enum TipoDato type;
  int offset, depth, active;
} AsmVariable;

typedef struct {
  Nodo *method;
  int begin, end, bytes, count;
  AsmVariable vars[ASM_LIMIT];
  int bindings[ASM_LIMIT], slots[ASM_LIMIT];
  enum TipoDato types[ASM_LIMIT];
} AsmFunction;

typedef struct {
  AsmFunction functions[ASM_LIMIT];
  AsmVariable globals[ASM_LIMIT];
  int count, globalCount;
} AsmProgram;

static int asmError(const char *message) {
  fprintf(stderr, "Error assembly: %s\n", message);
  return 0;
}

static void asmMethods(AsmProgram *p, Nodo *node) {
  if (node == NULL) return;
  if (node->tipoNodo == N_METODO) {
    if (p->count < ASM_LIMIT) p->functions[p->count++].method = node;
  } else if (node->tipoNodo == N_SEQ) {
    asmMethods(p, node->izq);
    asmMethods(p, node->der);
  }
}

static AsmFunction *asmFunction(AsmProgram *p, const char *name) {
  for (int i = 0; i < p->count; i++)
    if (strcmp(p->functions[i].method->nombre, name) == 0)
      return &p->functions[i];
  return NULL;
}

static int asmRegister(AsmFunction *f, int r) {
  return r >= 0 && r < ASM_LIMIT && f->types[r] != T_ERROR;
}

static int asmDefine(AsmFunction *f, int r, enum TipoDato type) {
  if (r < 0 || r >= ASM_LIMIT || (type != T_INT && type != T_BOOL) ||
      f->types[r] != T_ERROR) return 0;
  f->bytes += 8;
  f->slots[r] = f->bytes;
  f->types[r] = type;
  return 1;
}

static int asmBinding(AsmProgram *p, AsmFunction *f, const char *name) {
  for (int j = f->count - 1; j >= 0; j--)
    if (f->vars[j].active && strcmp(f->vars[j].name, name) == 0) return j + 1;
  for (int j = 0; j < p->globalCount; j++)
    if (strcmp(p->globals[j].name, name) == 0) return -j - 1;
  return 0;
}

static enum TipoDato asmVariableType(AsmProgram *p, AsmFunction *f, int binding) {
  return binding > 0 ? f->vars[binding - 1].type : p->globals[-binding - 1].type;
}

static int asmTarget(AsmFunction *f, int label) {
  for (int i = f->begin + 1; i < f->end; i++)
    if (instrucciones[i].op == LABEL && instrucciones[i].res == label) return i;
  return -1;
}

/* Reject reachable fall-through; returns jump directly to a shared epilogue. */
static int asmControlFlow(AsmFunction *f) {
  int visited[ASM_LIMIT] = {0}, queue[ASM_LIMIT], head = 0, tail = 0;
  queue[tail++] = f->begin + 1;
  visited[f->begin + 1] = 1;
  while (head < tail) {
    int i = queue[head++];
    if (i == f->end) {
      if (f->method->tipoDato != T_VOID)
        return asmError("funcion no void puede terminar sin devolver un valor");
      continue;
    }
    Instruccion in = instrucciones[i];
    if (in.op == INS_RETURN) continue;
    int next[2] = {i + 1, -1};
    if (in.op == JMP) next[0] = asmTarget(f, in.res);
    if (in.op == JMPTrue || in.op == JMPFalse) next[1] = asmTarget(f, in.arg1);
    for (int k = 0; k < 2; k++) {
      int target = next[k];
      if (target < 0) continue;
      if (!visited[target]) {
        visited[target] = 1;
        queue[tail++] = target;
      }
    }
  }
  return 1;
}

static int asmPlan(AsmProgram *p, Nodo *root) {
  asmMethods(p, root);
  for (int i = 0; i < p->count; i++) {
    Simbolo *signature = p->functions[i].method->simbolo;
    if (signature->tipoDato == T_FLOAT)
      return asmError("el backend no soporta flotantes");
    for (int j = 0; j < signature->cantidadParametros; j++)
      if (signature->tipoParametros[j] == T_FLOAT)
        return asmError("el backend no soporta flotantes");
  }
  AsmFunction *f = NULL;
  int depth = 0, pending[ASM_LIMIT], pendingCount = 0, received = 0;
  for (int i = 0; i < CANTInstrucciones; i++) {
    Instruccion in = instrucciones[i];
    if (in.op == ARRANCO_F) {
      f = asmFunction(p, in.nombre);
      if (f == NULL) return asmError("funcion sin firma");
      f->begin = i;
      depth = pendingCount = received = 0;
      for (int j = 0; j < ASM_LIMIT; j++) f->types[j] = T_ERROR;
      continue;
    }
    if (f == NULL) {
      if (in.op != INS_DECLARAR_VAR || p->globalCount == ASM_LIMIT)
        return asmError("instruccion fuera de una funcion");
      if (in.res == T_FLOAT)
        return asmError("el backend no soporta flotantes");
      p->globals[p->globalCount++] = (AsmVariable){in.nombre, in.res, 0, 0, 1};
      continue;
    }
    if (pendingCount && in.op != INS_PARAM && in.op != INS_CALL)
      return asmError("secuencia de parametros incompleta");
    switch (in.op) {
    case INS_ENTRAR_AMBITO: depth++; break;
    case INS_SALIR_AMBITO:
      if (depth <= 0) return asmError("cierre de ambito invalido");
      for (int j = 0; j < f->count; j++)
        if (f->vars[j].depth == depth) f->vars[j].active = 0;
      depth--;
      break;
    case INS_RECIBIR_PARAM:
    case INS_DECLARAR_VAR: {
      enum TipoDato type = in.op == INS_RECIBIR_PARAM ? in.arg1 : in.res;
      if (type == T_FLOAT)
        return asmError("el backend no soporta flotantes");
      if (f->count == ASM_LIMIT) return asmError("demasiadas variables");
      if (in.op == INS_RECIBIR_PARAM) {
        Simbolo *signature = f->method->simbolo;
        if (in.res != received || received >= signature->cantidadParametros ||
            type != signature->tipoParametros[received++])
          return asmError("firma y parametros de entrada no coinciden");
      }
      f->bytes += 8;
      f->vars[f->count] = (AsmVariable){in.nombre, type, f->bytes, depth, 1};
      f->bindings[i] = ++f->count;
      break;
    }
    case INS_LOAD_MEM:
    case INS_STORE_MEM: {
      int binding = asmBinding(p, f, in.nombre);
      if (binding == 0) return asmError("variable fuera de ambito");
      f->bindings[i] = binding;
      enum TipoDato type = asmVariableType(p, f, binding);
      if (in.op == INS_LOAD_MEM) {
        if (!asmDefine(f, in.res, type)) return asmError("temporal de carga invalido");
      } else if (!asmRegister(f, in.arg1) || f->types[in.arg1] != type)
        return asmError("almacenamiento sin valor compatible");
      break;
    }
    case INS_LOAD_INT:
    case INS_LOAD_BOOL:
      if (!asmDefine(f, in.res, in.op == INS_LOAD_BOOL ? T_BOOL : T_INT))
        return asmError("temporal literal invalido");
      break;
    case INS_LOAD_FLOAT:
      return asmError("el backend no soporta flotantes");
    case INS_PARAM:
      if (in.res != pendingCount || pendingCount == ASM_LIMIT ||
          !asmRegister(f, in.arg1)) return asmError("parametro sin valor");
      pending[pendingCount++] = in.arg1;
      break;
    case INS_CALL: {
      AsmFunction *callee = asmFunction(p, in.nombre);
      if (callee == NULL || in.arg1 != pendingCount ||
          pendingCount != callee->method->simbolo->cantidadParametros)
        return asmError("llamada sin firma compatible");
      for (int j = 0; j < pendingCount; j++)
        if (f->types[pending[j]] != callee->method->simbolo->tipoParametros[j])
          return asmError("tipo de argumento incompatible");
      if (callee->method->tipoDato != T_VOID) {
        if (!asmDefine(f, in.res, callee->method->tipoDato))
          return asmError("resultado de llamada invalido");
      } else if (in.res != -1) return asmError("llamada void con resultado");
      pendingCount = 0;
      break;
    }
    case INS_RETURN:
      if ((f->method->tipoDato == T_VOID && in.arg1 != -1) ||
          (f->method->tipoDato != T_VOID &&
           (!asmRegister(f, in.arg1) || f->types[in.arg1] != f->method->tipoDato)))
        return asmError("retorno sin valor compatible");
      break;
    case LABEL:
      break;
    case JMP: case JMPTrue: case JMPFalse:
      if (in.op != JMP && (!asmRegister(f, in.res) || f->types[in.res] != T_BOOL))
        return asmError("salto sin condicion booleana");
      break;
    case TERMINO_F:
      f->end = i;
      if (depth || pendingCount || received != f->method->simbolo->cantidadParametros)
        return asmError("funcion o ambitos incompletos");
      for (int j = f->begin + 1; j < f->end; j++) {
        Instruccion branch = instrucciones[j];
        int label = branch.op == JMP ? branch.res : branch.arg1;
        if ((branch.op == JMP || branch.op == JMPTrue || branch.op == JMPFalse) &&
            asmTarget(f, label) < 0) return asmError("destino de salto inexistente");
      }
      if (!asmControlFlow(f)) return 0;
      f->bytes = (f->bytes + 15) & ~15;
      f = NULL;
      break;
    default: {
      int unary = in.op == INS_NEG_REG || in.op == INS_NOT_REG || in.op == INS_NOTReg;
      int arithmetic = in.op == INS_SUMA_REG || in.op == INS_RESTA_REG ||
                       in.op == INS_MUL_REG || in.op == INS_DIV_REG || in.op == INS_MOD_REG;
      int logical = in.op == INS_AND_REG || in.op == INS_OR_REG;
      int comparison = in.op == INS_COMP_REG || in.op == INS_NOIGUAL_REG ||
                       in.op == INS_MENOR_REG || in.op == INS_MAYOR_REG ||
                       in.op == INS_MENORIGUAL_REG || in.op == INS_MAYORIGUAL_REG;
      if ((!unary && !arithmetic && !logical && !comparison) ||
          !asmRegister(f, in.arg1) || (!unary && !asmRegister(f, in.arg2)))
        return asmError("operacion con temporal inexistente");
      enum TipoDato type = f->types[in.arg1];
      if ((!unary && type != f->types[in.arg2]) ||
          ((arithmetic || in.op == INS_NEG_REG) && type != T_INT) ||
          ((logical || (unary && in.op != INS_NEG_REG)) && type != T_BOOL))
        return asmError("operandos incompatibles");
      if (!asmDefine(f, in.res, arithmetic || in.op == INS_NEG_REG ? T_INT : T_BOOL))
        return asmError("temporal de resultado invalido");
      break;
    }
    }
  }
  return f == NULL ? 1 : asmError("funcion sin cierre");
}

static void asmMemory(char *buffer, size_t size, AsmProgram *p, AsmFunction *f, int binding) {
  if (binding > 0) snprintf(buffer, size, "-%d(%%rbp)", f->vars[binding - 1].offset);
  else snprintf(buffer, size, "%s(%%rip)", p->globals[-binding - 1].name);
}

static void asmEmit(FILE *out, AsmProgram *p) {
  const char *gpr[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
  fprintf(out, "# Linux x86-64 System V, GNU assembler AT&T syntax\n.bss\n");
  for (int i = 0; i < p->globalCount; i++) {
    const char *name = p->globals[i].name;
    fprintf(out, ".balign 8\n.globl %s\n.type %s, @object\n%s:\n.zero 8\n.size %s, 8\n",
            name, name, name, name);
  }
  fprintf(out, ".text\n");
  for (int n = 0; n < p->count; n++) {
    AsmFunction *f = &p->functions[n];
    const char *name = f->method->nombre;
    int pending[ASM_LIMIT], pendingCount = 0;

    // Prólogo con enter como indica la guía
    fprintf(out, ".globl %s\n.type %s, @function\n%s:\n\tenter $%d, $0\n",
            name, name, name, f->bytes);

    for (int i = f->begin + 1; i < f->end; i++) {
      Instruccion in = instrucciones[i];
      char memory[160];
      int a = asmRegister(f, in.arg1) ? f->slots[in.arg1] : 0;
      int b = asmRegister(f, in.arg2) ? f->slots[in.arg2] : 0;
      int r = asmRegister(f, in.res) ? f->slots[in.res] : 0;

      switch (in.op) {
      case INS_ENTRAR_AMBITO:
      case INS_SALIR_AMBITO:
        break;

      case INS_DECLARAR_VAR:
        asmMemory(memory, sizeof(memory), p, f, f->bindings[i]);
        fprintf(out, "\tmovq $0, %s\n", memory);
        break;

      case INS_RECIBIR_PARAM: {
        int location = in.res;
        asmMemory(memory, sizeof(memory), p, f, f->bindings[i]);
        if (location >= 6) {
          fprintf(out, "\tmovq %d(%%rbp), %%r10\n\tmovq %%r10, %s\n",
                  16 + 8 * (location - 6), memory);
        } else {
          fprintf(out, "\tmovq %s, %s\n", gpr[location], memory);
        }
        break;
      }

      case INS_LOAD_INT:
      case INS_LOAD_BOOL:
        fprintf(out, "\tmovq $%d, -%d(%%rbp)\n", in.arg1, r);
        break;

      case INS_LOAD_MEM:
        asmMemory(memory, sizeof(memory), p, f, f->bindings[i]);
        fprintf(out, "\tmovq %s, %%r10\n\tmovq %%r10, -%d(%%rbp)\n", memory, r);
        break;

      case INS_STORE_MEM:
        asmMemory(memory, sizeof(memory), p, f, f->bindings[i]);
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n\tmovq %%r10, %s\n", a, memory);
        break;

      case INS_PARAM:
        pending[pendingCount++] = in.arg1;
        break;

      case INS_CALL: {
        int overflow = pendingCount > 6 ? pendingCount - 6 : 0;
        int area = (overflow * 8 + 15) & ~15;
        if (area) fprintf(out, "\tsubq $%d, %%rsp\n", area);
        for (int j = 0; j < pendingCount; j++) {
          int slot = f->slots[pending[j]];
          if (j >= 6)
            fprintf(out, "\tmovq -%d(%%rbp), %%r10\n\tmovq %%r10, %d(%%rsp)\n",
                    slot, (j - 6) * 8);
          else
            fprintf(out, "\tmovq -%d(%%rbp), %s\n", slot, gpr[j]);
        }
        if (strcmp(in.nombre, "printInt") == 0)
          fprintf(out, "\tmovq $0, %%rax\n");
        fprintf(out, "\tcall %s\n", in.nombre);
        if (area) fprintf(out, "\taddq $%d, %%rsp\n", area);
        if (in.res >= 0)
          fprintf(out, "\tmovq %%rax, -%d(%%rbp)\n", r);
        pendingCount = 0;
        break;
      }

      case INS_RETURN:
        if (in.arg1 >= 0)
          fprintf(out, "\tmovq -%d(%%rbp), %%rax\n", a);
        else
          fprintf(out, "\tmovq $0, %%rax\n");
        fprintf(out, "\tjmp .Lcg_return_%d\n", n);
        break;

      case LABEL:
        fprintf(out, ".Lcg_label_%d:\n", in.res);
        break;

      case JMP:
        fprintf(out, "\tjmp .Lcg_label_%d\n", in.res);
        break;

      case JMPTrue:
      case JMPFalse:
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n\tmovq $1, %%r11\n\tcmpq %%r11, %%r10\n\t%s .Lcg_label_%d\n",
                r, in.op == JMPTrue ? "je" : "jne", in.arg1);
        break;

      default: {
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", a);
        const char *cc = NULL;
        switch (in.op) {
        case INS_SUMA_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n\taddq %%r11, %%r10\n", b);
          break;
        case INS_RESTA_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n\tsubq %%r11, %%r10\n", b);
          break;
        case INS_MUL_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n\timulq %%r11, %%r10\n", b);
          break;
        case INS_DIV_REG:
        case INS_MOD_REG:
          // idivq divide %rdx:%rax por el divisor; cociente en %rax, resto en %rdx
          fprintf(out, "\tmovq %%r10, %%rax\n\tcqto\n\tidivq -%d(%%rbp)\n", b);
          fprintf(out, "\tmovq %s, %%r10\n", in.op == INS_MOD_REG ? "%rdx" : "%rax");
          break;
        case INS_NEG_REG:
          fprintf(out, "\tnegq %%r10\n");
          break;
        case INS_NOT_REG:
        case INS_NOTReg:
          fprintf(out, "\tmovq $0, %%r11\n\tcmpq %%r11, %%r10\n"
                       "\tmovq $0, %%r11\n\tmovq $1, %%r10\n\tcmove %%r10, %%r11\n"
                       "\tmovq %%r11, %%r10\n");
          break;
        case INS_AND_REG:
        case INS_OR_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n\t%s %%r11, %%r10\n",
                  b, in.op == INS_AND_REG ? "andq" : "orq");
          break;
        case INS_COMP_REG:       cc = "e";  break;
        case INS_NOIGUAL_REG:    cc = "ne"; break;
        case INS_MENOR_REG:      cc = "l";  break;
        case INS_MAYOR_REG:      cc = "g";  break;
        case INS_MENORIGUAL_REG: cc = "le"; break;
        case INS_MAYORIGUAL_REG: cc = "ge"; break;
        default: break;
        }
        if (cc != NULL) {
          // Esquema exacto de la guía con %r10, %r11 y cmov*
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n\tcmpq %%r11, %%r10\n"
                       "\tmovq $0, %%r11\n\tmovq $1, %%r10\n"
                       "\tcmov%s %%r10, %%r11\n\tmovq %%r11, %%r10\n", b, cc);
        }
        fprintf(out, "\tmovq %%r10, -%d(%%rbp)\n", r);
        break;
      }
      }
    }
    fprintf(out, ".Lcg_return_%d:\n\tleave\n\tret\n.size %s, .-%s\n", n, name, name);
  }
  fprintf(out, ".section .note.GNU-stack,\"\",@progbits\n");
}

int generarAssembly(Nodo *root, const char *archivo) {
  AsmProgram *program = calloc(1, sizeof(*program));
  if (program == NULL) return asmError("no se pudo reservar memoria");
  if (!asmPlan(program, root)) { free(program); return 0; }
  size_t size = strlen(archivo) + sizeof(".tmp.XXXXXX");
  char *temporary = malloc(size);
  if (temporary == NULL) { free(program); return asmError("no se pudo reservar memoria"); }
  snprintf(temporary, size, "%s.tmp.XXXXXX", archivo);
  int fd = mkstemp(temporary);
  FILE *out = fd < 0 ? NULL : fdopen(fd, "w");
  int ok = out != NULL;
  if (ok) {
    asmEmit(out, program);
    ok = !ferror(out);
    if (fclose(out) != 0) ok = 0;
    if (ok && rename(temporary, archivo) != 0) ok = 0;
  } else if (fd >= 0) close(fd);
  if (!ok) { unlink(temporary); asmError("no se pudo escribir el archivo de salida"); }
  free(temporary);
  free(program);
  return ok;
}
