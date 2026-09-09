#include <stdio.h>
#include "codigoIntermedio.h"
#include "tablaSimbolos.h"

int CANTInstrucciones = 0;
Instruccion instrucciones[100];

static int CANTRegistros = 0;

static int nuevoRegistro(void) {
    return CANTRegistros++;
}

static void emitir(enum TipoOp op, int res, int arg1, int arg2) {
    if (CANTInstrucciones >= 100) {
        printf("Error: se supero el maximo de instrucciones (100)\n");
        return;
    }
    instrucciones[CANTInstrucciones].op   = op;
    instrucciones[CANTInstrucciones].res  = res;
    instrucciones[CANTInstrucciones].arg1 = arg1;
    instrucciones[CANTInstrucciones].arg2 = arg2;
    CANTInstrucciones++;
}

static int generar(Nodo *nodo) {
    if (nodo == NULL) return -1;

    switch (nodo->tipoNodo) {

        case N_NUM:
        case N_BOOL: {
            int r = nuevoRegistro();
            emitir(INS_LOAD_CONST, r, nodo->valor, 0);
            return r;
        }

        case N_ID: {
            int r = nuevoRegistro();
            emitir(INS_LOAD_MEM, r, nodo->indiceEnLaTablaSimbolos, 0);
            return r;
        }

        case N_SUMA:
        case N_MULT: {
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(nodo->tipoNodo == N_SUMA ? INS_ADD_REG : INS_MUL_REG,
                   r, rIzq, rDer);
            return r;
        }

        case N_ASIGN: {
            int rValor = generar(nodo->izq);
            emitir(INS_STORE_MEM, nodo->indiceEnLaTablaSimbolos, rValor, 0);
            return -1;
        }

        case N_SEQ: {
            generar(nodo->izq);
            generar(nodo->der);
            return -1;
        }

        case N_DECL:
            return -1;

        default:
            printf("Error linea %d: nodo desconocido en generar\n", nodo->linea);
            return -1;
    }
}

void generarCodigo(Nodo *nodo) {
    CANTInstrucciones = 0;
    CANTRegistros = 0;
    generar(nodo);
}

static char *nombreOp(enum TipoOp op) {
    switch (op) {
        case INS_LOAD_CONST: return "Load_Const";
        case INS_LOAD_MEM:   return "Load_Mem";
        case INS_STORE_MEM:  return "Store_Mem";
        case INS_ADD_REG:    return "Add_Reg";
        case INS_MUL_REG:    return "Mul_Reg";
        default:             return "?";
    }
}

static char *nombreVariable(int indice) {
    if (indice >= 0 && indice < CANTSimbolos) return tablaSimbolos[indice].nombre;
    return "?";
}

void imprimirCodigo(void) {
    printf("\n--- CODIGO INTERMEDIO ---\n");
    for (int i = 0; i < CANTInstrucciones; i++) {
        Instruccion ins = instrucciones[i];
        printf("%3d:  %-11s", i, nombreOp(ins.op));
        switch (ins.op) {
            case INS_LOAD_CONST:
                printf("%d, R%d\n", ins.arg1, ins.res);
                break;
            case INS_LOAD_MEM:
                printf("%s, R%d\n", nombreVariable(ins.arg1), ins.res);
                break;
            case INS_STORE_MEM:
                printf("R%d, %s\n", ins.arg1, nombreVariable(ins.res));
                break;
            case INS_ADD_REG:
            case INS_MUL_REG:
                printf("R%d, R%d, R%d\n", ins.arg1, ins.arg2, ins.res);
                break;
            default:
                printf("\n");
                break;
        }
    }
    printf("(%d instrucciones, %d registros)\n", CANTInstrucciones, CANTRegistros);
}
