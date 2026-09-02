#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

Nodo *crearNodo(enum TipoNodo tipo, int indiceEnLaTablaSimbolos, char* nombre, int valor, Nodo *izq, Nodo *der, int linea) {
    Nodo *nuevoNodo = (Nodo *)malloc(sizeof(Nodo));
    nuevoNodo->tipoNodo = tipo;
    nuevoNodo->tipoDato = T_ERROR;
    nuevoNodo->indiceEnLaTablaSimbolos = indiceEnLaTablaSimbolos;
    nuevoNodo->nombre = nombre;
    nuevoNodo->valor = valor;
    nuevoNodo->izq = izq;
    nuevoNodo->der = der;
    nuevoNodo->linea = linea;
    return nuevoNodo;
}

Nodo *crearNodoDecl(char *nombre, enum TipoDato tipo, int linea) {
    Nodo *nuevoNodo = crearNodo(N_DECL, -1, nombre, 0, NULL, NULL, linea);
    nuevoNodo->tipoDato = tipo;
    return nuevoNodo;
}

char *nombreNodo(enum TipoNodo t) {
    switch(t) {
        case N_NUM: return "NUM";
        case N_BOOL: return "BOOL";
        case N_ID: return "ID";
        case N_SUMA: return "+";
        case N_MULT: return "*";
        case N_ASIGN: return "=";
        case N_DECL: return "DECL";
        case N_SEQ: return "SEQ";
        default: return "?";
    }
}

void imprimirArbol(Nodo *nodo, int nivel) {
    if (nodo == NULL) return;
    for (int i = 0; i < nivel; i++) printf("  ");
    if (nodo->nombre != NULL)
        printf("%s (%s)\n", nombreNodo(nodo->tipoNodo), nodo->nombre);
    else if (nodo->tipoNodo == N_NUM || nodo->tipoNodo == N_BOOL)
        printf("%s (%d)\n", nombreNodo(nodo->tipoNodo), nodo->valor);
    else
        printf("%s\n", nombreNodo(nodo->tipoNodo));
    imprimirArbol(nodo->izq, nivel + 1);
    imprimirArbol(nodo->der, nivel + 1);
}