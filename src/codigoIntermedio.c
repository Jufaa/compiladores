#include <stdio.h>
#include "codigoIntermedio.h"
#include "tablaSimbolos.h"

int CANTInstrucciones = 0;
Instruccion instrucciones[100];

static int CANTRegistros = 0;

static int nuevoRegistro() {
    return CANTRegistros++;
}

static void emitir(enum TipoOp op, int res, int arg1, int arg2, float valorFloat, char* nombre) {
    if (CANTInstrucciones >= 100) {
        printf("Error: se supero el maximo de instrucciones (100)\n");
        return;
    }
    instrucciones[CANTInstrucciones].op   = op;
    instrucciones[CANTInstrucciones].res  = res;
    instrucciones[CANTInstrucciones].arg1 = arg1;
    instrucciones[CANTInstrucciones].arg2 = arg2;
    instrucciones[CANTInstrucciones].valorFloat = valorFloat;
    instrucciones[CANTInstrucciones].nombre = nombre;
    CANTInstrucciones++;
}

static int generar(Nodo *nodo) {
    if (nodo == NULL) return -1;

    switch (nodo->tipoNodo) {
        case N_FLOAT:{
            int r = nuevoRegistro();
            float arg1 = nodo->valorFloat;
            emitir(INS_LOAD_FLOAT, r, 0, 0, arg1, NULL);
            return r;
        }
        case N_NUM:{
            int r = nuevoRegistro();
            int arg1 = nodo->valor;
            emitir(INS_LOAD_INT, r, arg1, 0, 0,NULL);
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
            char* nombreVariable = nodo->nombre;
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
        case N_MULT:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_MUL_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_RESTA:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_RESTA_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_DIV:
        {
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
        case N_COMPARACION:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_COMP_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_MENOR:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_MENOR_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_MAYOR:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_MAYOR_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_AND:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_AND_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_OR:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_OR_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_NEG:{
            int rIzq = generar(nodo->izq);
            int r = nuevoRegistro();
            emitir(INS_NEG_REG, r, rIzq, 0, 0, NULL);
            return r;
        }
        case N_NOT:{            
            int rIzq = generar(nodo->izq);
            int r = nuevoRegistro();
            emitir(INS_NOT_REG, r, rIzq, 0, 0, NULL);
            return r;

        }

        case N_LLAMADA:{
            int rParam = nuevoRegistro();
            generar(nodo->izq); // parametros
            emitir(INS_PARAM, 0, rParam, 0, 0, nodo->nombre);
            int r = nuevoRegistro(); //call
            emitir(INS_CALL, r, 0, 0, 0, nodo->nombre);
            return r;
        }
        case N_PARAM:{

            emitir(INS_PARAM, nodo->tipoDato, 0, 0, 0, nodo->nombre);
            return -1;
        }
        case N_METODO:{
            emitir(ARRANCO_F, 0, 0, 0, 0, nodo->nombre);
            generar(nodo->izq); // parámetros
            generar(nodo->der); // cuerpo
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
        case INS_SUMA_REG:   return "Suma_Reg";
        case INS_MUL_REG:    return "Mul_Reg";
        case INS_RESTA_REG:  return "Resta_Reg";
        default:             return "?";
    }
}

static char *nombreVariable(char* nombre) {
    if (nombre != "") return buscarSimbolo(nombre)->nombre;
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
            case INS_SUMA_REG:
            case INS_RESTA_REG:
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
