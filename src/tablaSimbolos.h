#ifndef TABLASIMBOLOS_H
#define TABLASIMBOLOS_H
#include <stdbool.h>
#include "ast.h"
#include "semantico.h"
#include <stdio.h>

enum TipoSimbolos {V_GLOBAL, V_LOCAL, V_PARAM, V_FUNCION};

typedef struct Simbolo {
    enum TipoSimbolos tipoVariable;
    enum TipoDato tipoDato;
    struct Simbolo *sig;
    char nombre[100];
    int linea;
    int valor;
} Simbolo;

typedef struct TS {
    int nivel;
    Simbolo *simbolos;
    struct TS *sig;
} TS;

extern TS *pila;
extern int CANTSimbolos;

void inicializarTabla();
void abrirNivel();
void cerrarNivel();
Simbolo *agregarSimbolo(enum TipoSimbolos tipoVariable, enum TipoDato tipoDato, char *nombre, int linea);
int buscarSimboloEnUnNivel(char *nombre, int nivel);
bool vacia();
Simbolo *buscarSimbolo(char *nombre);
void imprimirTabla(void);

#endif