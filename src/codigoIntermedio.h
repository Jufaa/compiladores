#ifndef CODIGOINTERMEDIO_H
#define CODIGOINTERMEDIO_H

#include "ast.h"

enum TipoOp {
    LABEL, JMP, JMPTrue, JMPFalse, ARRANCO_F, TERMINO_F, INS_PARAM,
    INS_LOAD_FLOAT, INS_LOAD_INT, INS_LOAD_BOOL, INS_LOAD_MEM, INS_STORE_MEM, INS_NOT_REG,
    INS_SUMA_REG, INS_MUL_REG, INS_DIV_REG, INS_MOD_REG, INS_COMP_REG, INS_MENOR_REG, INS_MAYOR_REG, INS_AND_REG, INS_OR_REG, INS_NEG_REG, INS_NOTReg, INS_CALL, INS_RETURN, INS_RESTA_REG
};

typedef struct {
    enum TipoOp op;
    int res, arg1, arg2;
    float valorFloat;
    char* nombre;
} Instruccion;

extern Instruccion instrucciones[];
extern int CANTInstrucciones;

void generarCodigo(Nodo *nodo);
void imprimirCodigo(void);

#endif


int doble(int x) {
    return x * 2;
}

