#include "assembler.h"
#include "codigoIntermedio.h"
#include "tablaSimbolos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASM_LIMIT 100

typedef struct {
  const char *nombre;
  int offset, bloque, activo;
} AsmVariable;

typedef struct {
  Nodo *nodoFuncion;
  int inicioDelaFuncion, finDeLaFuncion, bytes, count;
  AsmVariable variables[ASM_LIMIT];
  int variableALeer[ASM_LIMIT];
  int numRBP[ASM_LIMIT];
} AsmFuncion;

typedef struct {
  AsmFuncion funciones[ASM_LIMIT];
  AsmVariable variablesGlobales[ASM_LIMIT];
  int count, globalCount;
} AsmProgram;

static int asmError(const char *mensaje) {
  fprintf(stderr, "Error assembly: %s\n", mensaje);
  return 0;
}

static void asmMetodos(AsmProgram *programa, Nodo *nodo) {
  if (nodo == NULL) return;
  if (nodo->tipoNodo == N_METODO) {
    programa->funciones[programa->count].nodoFuncion = nodo;
    programa->count++;
  } else if (nodo->tipoNodo == N_SEQ) {
    asmMetodos(programa, nodo->izq);
    asmMetodos(programa, nodo->der);
  }
}

static AsmFuncion *asmBuscarFuncion(AsmProgram *programa, const char *nombre) {
  for (int i = 0; i < programa->count; i++)
    if (strcmp(programa->funciones[i].nodoFuncion->nombre, nombre) == 0)
      return &programa->funciones[i];
  return NULL;
}

static void asmNuevoRegistro(AsmFuncion *funcion, int tTemporal) {
  funcion->bytes += 8;
  funcion->numRBP[tTemporal] = funcion->bytes;
}

static int asmBuscarVariable(AsmProgram *programa, AsmFuncion *funcion,
                             const char *nombre) {
  for (int j = funcion->count - 1; j >= 0; j--)
    if (funcion->variables[j].activo &&
        strcmp(funcion->variables[j].nombre, nombre) == 0)
      return j + 1;
  for (int j = 0; j < programa->globalCount; j++)
    if (strcmp(programa->variablesGlobales[j].nombre, nombre) == 0)
      return -j - 1;
  return 0;
}

static int asmPlan(AsmProgram *programa, Nodo *raiz) {
  asmMetodos(programa, raiz);
  for (int i = 0; i < programa->count; i++) {
    Simbolo *simbolo = programa->funciones[i].nodoFuncion->simbolo;
    if (simbolo->tipoDato == T_FLOAT)
      return asmError("el backend no soporta flotantes");
    for (int j = 0; j < simbolo->cantidadParametros; j++)
      if (simbolo->tipoParametros[j] == T_FLOAT)
        return asmError("el backend no soporta flotantes");
  }

  AsmFuncion *funcion = NULL;
  int bloque = 0;
  for (int i = 0; i < CANTInstrucciones; i++) {
    Instruccion in = instrucciones[i];
    if (in.op == ARRANCO_F) {
      funcion = asmBuscarFuncion(programa, in.nombre);
      funcion->inicioDelaFuncion = i;
      bloque = 0;
      continue;
    }
    if (funcion == NULL) {
      if (in.res == T_FLOAT)
        return asmError("el backend no soporta flotantes");
      programa->variablesGlobales[programa->globalCount++] =
          (AsmVariable){in.nombre, 0, 0, 1};
      continue;
    }

    switch (in.op) {
    case INS_ENTRAR_AMBITO:
      bloque++;
      break;
    case INS_SALIR_AMBITO:
      for (int j = 0; j < funcion->count; j++)
        if (funcion->variables[j].bloque == bloque)
          funcion->variables[j].activo = 0;
      bloque--;
      break;
    case INS_RECIBIR_PARAM:
    case INS_DECLARAR_VAR:
      if ((in.op == INS_RECIBIR_PARAM ? in.arg1 : in.res) == T_FLOAT)
        return asmError("el backend no soporta flotantes");
      funcion->bytes += 8;
      funcion->variables[funcion->count] =
          (AsmVariable){in.nombre, funcion->bytes, bloque, 1};
      funcion->variableALeer[i] = ++funcion->count;
      break;
    case INS_LOAD_MEM:
      funcion->variableALeer[i] = asmBuscarVariable(programa, funcion, in.nombre);
      asmNuevoRegistro(funcion, in.res);
      break;
    case INS_STORE_MEM:
      funcion->variableALeer[i] = asmBuscarVariable(programa, funcion, in.nombre);
      break;
    case INS_CALL:
      if (in.res >= 0) asmNuevoRegistro(funcion, in.res);
      break;
    case TERMINO_F:
      funcion->finDeLaFuncion = i;
      funcion->bytes = (funcion->bytes + 15) & ~15;
      funcion = NULL;
      break;
    case INS_LOAD_FLOAT:
      return asmError("el backend no soporta flotantes");
    case INS_PARAM:
    case INS_RETURN:
    case LABEL:
    case JMP:
    case JMPTrue:
    case JMPFalse:
      break;
    default:
      asmNuevoRegistro(funcion, in.res);
      break;
    }
  }
  return 1;
}

