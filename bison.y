%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int yylex();
extern FILE *yyin;
void yyerror(char *s);
%}
%{
enum TipoNodo {N_NUM, N_BOOL, N_ID, N_SUMA, N_MULT, N_ASIGN, N_DECL, N_SEQ, N_BLOQUE};
enum TipoDato {T_INT, T_BOOL, T_ERROR};

typedef struct{
    char* nombre;
    enum TipoDato tipoDato;
    int valor;
}TS;

typedef struct Nodo{
    enum TipoNodo tipoNodo;
    int indiceEnLaTablaSimbolos;
    char* nombre;
    int valor;
    struct Nodo *izq;
    struct Nodo *der;
} Nodo;


%{

int CANTSimbolos = 0;
TS tablaSimbolos[100];

void agregarSimbolo(enum TipoDato tipo, char *nombre);
enum TipoDato buscarVariable(char *nombre);

Nodo *crearNodo(enum TipoNodo tipo, int valor, Nodo *izq, Nodo *der);
void imprimirArbol(Nodo *nodo, int nivel);

}

%union {
    struct {
        int tipo; // tipo de dato (T_INT, T_BOOL, T_ERROR)
        int valor;
        char *cadena;
        int linea;
    }bloque;
}

%type <bloque> E

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
%token <bloque> TID         272
%token <bloque> TNUM        273   
%token TVACIO      274
%token TBLANCO     276
%token TERROR      277

%%


    P: P DECLARACION
     | P ASIGNACION
     | P E TPUNTOCOMA  { printf("Expresion: tipo=%d valor=%d\n", $2.tipo, $2.valor); }
     | DECLARACION
     | ASIGNACION
     | E TPUNTOCOMA    { printf("Expresion: tipo=%d valor=%d\n", $1.tipo, $1.valor); } ;
    E: E TSUMA E {
        if ($1.tipo == T_INT && $3.tipo == T_INT) {
           $$.tipo = T_INT; $$.valor = $1.valor + $3.valor; 
        } else {
            printf("Error: '+' solo funciona con int\n");
            $$.tipo = T_ERROR; $$.valor = 0;
        }
    }
        | E TMULTIPLICACION E {
            if ($1.tipo == T_INT && $3.tipo == T_INT) {
                $$.tipo = T_INT; $$.valor = $1.valor * $3.valor;  
            } else {
                printf("Error: '*' solo funciona con int\n");
                $$.tipo = T_ERROR; $$.valor = 0;
            }
        }
        | TPA E TPC   { $$ = $2; }
        | TNUM         { $$.tipo = T_INT; $$.valor = $1.valor; }
        | TTRUE        { $$.tipo = T_BOOL; $$.valor = 1; }
        | TFALSE       { $$.tipo = T_BOOL; $$.valor = 0; }
        | TID          { $$.tipo = buscarVariable($1.cadena); $$.valor=0; };

    DECLARACION: TINT TID TPUNTOCOMA{agregarSimbolo(T_INT, $2.cadena);}
        | TBOOL TID TPUNTOCOMA{agregarSimbolo(T_BOOL, $2.cadena);};

    ASIGNACION: TID TASIGNACION E TPUNTOCOMA
    {
      enum TipoDato tipoVar = buscarVariable($1.cadena);
      if (tipoVar == T_ERROR)
          printf("Error: '%s' no declarada\n", $1.cadena);
      else if (tipoVar != $3.tipo)
          printf("Error: tipo %d esperado, %d recibido\n", tipoVar, $3.tipo);
    };
%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

void agregarSimbolo(enum TipoDato tipo, char *nombre) {
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipoDato = tipo;
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

Nodo *crearNodo(enum TipoNodo tipo, int valor, Nodo *izq, Nodo *der) {
    Nodo *nuevoNodo = (Nodo *)malloc(sizeof(Nodo));
    nuevoNodo->tipoNodo = tipo;
    nuevoNodo->valor = valor;
    nuevoNodo->izq = izq;
    nuevoNodo->der = der;
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
    printf("%s: %d\n", nombreNodo(nodo->tipoNodo), nodo->valor);
    imprimirArbol(nodo->izq, nivel + 1);
    imprimirArbol(nodo->der, nivel + 1);
}

int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    return 0;
}
