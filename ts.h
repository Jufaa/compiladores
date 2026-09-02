#ifndef TS_H
#define TS_H
#include "ast.h"

typedef struct {
    char* nombre;
    enum TipoDato tipoDato;
    int valor;
} TS;

void agregarSimbolo(enum TipoDato tipo, char *nombre);
enum TipoDato buscarVariable(char *nombre);

#endif