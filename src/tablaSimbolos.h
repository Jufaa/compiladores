#ifndef TABLASIMBOLOS_H
#define TABLASIMBOLOS_H
#include <stdbool.h>
#include "ast.h"
#include "semantico.h"
typedef struct Simbolo {
    enum TipoDato tipoDato;
    char nombre[100];
    int linea;
    int valor;
    struct Simbolo *sig;
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
int agregarSimbolo(enum TipoDato tipo, char *nombre, int linea);
int buscarVariable(char *nombre, int nivel);
bool vacia();
void imprimirTabla(void);

#endif