%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "ts.h"
#include "semantico.h"
#include "interprete.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(char *s);

Nodo *raiz = NULL;

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

int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    printf("\n--- ARBOL ---\n");
    imprimirArbol(raiz, 0);

    printf("\n--- ANALISIS SEMANTICO ---\n");
    resolverNombres(raiz);
    if (hayErrores) {
        printf("Compilacion abortada por errores.\n");
        return 1;
    }
    chequearTipos(raiz);
    if (hayErrores) {
        printf("Compilacion abortada por errores.\n");
        return 1;
    }
    printf("Sin errores semanticos.\n");

    evaluar(raiz);
    imprimirTabla();
    return 0;
}
