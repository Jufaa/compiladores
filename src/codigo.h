#ifndef CODIGO_H
#define CODIGO_H

#include "ast.h"

enum TipoOp {
    INS_LOAD_CONST, INS_LOAD_MEM, INS_STORE_MEM,
    INS_ADD_REG, INS_MUL_REG
};

typedef struct {
    enum TipoOp op;
    int res, arg1, arg2;
} Instruccion;

extern Instruccion instrucciones[];
extern int CANTInstrucciones;

void generarCodigo(Nodo *nodo);
void imprimirCodigo(void);

#endif
