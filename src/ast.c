#include <stdio.h>
#include <string.h>
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

#define MAX_SENTENCIAS 512

/* Una cadena de N_SEQ es conceptualmente una LISTA de sentencias, no un arbol
   anidado. La aplanamos para que las 26 sentencias de un programa no queden
   con 26 niveles de sangria. */
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
        printf("%s (%s)\n", nombreNodo(nodo->tipoNodo), nodo->nombre);
    else if (nodo->tipoNodo == N_NUM || nodo->tipoNodo == N_BOOL)
        printf("%s (%d)\n", nombreNodo(nodo->tipoNodo), nodo->valor);
    else
        printf("%s\n", nombreNodo(nodo->tipoNodo));
}

/* El prefijo se va acumulando: una vertical si la rama sigue mas abajo,
   espacios si esa rama ya se cerro. */
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
