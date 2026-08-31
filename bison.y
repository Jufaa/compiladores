%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(char *s);

enum TipoNodo {N_NUM, N_BOOL, N_ID, N_SUMA, N_MULT, N_ASIGN, N_DECL, N_SEQ, N_BLOQUE};
enum TipoDato {T_INT, T_BOOL, T_ERROR};


typedef struct{
    char* nombre;
    enum TipoDato tipoDato;
    int valor;
}TS;

typedef struct Nodo{
    enum TipoNodo tipoNodo;
    enum TipoDato tipoDato;
    int indiceEnLaTablaSimbolos;
    char* nombre;
    int valor;
    struct Nodo *izq;
    struct Nodo *der;
    int linea;
} Nodo;

int CANTSimbolos = 0;
Nodo *raiz = NULL;
TS tablaSimbolos[100];

void agregarSimbolo(enum TipoDato tipo, char *nombre);
enum TipoDato buscarVariable(char *nombre);

Nodo *crearNodo(enum TipoNodo tipo, int indiceEnLaTablaSimbolos, char* nombre,int valor, Nodo *izq, Nodo *der, int linea);
void imprimirArbol(Nodo *nodo, int nivel);
Nodo *crearNodoDecl(char *nombre, enum TipoDato tipo, int linea);

%}


%union {
    int valor;
    char *cadena;
    struct Nodo *nodo;
}

%type <nodo> E T F P DECLARACION ASIGNACION

%token TINT        256
%token TBOOL       257
%token TVOID       258
%token TTRUE       259
%token TFALSE      260
%token TMAIN       261
%token TRETURN     262
%token TASIGNACION 263
%token TSUMA       264
%token TMULTIPLICACION 265
%token TPA         266
%token TPC         267
%token TLLAVEA     268
%token TLLAVEC     269
%token TPUNTOCOMA  270
%token TCOMA       271
%token <cadena> TID         272
%token <valor> TNUM        273   
%token TVACIO      274
%token TBLANCO     276
%token TERROR      277

%%


    P: P DECLARACION     { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); raiz = $$; }
     | P ASIGNACION      { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); raiz = $$; }
     | P E TPUNTOCOMA    { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); raiz = $$; }
     | DECLARACION       { $$ = $1; raiz = $$; }
     | ASIGNACION        { $$ = $1; raiz = $$; }
     | E TPUNTOCOMA      { $$ = $1; raiz = $$; }
     ;
    E: E TSUMA T {$$ = crearNodo(N_SUMA, -1, NULL, 0, $1, $3, yylineno);}
     | T { $$ = $1; }
     ;

    T: T TMULTIPLICACION F {$$ = crearNodo(N_MULT, -1, NULL, 0, $1, $3, yylineno);}
     | F { $$ = $1; }
     ;

    F: TPA E TPC   { $$ = $2; }
     | TNUM        { $$ = crearNodo(N_NUM, -1, NULL, $1, NULL, NULL, yylineno); }
     | TTRUE       { $$ = crearNodo(N_BOOL, -1, NULL, 1, NULL, NULL, yylineno); }
     | TFALSE      { $$ = crearNodo(N_BOOL, -1, NULL, 0, NULL, NULL, yylineno); }
     | TID         { $$ = crearNodo(N_ID, -1, $1, 0, NULL, NULL, yylineno);}
     ;
    DECLARACION: TINT TID TPUNTOCOMA{$$ = crearNodoDecl($2, T_INT, yylineno);}
        | TBOOL TID TPUNTOCOMA{$$ = crearNodoDecl($2, T_BOOL, yylineno);};

    ASIGNACION: TID TASIGNACION E TPUNTOCOMA
      { $$ = crearNodo(N_ASIGN, -1, $1, 0, $3, NULL, yylineno); };
%%

void yyerror(char *s) {
    fprintf(stderr, "Error de sintaxis en la linea %d: %s\n", yylineno, s);
}

void agregarSimbolo(enum TipoDato tipo, char *nombre) {
    if (CANTSimbolos >= 100) {
        printf("Error linea %d: tabla de simbolos llena (maximo 100)\n", yylineno);
        return;
    }
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            printf("Error linea %d: '%s' ya fue declarada\n", yylineno, nombre);
            return;
        }
    }
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipoDato = tipo;
    tablaSimbolos[CANTSimbolos].valor = 0;
    CANTSimbolos++;
}

enum TipoDato buscarVariable(char *nombre) {
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            return tablaSimbolos[i].tipoDato;
        }
    }
    return T_ERROR;
}

Nodo *crearNodo(enum TipoNodo tipo, int indiceEnLaTablaSimbolos, char* nombre,int valor, Nodo *izq, Nodo *der, int linea){
    TipoDato dato = buscarVariabe(nombre);
    if(dat != T_ERROR){
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
    }else{
        return 0; // no se qe se puede devolver un error o algo
    }
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

Nodo *crearNodoDecl(char *nombre, enum TipoDato tipo, int linea){
    Nodo *nuevoNodo = crearNodo(N_DECL, -1, nombre, 0, NULL, NULL, linea);
    nuevoNodo->tipoDato = tipo;
    return nuevoNodo;
}
int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    printf("\n--- ARBOL ---\n");
    imprimirArbol(raiz, 0);
    return 0;
}
