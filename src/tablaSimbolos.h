#ifndef TABLASIMBOLOS_H
#define TABLASIMBOLOS_H
#include "ast.h"

typedef struct {
    char* nombre;
    enum TipoDato tipoDato;
    int valor;
} TS;

extern TS tablaSimbolos[];
extern int CANTSimbolos;

int agregarSimbolo(enum TipoDato tipo, char *nombre, int linea);
int buscarVariable(char *nombre);
void imprimirTabla(void);

#endif