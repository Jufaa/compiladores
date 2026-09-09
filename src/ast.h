#ifndef AST_H
#define AST_H

enum TipoNodo {N_NUM, N_BOOL, N_ID, N_SUMA, N_MULT, N_ASIGN, N_DECL, N_SEQ};
enum TipoDato {T_INT, T_BOOL, T_ERROR};

typedef struct Nodo {
    enum TipoNodo tipoNodo;
    enum TipoDato tipoDato;
    int indiceEnLaTablaSimbolos;
    char* nombre;
    int valor;
    struct Nodo *izq;
    struct Nodo *der;
    int linea;
} Nodo;

Nodo *crearNodo(enum TipoNodo tipo, int indiceEnLaTablaSimbolos, char* nombre, int valor, Nodo *izq, Nodo *der, int linea);
Nodo *crearNodoDecl(char *nombre, enum TipoDato tipo, int linea);
void imprimirArbol(Nodo *nodo, int nivel);
char *nombreNodo(enum TipoNodo t);
char *nombreTipo(enum TipoDato t);
void resolverNombres(Nodo *nodo);

#endif