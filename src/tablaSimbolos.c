#include <stdio.h>
#include <string.h>
#include "tablaSimbolos.h"
#include <stdlib.h>
#include <stdbool.h>
#include "ast.h"
#include "semantico.h"

extern int yylineno; 


int CANTSimbolos = 0;
TS *pila = NULL; 
static Simbolo **almacenGlob=NULL;
static int capacidad = 0;

static char *copiarCadena(const char *cadena) {
    char *copia = malloc(strlen(cadena) + 1);
    if (copia != NULL) {
        strcpy(copia, cadena);
    }
    return copia;
}

bool vacia() {
    return pila == NULL;
}

void liberarTabla(void){
    while(pila != NULL){
        TS *aux = pila;
        pila = pila->sig;
        free(aux);
    }
    for(int i = 0; i < CANTSimbolos; i++){
        free(almacenGlob[i].nombre);
        free(almacenGlob[i].tiposParametros);
        free(almacenGlob[i]);
    }
    free(almacenGlob);
    almacenGlob = NULL;
    capacidad = 0;
    CantSimbolos=0;
}

void inicializarTabla() { 
    liberarTabla();
}

void abrirNivel() {
    TS *nuevo = malloc(sizeof(TS));

    nuevo->nivel = (pila == NULL) ? 0 : pila->nivel + 1;
    nuevo->simbolos = NULL;
    nuevo->sig = pila;

    pila = nuevo;
}

void cerrarNivel() {
    if (pila == NULL) {
        printf("Error: no hay niveles abiertos para cerrar\n");
        return;
    }
    TS *nivelActual = pila;
    pila = pila->sig;

    free(nivelActual);
}

int buscarEnNivelActual(char *nombre) {
    if (pila == NULL) {
        return -1;
    }

    Simbolo *simboloActual = pila->simbolos;

    while (simboloActual != NULL) {
        if (strcmp(simboloActual->nombre, nombre) == 0) {
            return simboloActual->indice;
        }
        simboloActual = simboloActual->sig;
    }

    return -1;
}

int buscarSimbolo(char *nombre) {
    TS *nivelActual = pila;

    while (nivelActual != NULL) {
        Simbolo *simboloActual = nivelActual->simbolos;

        while (simboloActual != NULL) {
            if (strcmp(simboloActual->nombre, nombre) == 0) {
                return simboloActual->indice;
            }
            simboloActual = simboloActual->sig;
        }

        nivelActual = nivelActual->sig;
    }

    return -1;
}

int agregarSimbolo(enum Clase clase,enum tipoDato tipo, char *nombre, int linea){
    

    if (pila == NULL) {
        printf("Error: no hay ningun nivel abierto\n");
        return -1;
    }

    if (buscarEnNivelActual(nombre) != -1) {
        printf("Error linea %d: '%s' ya fue declarada\n", linea, nombre);
        return -1;
    }
    
    if (CANTSimbolos >= capacidad) {
        int nuevaCap = (capacidad == 0) ? 10 : capacidad * 2;
        Simbolo **tmp = realloc(almacenGlob, nuevaCap * sizeof(Simbolo *));
        if (tmp == NULL) { fprintf(stderr, "Error: sin memoria\n"); exit(1); }
        almacenGlob = tmp;
        capacidad = nuevaCap;
    }

    Simbolo *nuevo = calloc(1, sizeof(Simbolo));
    if (nuevo == NULL) { fprintf(stderr, "Error: sin memoria\n"); exit(1); }
    nuevo->indice = CANTSimbolos;
    nuevo->clase = clase;
    nuevo->tipoDato = tipo;
    nuevo->nombre = copiarCadena(nombre);
    nuevo->linea = linea;
    nuevo->nivel = pila->nivel;
    nuevo->sig = pila->simbolos;
    pila->simbolos = nuevo;

    almacenGlob[CANTSimbolos] = nuevo;
    return CANTSimbolos++;
}

void setInfoFuncion(int indice, int numeroParametros, enum TipoDato *tiposParametros) {
    Simbolo *simbolo = obtenerSimbolo(indice);
    if (simbolo == NULL) {
        printf("Error: simbolo invalido\n");
        return;
    }
    free(simbolo->tiposParametros);
    simbolo->tiposParametros = NULL;
    simbolo->numeroParametros = numeroParametros;
    if (numeroParametros > 0) {
        simbolo->tiposParametros = malloc(numeroParametros * sizeof(enum TipoDato));
        for (int i = 0; i < numeroParametros; i++) {
            simbolo->tiposParametros[i] = tiposParametros[i];
        }
    }
}

Simbolo *obtenerSimbolo(int indice) {
    if (indice < 0 || indice >= CANTSimbolos) {
        return NULL;
    }
    return almacenGlob[indice];
}

static const char *nombreTipo(enum TipoDato tipo) {
    switch (tipo) {
        case T_INT: return "int";
        case T_FLOAT: return "float";
        case T_STRING: return "string";
        case T_BOOL: return "bool";
        default: return "desconocido";
    }
}

static const char *nombreClase(enum Clase clase) {
    switch (clase) {
        case C_VAR: return "variable";
        case C_PARAM: return "parametro";
        case C_FUNC: return "funcion";
        default: return "?";
    }
}

void imprimirTabla(void) {
    printf("\n--- TABLA DE SIMBOLOS ---\n");

    for (int i = 0; i < CANTSimbolos; i++) {
        Simbolo *s = almacenGlob[i];

        printf("[%d] nivel %d, linea %d: %-5s %s : %s",
               s->indice, s->nivel, s->linea,
               nombreClase(s->clase), s->nombre, nombreTipo(s->tipoDato));

        if (s->clase == C_FUNC) {
            printf("  (");
            for (int j = 0; j < s->numeroParametros; j++) {
                printf("%s%s", j > 0 ? ", " : "", nombreTipo(s->tiposParametros[j]));
            }
            printf(")");
        }
        printf("\n");
    }
}
