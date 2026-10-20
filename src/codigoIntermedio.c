#include <stdio.h>
#include "codigoIntermedio.h"
#include "tablaSimbolos.h"

int CANTInstrucciones = 0;
Instruccion instrucciones[100];

static int CANTRegistros = 0;
static int CANTEtiquetas = 0;

static int nuevoRegistro() {
    return CANTRegistros++;
}

static int nuevaEtiqueta() {
    return CANTEtiquetas++;
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
        case N_NOIGUAL:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_COMP_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_MENORIGUAL:{
            int rIzq = generar(nodo->izq);
            int rDer = generar(nodo->der);
            int r = nuevoRegistro();
            emitir(INS_MENOR_REG, r, rIzq, rDer, 0, NULL);
            return r;
        }
        case N_MAYORIGUAL:{
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
        
        case N_BLOQUE: {
            generar(nodo->izq); // declaraciones
            generar(nodo->der); // sentencias
            return -1;
        }
        case N_IF:{
            int fin = nuevaEtiqueta(); 
            int rCond = generar(nodo->izq);
            emitir(JMPFalse, rCond, fin, 0, 0, NULL);
            generar(nodo->der);
            emitir(LABEL, fin, 0, 0, 0, NULL);
            return -1;

        }
        case N_IFELSE:{
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
    CANTEtiquetas = 0;
    generar(nodo);
}

static const char *nombreOp(enum TipoOp op) {
    switch (op) {
        case LABEL:          return "LABEL";
        case JMP:            return "JMP";
        case JMPTrue:        return "JMPTrue";
        case JMPFalse:       return "JMPFalse";
        case ARRANCO_F:      return "ARRANCO_F";
        case TERMINO_F:      return "TERMINO_F";
        case INS_PARAM:      return "PARAM";
        case INS_LOAD_FLOAT: return "LOAD_FLOAT";
        case INS_LOAD_INT:   return "LOAD_INT";
        case INS_LOAD_BOOL:  return "LOAD_BOOL";
        case INS_LOAD_MEM:   return "LOAD_MEM";
        case INS_STORE_MEM:  return "STORE_MEM";
        case INS_NOT_REG:
        case INS_NOTReg:     return "NOT_REG";
        case INS_SUMA_REG:   return "SUMA_REG";
        case INS_MUL_REG:    return "MUL_REG";
        case INS_DIV_REG:    return "DIV_REG";
        case INS_MOD_REG:    return "MOD_REG";
        case INS_COMP_REG:   return "COMP_REG";
        case INS_MENOR_REG:  return "MENOR_REG";
        case INS_MAYOR_REG:  return "MAYOR_REG";
        case INS_NOIGUAL_REG:  return "NOIGUAL_REG";
        case INS_MENORIGUAL_REG:  return "MENORIGUAL_REG";
        case INS_MAYORIGUAL_REG:  return "MAYORIGUAL_REG";
        case INS_AND_REG:    return "AND_REG";
        case INS_OR_REG:     return "OR_REG";
        case INS_NEG_REG:    return "NEG_REG";
        case INS_CALL:       return "CALL";
        case INS_RETURN:     return "RETURN";
        case INS_RESTA_REG:  return "RESTA_REG";
        default:             return "DESCONOCIDO";
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
                printf("res=%d, arg1=R%d, nombre=%s\n",
                       ins.res, ins.arg1,
                       ins.nombre != NULL ? ins.nombre : "?");
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
            case INS_SUMA_REG:
            case INS_MUL_REG:
            case INS_DIV_REG:
            case INS_MOD_REG:
            case INS_COMP_REG:
            case INS_MENOR_REG:
            case INS_MAYOR_REG:
            case INS_MENORIGUAL_REG:
            case INS_MAYORIGUAL_REG:
            case INS_NOIGUAL_REG:
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
            case INS_CALL:
                printf("%s -> R%d\n", ins.nombre != NULL ? ins.nombre : "?", ins.res);
                break;
            case INS_RETURN:
                if (ins.arg1 < 0) {
                    printf("\n");
                } else {
                    printf("R%d\n", ins.arg1);
                }
                break;
            default:
                printf("\n");
                break;
        }
    }
    printf("(%d instrucciones, %d registros)\n", CANTInstrucciones, CANTRegistros);
}
