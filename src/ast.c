#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#define MAX_SENTENCIAS 512

Nodo *crearNodo(enum TipoNodo tipo, int indiceEnLaTablaSimbolos, char* nombre, int valor, Nodo *izq, Nodo *der, int linea) {
    Nodo *nuevoNodo = (Nodo *)malloc(sizeof(Nodo));
    nuevoNodo->tipoNodo = tipo;
    nuevoNodo->tipoDato = T_ERROR;
    nuevoNodo->indiceEnLaTablaSimbolos = indiceEnLaTablaSimbolos;
    nuevoNodo->nombre = nombre;
    nuevoNodo->valor = valor;
    nuevoNodo->valorFloat = 0;
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

char *nombreTipo(enum TipoDato t) {
    switch (t) {
        case T_INT:  return "int";
        case T_BOOL: return "boolean";
        case T_FLOAT: return "float";
        case T_VOID: return "void";
        default:     return "error";
    }
}

char *nombreNodo(enum TipoNodo t) {
    switch(t) {
        case N_NUM: return "NUM";
        case N_FLOAT: return "FLOAT";
        case N_BOOL: return "BOOL";
        case N_ID: return "ID";
        case N_SUMA: return "+";
        case N_MULT: return "*";
        case N_ASIGN: return "=";
        case N_DECL: return "DECL";
        case N_SEQ: return "SEQ";
        case N_RESTA: return "-";
        case N_DIV: return "/";
        case N_MOD: return "%";
        case N_COMPARACION: return "==";
        case N_MENOR: return "<";
        case N_MAYOR: return ">";
        case N_AND: return "&&";
        case N_OR: return "||";
        case N_IF: return "IF";
        case N_IFELSE: return "IF-ELSE";
        case N_WHILE: return "WHILE";
        case N_NEG: return "NEG";
        case N_NOT: return "!";
        case N_RETURN: return "RETURN";
        case N_BLOQUE: return "BLOQUE";
        case N_METODO: return "METODO";
        case N_PARAM: return "PARAM";
        case N_LLAMADA: return "LLAMADA";
        default: return "?";
    }
}








// para el imprimir el arbol
static void juntarSentencias(Nodo *nodo, Nodo **lista, int *cant) {
    if (nodo == NULL) return;
    if (nodo->tipoNodo == N_SEQ) {
        juntarSentencias(nodo->izq, lista, cant);
        juntarSentencias(nodo->der, lista, cant);
        return;
    }
    if (*cant < MAX_SENTENCIAS) lista[(*cant)++] = nodo;
}

static void etiquetaNodo(Nodo *nodo) {
    if (nodo->nombre != NULL)
        printf("%s (%s)", nombreNodo(nodo->tipoNodo), nodo->nombre);
    else if (nodo->tipoNodo == N_FLOAT)
        printf("%s (%g)", nombreNodo(nodo->tipoNodo), nodo->valorFloat);
    else if (nodo->tipoNodo == N_NUM || nodo->tipoNodo == N_BOOL)
        printf("%s (%d)", nombreNodo(nodo->tipoNodo), nodo->valor);
    else
        printf("%s", nombreNodo(nodo->tipoNodo));
    if (nodo->tipoDato != T_ERROR)
        printf(" : %s", nombreTipo(nodo->tipoDato));

    printf("\n");
}

static void imprimirRama(Nodo *nodo, const char *prefijo, int esUltimo) {
    if (nodo == NULL) return;

    printf("%s%s", prefijo, esUltimo ? "└── " : "├── ");
    etiquetaNodo(nodo);

    char nuevoPrefijo[1024];
    snprintf(nuevoPrefijo, sizeof(nuevoPrefijo), "%s%s",
             prefijo, esUltimo ? "    " : "│   ");

    Nodo *hijos[2];
    int cant = 0;
    if (nodo->izq != NULL) hijos[cant++] = nodo->izq;
    if (nodo->der != NULL) hijos[cant++] = nodo->der;

    for (int i = 0; i < cant; i++) {
        imprimirRama(hijos[i], nuevoPrefijo, i == cant - 1);
    }
}

void imprimirArbol(Nodo *nodo, int nivel) {
    (void)nivel;
    if (nodo == NULL) return;

    Nodo *sentencias[MAX_SENTENCIAS];
    int cant = 0;
    juntarSentencias(nodo, sentencias, &cant);

    printf("PROGRAMA\n");
    for (int i = 0; i < cant; i++) {
        imprimirRama(sentencias[i], "", i == cant - 1);
    }
}