static void asmEmit(FILE *out, AsmProgram *p) {
  const char *gpr[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
  fprintf(out, ".bss\n");
  for (int i = 0; i < p->globalCount; i++) {
    const char *name = p->variablesGlobales[i].nombre;
    fprintf(out, ".balign 8\n");
    fprintf(out, ".globl %s\n", name);
    fprintf(out, "%s:\n", name);
    fprintf(out, ".zero 8\n");
  }
  fprintf(out, ".text\n");
  for (int n = 0; n < p->count; n++) {
    AsmFuncion *f = &p->funciones[n];
    const char *name = f->nodoFuncion->nombre;
    int pending[ASM_LIMIT], pendingCount = 0;

    fprintf(out, ".globl %s\n", name);
    fprintf(out, "%s:\n", name);
    fprintf(out, "\tenter $%d, $0\n", f->bytes);

    for (int i = f->inicioDelaFuncion + 1; i < f->finDeLaFuncion; i++) {
      Instruccion in = instrucciones[i];

      switch (in.op) {
      case INS_ENTRAR_AMBITO:
      case INS_SALIR_AMBITO:
        break;

      case INS_DECLARAR_VAR: {
        int variable = f->variableALeer[i];
        if (variable > 0) {
          int offset = f->variables[variable - 1].offset;
          fprintf(out, "\tmovq $0, -%d(%%rbp)\n", offset);
        } else {
          const char *nombre = p->variablesGlobales[-variable - 1].nombre;
          fprintf(out, "\tmovq $0, %s(%%rip)\n", nombre);
        }
        break;
      }

      case INS_RECIBIR_PARAM: {
        int location = in.res;
        int variable = f->variableALeer[i];
        int offset = f->variables[variable - 1].offset;
        if (location >= 6) {
          fprintf(out, "\tmovq %d(%%rbp), %%r10\n", 16 + 8 * (location - 6));
          fprintf(out, "\tmovq %%r10, -%d(%%rbp)\n", offset);
        } else {
          fprintf(out, "\tmovq %s, -%d(%%rbp)\n", gpr[location], offset);
        }
        break;
      }

      case INS_LOAD_INT:
      case INS_LOAD_BOOL:
        fprintf(out, "\tmovq $%d, -%d(%%rbp)\n", in.arg1, f->numRBP[in.res]);
        break;

      case INS_LOAD_MEM: {
        int variable = f->variableALeer[i];
        if (variable > 0) {
          int offset = f->variables[variable - 1].offset;
          fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", offset);
        } else {
          const char *nombre = p->variablesGlobales[-variable - 1].nombre;
          fprintf(out, "\tmovq %s(%%rip), %%r10\n", nombre);
        }
        fprintf(out, "\tmovq %%r10, -%d(%%rbp)\n", f->numRBP[in.res]);
        break;
      }

      case INS_STORE_MEM: {
        int variable = f->variableALeer[i];
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", f->numRBP[in.arg1]);
        if (variable > 0) {
          int offset = f->variables[variable - 1].offset;
          fprintf(out, "\tmovq %%r10, -%d(%%rbp)\n", offset);
        } else {
          const char *nombre = p->variablesGlobales[-variable - 1].nombre;
          fprintf(out, "\tmovq %%r10, %s(%%rip)\n", nombre);
        }
        break;
      }

      case INS_PARAM:
        pending[pendingCount++] = in.arg1;
        break;

      case INS_CALL: {
        int overflow = pendingCount > 6 ? pendingCount - 6 : 0;
        int area = (overflow * 8 + 15) & ~15;
        if (area) fprintf(out, "\tsubq $%d, %%rsp\n", area);
        for (int j = 0; j < pendingCount; j++) {
          int slot = f->numRBP[pending[j]];
          if (j >= 6) {
            fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", slot);
            fprintf(out, "\tmovq %%r10, %d(%%rsp)\n", (j - 6) * 8);
          } else {
            fprintf(out, "\tmovq -%d(%%rbp), %s\n", slot, gpr[j]);
          }
        }
        if (strcmp(in.nombre, "printInt") == 0)
          fprintf(out, "\tmovq $0, %%rax\n");
        fprintf(out, "\tcall %s\n", in.nombre);
        if (area) fprintf(out, "\taddq $%d, %%rsp\n", area);
        if (in.res >= 0)
          fprintf(out, "\tmovq %%rax, -%d(%%rbp)\n", f->numRBP[in.res]);
        pendingCount = 0;
        break;
      }

      case INS_RETURN:
        if (in.arg1 >= 0)
          fprintf(out, "\tmovq -%d(%%rbp), %%rax\n", f->numRBP[in.arg1]);
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
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", f->numRBP[in.res]);
        fprintf(out, "\tmovq $1, %%r11\n");
        fprintf(out, "\tcmpq %%r11, %%r10\n");
        fprintf(out, "\t%s .Lcg_label_%d\n", in.op == JMPTrue ? "je" : "jne", in.arg1);
        break;

      default: {
        int b = 0;
        if (in.op != INS_NEG_REG && in.op != INS_NOT_REG && in.op != INS_NOTReg)
          b = f->numRBP[in.arg2];
        fprintf(out, "\tmovq -%d(%%rbp), %%r10\n", f->numRBP[in.arg1]);
        const char *cc = NULL;
        switch (in.op) {
        case INS_SUMA_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n", b);
          fprintf(out, "\taddq %%r11, %%r10\n");
          break;
        case INS_RESTA_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n", b);
          fprintf(out, "\tsubq %%r11, %%r10\n");
          break;
        case INS_MUL_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n", b);
          fprintf(out, "\timulq %%r11, %%r10\n");
          break;
        case INS_DIV_REG:
        case INS_MOD_REG:
          fprintf(out, "\tmovq %%r10, %%rax\n");
          fprintf(out, "\tcqto\n");
          fprintf(out, "\tidivq -%d(%%rbp)\n", b);
          fprintf(out, "\tmovq %s, %%r10\n", in.op == INS_MOD_REG ? "%rdx" : "%rax");
          break;
        case INS_NEG_REG:
          fprintf(out, "\tnegq %%r10\n");
          break;
        case INS_NOT_REG:
        case INS_NOTReg:
          fprintf(out, "\tmovq $0, %%r11\n");
          fprintf(out, "\tcmpq %%r11, %%r10\n");
          fprintf(out, "\tmovq $0, %%r11\n");
          fprintf(out, "\tmovq $1, %%r10\n");
          fprintf(out, "\tcmove %%r10, %%r11\n");
          fprintf(out, "\tmovq %%r11, %%r10\n");
          break;
        case INS_AND_REG:
        case INS_OR_REG:
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n", b);
          fprintf(out, "\t%s %%r11, %%r10\n", in.op == INS_AND_REG ? "andq" : "orq");
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
          fprintf(out, "\tmovq -%d(%%rbp), %%r11\n", b);
          fprintf(out, "\tcmpq %%r11, %%r10\n");
          fprintf(out, "\tmovq $0, %%r11\n");
          fprintf(out, "\tmovq $1, %%r10\n");
          fprintf(out, "\tcmov%s %%r10, %%r11\n", cc);
          fprintf(out, "\tmovq %%r11, %%r10\n");
        }
        fprintf(out, "\tmovq %%r10, -%d(%%rbp)\n", f->numRBP[in.res]);
        break;
      }
      }
    }
    fprintf(out, ".Lcg_return_%d:\n", n);
    fprintf(out, "\tleave\n");
    fprintf(out, "\tret\n");
  }
  fprintf(out, ".section .note.GNU-stack,\"\",@progbits\n");
}

int generarAssembly(Nodo *raiz, const char *archivo) {
  AsmProgram *program = calloc(1, sizeof(*program));
  if (program == NULL) return asmError("no se pudo reservar memoria");
  if (!asmPlan(program, raiz)) {
    free(program);
    return 0;
  }
  FILE *out = fopen(archivo, "w");
  if (out == NULL) {
    free(program);
    return asmError("no se pudo abrir el archivo de salida");
  }
  asmEmit(out, program);
  int ok = !ferror(out);
  if (fclose(out) != 0) ok = 0;
  if (!ok) {
    remove(archivo);
    asmError("no se pudo escribir el archivo de salida");
  }
  free(program);
  return ok;
}
