#ifndef AST_H
#define AST_H

enum TipoNodo {
    N_NUM, N_FLOAT, N_BOOL, N_ID,
    N_SUMA, N_MULT, N_RESTA, N_DIV, N_MOD,
    N_COMPARACION, N_MENOR, N_MAYOR,
    N_AND, N_OR,
    N_NEG, N_NOT,
    N_ASIGN, N_DECL, N_SEQ,
    N_IF, N_IFELSE, N_WHILE,
    N_RETURN, N_BLOQUE,
    N_METODO, N_PARAM, N_LLAMADA
};

enum TipoDato {T_INT, T_BOOL, T_FLOAT, T_ERROR, T_VOID};

typedef struct Nodo {
    enum TipoNodo tipoNodo;
    enum TipoDato tipoDato;
    int indiceEnLaTablaSimbolos;
    char* nombre;
    int valor;
    float valorFloat; //TODO: q onda aca
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