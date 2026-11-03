#include "codigoIntermedio.h"
#include "tablaSimbolos.h"
#include <stdio.h>

int CANTInstrucciones = 0;
Instruccion instrucciones[100];
static int errorGeneracion = 0;

static int CANTRegistros = 0;
static int CANTEtiquetas = 0;

static int nuevoRegistro() { return CANTRegistros++; }

static int nuevaEtiqueta() { return CANTEtiquetas++; }
// emitir tendria qe tneer solo 3 operandos.
static void emitir(enum TipoOp op, int res, int arg1, int arg2,
                   float valorFloat, char *nombre) {
  if (CANTInstrucciones >= 100) {
    printf("Error: se supero el maximo de instrucciones (100)\n");
    errorGeneracion = 1;
    return;
  }
  instrucciones[CANTInstrucciones].op = op;
  instrucciones[CANTInstrucciones].res = res;
  instrucciones[CANTInstrucciones].arg1 = arg1;
  instrucciones[CANTInstrucciones].arg2 = arg2;
  instrucciones[CANTInstrucciones].valorFloat = valorFloat;
  instrucciones[CANTInstrucciones].nombre = nombre;
  CANTInstrucciones++;
}
static void recibirParametros(Nodo *nodo, int *posicion) {
  if (nodo == NULL || errorGeneracion) {
    return;
  }

  if (nodo->tipoNodo == N_SEQ) {
    recibirParametros(nodo->izq, posicion);
    recibirParametros(nodo->der, posicion);
    return;
  }

  emitir(INS_RECIBIR_PARAM, (*posicion)++, nodo->tipoDato, 0, 0, nodo->nombre);
}

static int generar(Nodo *nodo) {
  if (nodo == NULL || errorGeneracion)
    return -1;

  switch (nodo->tipoNodo) {
  case N_FLOAT: {
    int r = nuevoRegistro();
    float arg1 = nodo->valorFloat;
    emitir(INS_LOAD_FLOAT, r, 0, 0, arg1, NULL);
    return r;
  }
  case N_NUM: {
    int r = nuevoRegistro();
    int arg1 = nodo->valor;
    emitir(INS_LOAD_INT, r, arg1, 0, 0, NULL);
    return r;
  }
  case N_BOOL: {
    int r = nuevoRegistro();
    int arg1 = nodo->valor;
    emitir(INS_LOAD_BOOL, r, arg1, 0, 0, NULL);
    return r;
  }

  case N_ID: {
    int r = nuevoRegistro();
    char *nombreVariable = nodo->nombre;
    emitir(INS_LOAD_MEM, r, 0, 0, 0, nombreVariable);
    return r;
  }
  case N_SUMA: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_SUMA_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_MULT: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_MUL_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_RESTA: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_RESTA_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_DIV: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_DIV_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_MOD: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_MOD_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_COMPARACION: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_COMP_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_MENOR: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_MENOR_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_MAYOR: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_MAYOR_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_AND: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_AND_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_OR: {
    int rIzq = generar(nodo->izq);
    int rDer = generar(nodo->der);
    int r = nuevoRegistro();
    emitir(INS_OR_REG, r, rIzq, rDer, 0, NULL);
    return r;
  }
  case N_NEG: {
    int rIzq = generar(nodo->izq);
    int r = nuevoRegistro();
    emitir(INS_NEG_REG, r, rIzq, 0, 0, NULL);
    return r;
  }
  case N_NOT: {
    int rIzq = generar(nodo->izq);
    int r = nuevoRegistro();
    emitir(INS_NOT_REG, r, rIzq, 0, 0, NULL);
    return r;
  }

  case N_LLAMADA: {
    int registros[100];
    int cantidad = 0;

    generarArgumentos(nodo->izq, registros, &cantidad);

    if (errorGeneracion) {
      return -1;
    }

    for (int i = 0; i < cantidad; i++) {
      emitir(INS_PARAM, i, registros[i], 0, 0, NULL);
    }

    int resultado = nodo->tipoDato == T_VOID ? -1 : nuevoRegistro();

    emitir(INS_CALL, resultado, cantidad, 0, 0, nodo->nombre);

    return resultado;
  }
  case N_PARAM: {

    return -1;
  }

  case N_METODO: {
    int posicion = 0;

    emitir(ARRANCO_F, 0, 0, 0, 0, nodo->nombre);

    recibirParametros(nodo->izq, &posicion);
    generar(nodo->der);

    if (nodo->tipoDato == T_VOID) {
      emitir(INS_RETURN, 0, -1, 0, 0, NULL);
    }

    emitir(TERMINO_F, 0, 0, 0, 0, nodo->nombre);
    return -1;
  }

  case N_RETURN: {
    int rValor = generar(nodo->izq);
    emitir(INS_RETURN, 0, rValor, 0, 0, NULL);
    return -1;
  }

  case N_ASIGN: {
    int rValor = generar(nodo->izq);
    emitir(INS_STORE_MEM, 0, rValor, 0, 0, nodo->nombre);
    return -1;
  }

  case N_SEQ: {
    generar(nodo->izq);
    generar(nodo->der);
    return -1;
  }

  case N_BLOQUE: {
    emitir(INS_ENTRAR_AMBITO, 0, 0, 0, 0, NULL);

    generar(nodo->izq);
    generar(nodo->der);

    emitir(INS_SALIR_AMBITO, 0, 0, 0, 0, NULL);
    return -1;
  }

  case N_IF: {
    int fin = nuevaEtiqueta();
    int rCond = generar(nodo->izq);
    emitir(JMPFalse, rCond, fin, 0, 0, NULL);
    generar(nodo->der);
    emitir(LABEL, fin, 0, 0, 0, NULL);
    return -1;
  }
  case N_IFELSE: {
    int fin = nuevaEtiqueta();
    int lElse = nuevaEtiqueta();
    int rCond = generar(nodo->izq);
    emitir(JMPFalse, rCond, lElse, 0, 0, NULL);
    generar(nodo->der->izq); // rama true
    emitir(JMP, fin, 0, 0, 0, NULL);
    emitir(LABEL, lElse, 0, 0, 0, NULL);
    generar(nodo->der->der); // rama falsa
    emitir(LABEL, fin, 0, 0, 0, NULL);
    return -1;
  }
  case N_WHILE: {
    int inicio = nuevaEtiqueta();
    int fin = nuevaEtiqueta();
    emitir(LABEL, inicio, 0, 0, 0, NULL);
    int rCondicion = generar(nodo->izq);
    emitir(JMPFalse, rCondicion, fin, 0, 0, NULL);
    generar(nodo->der);
    emitir(JMP, inicio, 0, 0, 0, NULL);
    emitir(LABEL, fin, 0, 0, 0, NULL);
    return -1;
  }
  case N_NOIGUAL:
  case N_MENORIGUAL:
  case N_MAYORIGUAL: {
    int izq = generar(nodo->izq);
    int der = generar(nodo->der);
    int resultado = nuevoRegistro();

    enum TipoOp op;

    if (nodo->tipoNodo == N_NOIGUAL) {
      op = INS_NOIGUAL_REG;
    } else if (nodo->tipoNodo == N_MENORIGUAL) {
      op = INS_MENORIGUAL_REG;
    } else {
      op = INS_MAYORIGUAL_REG;
    }

    emitir(op, resultado, izq, der, 0, NULL);
    return resultado;
  }
  case N_DECL: {
    emitir(INS_DECLARAR_VAR, nodo->tipoDato, 0, 0, 0, nodo->nombre);
    return -1;
  }

  default:
    printf("Error linea %d: nodo desconocido en generar\n", nodo->linea);
    errorGeneracion = 1;
    return -1;
  }
}

