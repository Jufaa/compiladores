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
static TS *nivelesCerrados = NULL;
bool vacia() {
    return pila == NULL;
}

void inicializarTabla() {
    liberarTabla();
}

static void liberarNiveles(TS *nivel) {
    while (nivel != NULL) {
        TS *siguiente = nivel->sig;
        Simbolo *simbolo = nivel->simbolos;
        while (simbolo != NULL) {
            Simbolo *siguienteSimbolo = simbolo->sig;
            free(simbolo->tipoParametros);
            free(simbolo);
            simbolo = siguienteSimbolo;
        }
        free(nivel);
        nivel = siguiente;
    }
}

void liberarTabla(void) {
    liberarNiveles(pila);
    liberarNiveles(nivelesCerrados);
    CANTSimbolos = 0;
    pila = NULL;
    nivelesCerrados = NULL;
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

    for (Simbolo *actual = pila->simbolos; actual != NULL; actual = actual->sig) {
        CANTSimbolos--;
    }

    TS *nivelActual = pila;
    pila = pila->sig;

    // Closed scopes are invisible, but AST nodes still reference their symbols.
    nivelActual->sig = nivelesCerrados;
    nivelesCerrados = nivelActual;
}

Simbolo *agregarSimbolo(enum TipoSimbolos tipoVariable,
                        enum TipoDato tipoDato,
                        char *nombre,
                        int linea, int cantidadParametros,
                        enum TipoDato *tipoParametros) {

    if (cantidadParametros < 0 ||
        (cantidadParametros > 0 &&
         (tipoVariable != V_FUNCION || tipoParametros == NULL))) {
        printf("Error linea %d: firma de funcion invalida\n", linea);
        return NULL;
    }

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
    nuevo->cantidadParametros = cantidadParametros;
    nuevo->tipoParametros = NULL;
    if (cantidadParametros > 0) {
        nuevo->tipoParametros = malloc(sizeof(enum TipoDato) * cantidadParametros);
        if (nuevo->tipoParametros == NULL) {
            printf("Error: no se pudo reservar memoria para los parametros\n");
            free(nuevo);
            return NULL;
        }
        memcpy(nuevo->tipoParametros, tipoParametros,
               sizeof(enum TipoDato) * cantidadParametros);
    }
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
