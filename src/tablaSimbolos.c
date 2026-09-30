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
bool vacia() {
    return pila == NULL;
}

void inicializarTabla() {
    CANTSimbolos = 0;
    pila = NULL;
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

    Simbolo *actual = pila->simbolos;

    while (actual != NULL) {
        Simbolo *aux = actual;
        actual = actual->sig;
        free(aux);
        CANTSimbolos--;
    }

    TS *nivelActual = pila;
    pila = pila->sig;

    free(nivelActual);
}

Simbolo *agregarSimbolo(enum TipoSimbolos tipoVariable,
                        enum TipoDato tipoDato,
                        char *nombre,
                        int linea) {

    if (CANTSimbolos >= 100) {
        printf("Error linea %d: tabla de simbolos llena\n", linea);
        return NULL;
    }

    if (pila == NULL) {
        printf("Error: no hay ningun nivel abierto\n");
        return NULL;
    }

    if (buscarSimboloEnUnNivel(nombre, pila->nivel) == 1) {
        printf("Error linea %d: '%s' ya fue declarada\n",
               linea, nombre);
        return NULL;
    }

    Simbolo *nuevo = malloc(sizeof(Simbolo));

    if (nuevo == NULL) {
        printf("Error: no se pudo reservar memoria\n");
        return NULL;
    }

    nuevo->tipoVariable = tipoVariable;
    nuevo->tipoDato = tipoDato;
    strcpy(nuevo->nombre, nombre);
    nuevo->linea = linea;
    nuevo->valor = 0;

    nuevo->sig = pila->simbolos;
    pila->simbolos = nuevo;

    CANTSimbolos++;

    return nuevo;
}

//1 encontrado - 0 nivel existe pero no esta, -1=nivel noexiste
int buscarSimboloEnUnNivel(char *nombre, int nivel) {
    TS *nivelActual = pila;

    while (nivelActual != NULL) {

        if (nivelActual->nivel == nivel) {

            Simbolo *simboloActual = nivelActual->simbolos;

            while (simboloActual != NULL) {

                if (strcmp(simboloActual->nombre, nombre) == 0) {
                    return 1;
                }

                simboloActual = simboloActual->sig;
            }

            return 0;
        }

        nivelActual = nivelActual->sig;
    }

    return -1;
}

Simbolo *buscarSimbolo(char *nombre) {
    TS *nivelActual = pila;

    while (nivelActual != NULL) {

        Simbolo *simboloActual = nivelActual->simbolos;

        while (simboloActual != NULL) {

            if (strcmp(simboloActual->nombre, nombre) == 0) {
                return simboloActual;
            }

            simboloActual = simboloActual->sig;
        }

        nivelActual = nivelActual->sig;
    }

    return NULL;
}
void imprimirTabla(void) {
    printf("\n--- TABLA DE SIMBOLOS ---\n");

    TS *nivelActual = pila;

    while (nivelActual != NULL) {

        printf("\nNivel %d:\n", nivelActual->nivel);

        Simbolo *simboloActual = nivelActual->simbolos;

        while (simboloActual != NULL) {

            printf("[%d] %-8s : %-4s = %d\n",
                   simboloActual->linea,
                   simboloActual->nombre,
                   nombreTipo(simboloActual->tipoDato),
                   simboloActual->valor);

            simboloActual = simboloActual->sig;
        }

        nivelActual = nivelActual->sig;
    }
}