void generarArgumentos(Nodo *nodo, int registros[100], int *cantidad) {
  if (nodo == NULL || errorGeneracion) {
    return;
  }

  if (nodo->tipoNodo == N_SEQ) {
    generarArgumentos(nodo->izq, registros, cantidad);
    generarArgumentos(nodo->der, registros, cantidad);
    return;
  }

  if (*cantidad >= 100) {
    printf("Error: demasiados argumentos\n");
    errorGeneracion = 1;
    return;
  }

  int registro = generar(nodo);

  if (registro < 0) {
    printf("Error: argumento sin valor\n");
    errorGeneracion = 1;
    return;
  }

  registros[(*cantidad)++] = registro;
}

int generarCodigo(Nodo *nodo) {
  CANTInstrucciones = 0;
  CANTRegistros = 0;
  CANTEtiquetas = 0;
  errorGeneracion = 0;

  generar(nodo);

  return !errorGeneracion;
}

static const char *nombreOp(enum TipoOp op) {
  switch (op) {
  case LABEL:
    return "LABEL";
  case JMP:
    return "JMP";
  case JMPTrue:
    return "JMPTrue";
  case JMPFalse:
    return "JMPFalse";
  case ARRANCO_F:
    return "ARRANCO_F";
  case TERMINO_F:
    return "TERMINO_F";
  case INS_PARAM:
    return "PARAM";
  case INS_LOAD_FLOAT:
    return "LOAD_FLOAT";
  case INS_LOAD_INT:
    return "LOAD_INT";
  case INS_LOAD_BOOL:
    return "LOAD_BOOL";
  case INS_LOAD_MEM:
    return "LOAD_MEM";
  case INS_STORE_MEM:
    return "STORE_MEM";
  case INS_NOT_REG:
  case INS_NOTReg:
    return "NOT_REG";
  case INS_SUMA_REG:
    return "SUMA_REG";
  case INS_MUL_REG:
    return "MUL_REG";
  case INS_DIV_REG:
    return "DIV_REG";
  case INS_MOD_REG:
    return "MOD_REG";
  case INS_COMP_REG:
    return "COMP_REG";
  case INS_MENOR_REG:
    return "MENOR_REG";
  case INS_MAYOR_REG:
    return "MAYOR_REG";
  case INS_AND_REG:
    return "AND_REG";
  case INS_OR_REG:
    return "OR_REG";
  case INS_NEG_REG:
    return "NEG_REG";
  case INS_CALL:
    return "CALL";
  case INS_RETURN:
    return "RETURN";
  case INS_RESTA_REG:
    return "RESTA_REG";
  case INS_NOIGUAL_REG:
    return "NOIGUAL_REG";
  case INS_MENORIGUAL_REG:
    return "MENORIGUAL_REG";
  case INS_MAYORIGUAL_REG:
    return "MAYORIGUAL_REG";

  case INS_RECIBIR_PARAM:
    return "RECIBIR_PARAM";

  case INS_ENTRAR_AMBITO:
    return "ENTRAR_AMBITO";
  case INS_SALIR_AMBITO:
    return "SALIR_AMBITO";
  case INS_DECLARAR_VAR:
    return "DECLARAR_VAR";
  default:
    return "DESCONOCIDO";
  }
}

