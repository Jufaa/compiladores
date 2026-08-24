
%{
#include <stdio.h>
#include <stdlib.h>
extern int yylex();
extern FILE *yyin;
void yyerror(char *s);



typedef struct Nodo{
    char *tipo;
    int valor;
    struct Nodo *izq;
    struct Nodo *der;
} Nodo;

%}
%union {
    int numero;
    Nodo *nodo;
}

%token <numero> TNUM

%type <nodo> E

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
%token TID         272
%token TNUM        273
%token TVACIO      274
%token TBLANCO     276
%token TERROR      277
%%

P: E TPUNTOCOMA { printf("Expresion valida\n"); imprimirAST($1, 0); }
 ;
E: E TSUMA E        { $$ = $1 + $3; printf("Suma detectada\n"); imprimirAST(crearNodo("SUMA", 0, $1, $3), 0); }
 | E TMULTIPLICACION E  { $$ = $1 * $3; printf("Multiplicación detectada\n"); imprimirAST(crearNodo("MULT", 0, $1, $3), 0); }
 | TPA E TPC        { $$ = $2; printf("Parentesis abierto y cerrado detectado con el numero %d\n ", $2); imprimirAST(crearNodo("PARENTESIS", 0, $2, NULL), 0); }
 | TNUM             { $$ = $1; printf("Numero: %d\n", $1); imprimirAST(crearNodo("NUM", $1, NULL, NULL), 0); }
 ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

Nodo *crearNodo(char *tipo, int valor, Nodo *izq, Nodo *der) {
    Nodo *n = malloc(sizeof(Nodo));

    n->tipo = tipo;
    n->valor = valor;
    n->izq = izq;
    n->der = der;

    return n;
}


void imprimirAST(Nodo *n, int nivel) {
    if (n == NULL)
        return;

    for (int i = 0; i < nivel; i++)
        printf("  ");

    if (n->tipo == "NUM")
        printf("%d\n", n->valor);
    else
        printf("%s\n", n->tipo);

    imprimirAST(n->izq, nivel + 1);
    imprimirAST(n->der, nivel + 1);
}

int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    return 0;
}
