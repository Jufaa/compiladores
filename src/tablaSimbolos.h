#ifndef TABLASIMBOLOS_H
#define TABLASIMBOLOS_H
#include <stdbool.h>
#include "ast.h"
#include "semantico.h"

enum Clase { C_VAR, C_PARAM ,C_FUNC };

typedef struct Simbolo {
    int indice;
    enum Clase clase;
    enum TipoDato tipoDato;
    char *nombre;
    int linea;
    int nivel;
    int valor;
    int numeroParametros;
    enum TipoDato *tiposParametros;
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
void liberarTabla();
void abrirNivel();
void cerrarNivel();
int agregarSimbolo(enum Clase clase, enum TipoDato tipo, char *nombre, int linea);
int buscarVariable(char *nombre, int nivel);
void setInfoFuncion(int indice, int numeroParametros, enum TipoDato *tiposParametros);
int buscarEnNivelActual(char *nombre);
int buscarSimbolo(char *nombre);
Simbolo *obtenerSimbolo(int indice);
bool vacia();
void imprimirTabla(void);

#endif