void imprimirCodigo(void) {
  printf("\n--- CODIGO INTERMEDIO ---\n");
  for (int i = 0; i < CANTInstrucciones; i++) {
    Instruccion ins = instrucciones[i];
    printf("%3d:  %-11s", i, nombreOp(ins.op));
    switch (ins.op) {
    case LABEL:
      printf("L%d:\n", ins.res);
      break;
    case JMP:
      printf("L%d\n", ins.res);
      break;
    case JMPTrue:
    case JMPFalse:
      printf("R%d, L%d\n", ins.res, ins.arg1);
      break;
    case ARRANCO_F:
    case TERMINO_F:
      printf("%s\n", ins.nombre != NULL ? ins.nombre : "?");
      break;
    case INS_PARAM:
      printf("%d, R%d\n", ins.res, ins.arg1);
      break;

    case INS_CALL:
      printf("%s, %d", ins.nombre, ins.arg1);

      if (ins.res >= 0) {
        printf(" -> R%d", ins.res);
      }

      printf("\n");
      break;
    case INS_LOAD_FLOAT:
      printf("%g, R%d\n", ins.valorFloat, ins.res);
      break;
    case INS_LOAD_INT:
      printf("%d, R%d\n", ins.arg1, ins.res);
      break;
    case INS_LOAD_BOOL:
      printf("%s, R%d\n", ins.arg1 ? "true" : "false", ins.res);
      break;
    case INS_LOAD_MEM:
      printf("%s, R%d\n", ins.nombre != NULL ? ins.nombre : "?", ins.res);
      break;
    case INS_STORE_MEM:
      printf("R%d, %s\n", ins.arg1, ins.nombre != NULL ? ins.nombre : "?");
      break;
    case INS_NOIGUAL_REG:
    case INS_MENORIGUAL_REG:
    case INS_MAYORIGUAL_REG:
    case INS_SUMA_REG:
    case INS_MUL_REG:
    case INS_DIV_REG:
    case INS_MOD_REG:
    case INS_COMP_REG:
    case INS_MENOR_REG:
    case INS_MAYOR_REG:
    case INS_AND_REG:
    case INS_OR_REG:
    case INS_RESTA_REG:
      printf("R%d, R%d -> R%d\n", ins.arg1, ins.arg2, ins.res);
      break;
    case INS_NEG_REG:
    case INS_NOT_REG:
    case INS_NOTReg:
      printf("R%d -> R%d\n", ins.arg1, ins.res);
      break;
    case INS_RETURN:
      if (ins.arg1 < 0) {
        printf("\n");
      } else {
        printf("R%d\n", ins.arg1);
      }
      break;

    case INS_RECIBIR_PARAM:
      printf("%d -> %s, tipo=%d\n", ins.res, ins.nombre, ins.arg1);
      break;
    case INS_ENTRAR_AMBITO:
    case INS_SALIR_AMBITO:
      printf("\n");
      break;

    case INS_DECLARAR_VAR:
      printf("%s, tipo=%d\n", ins.nombre, ins.res);
      break;
    default:
      printf("\n");
      break;
    }
  }
  printf("(%d instrucciones, %d registros)\n", CANTInstrucciones,
         CANTRegistros);
}
