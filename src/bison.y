%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(char *s);
enum TipoDato tipoActual;

Nodo *raiz = NULL;

%}


%union {
    int valor;
    float valorFloat;
    char *cadena;
    struct Nodo *nodo;
}

%token TINT TFLOAT TBOOL
%token TVOID TMAIN TRETURN
%token TTRUE TFALSE
%token TIF TELSE TWHILE
%token TASIGNACION TPUNTOCOMA TCOMA
%token TPA TPC TLLAVEA TLLAVEC
%token TSUMA TRESTA TMULTIPLICACION TDIVISION TMODULO
%token TDOBLEIGUAL TNOIGUAL TMENOR TMAYOR TMENORIGUAL TMAYORIGUAL
%token TAND TOR TNOT
%token <cadena> TID
%token <valor>  TNUM
%token <valorFloat> TFLOATNUM

%type <valor> TYPE
%type <nodo> VAR_DECLS VAR_DECL LISTA_IDS METHOD_DECLS METHOD_DECL PARAMS LISTA_PARAMS
%type <nodo> EXPR LITERAL METHOD_CALL ARGS LISTA_ARGS STATEMENTS STATEMENT SENTENCE BUCLE RETURNS BLOCK

%left TOR
%left TAND
%left TDOBLEIGUAL
%left TMENOR TMAYOR
%left TSUMA TRESTA
%left TMULTIPLICACION TDIVISION TMODULO
%precedence TNOT UMINUS

%%
    PROGRAMA: VAR_DECLS METHOD_DECLS
          { raiz = crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); }
    ;

    VAR_DECLS: %empty              { $$ = NULL; }
            | VAR_DECLS VAR_DECL  { $$ = ($1 == NULL) ? $2 : crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); }
    ;
    
    VAR_DECL: TYPE LISTA_IDS TPUNTOCOMA  { $$ = $2; }
    ;

    LISTA_IDS: TID                  { $$ = crearNodoDecl($1, tipoActual, yylineno); }
            | LISTA_IDS TCOMA TID  { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, crearNodoDecl($3, tipoActual, yylineno), yylineno); }
    ;


    METHOD_DECLS: METHOD_DECL               { $$ = $1; }
                | METHOD_DECLS METHOD_DECL  { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); }
    ;
    METHOD_DECL: TYPE TID TPA PARAMS TPC BLOCK { $$ = crearNodo(N_METODO, -1, $2, 0, $4, $6, yylineno); $$->tipoDato = $1; }
            | TVOID TID TPA PARAMS TPC BLOCK { $$ = crearNodo(N_METODO, -1, $2, 0, $4, $6, yylineno); $$->tipoDato = T_VOID; }
    ;

    PARAMS: %empty        { $$ = NULL; }
        | LISTA_PARAMS  { $$ = $1; }
    ;

    LISTA_PARAMS: TYPE TID
                    { $$ = crearNodo(N_PARAM, -1, $2, 0, NULL, NULL, yylineno); $$->tipoDato = $1; }
                | LISTA_PARAMS TCOMA TYPE TID
                    { Nodo *p = crearNodo(N_PARAM, -1, $4, 0, NULL, NULL, yylineno);
                    p->tipoDato = $3;
                    $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, p, yylineno); }
    ;

    BLOCK: TLLAVEA VAR_DECLS STATEMENTS TLLAVEC
            { $$ = crearNodo(N_BLOQUE, -1, NULL, 0, $2, $3, yylineno); }
    ;


    TYPE: TINT    { tipoActual = T_INT;   $$ = T_INT; }
        | TBOOL   { tipoActual = T_BOOL;  $$ = T_BOOL; }
        | TFLOAT  { tipoActual = T_FLOAT; $$ = T_FLOAT; }
    ;

    STATEMENTS: %empty                { $$ = NULL; }
            | STATEMENTS STATEMENT  { $$ = ($1 == NULL) ? $2 : crearNodo(N_SEQ, -1, NULL, 0, $1, $2, yylineno); }
    ;

    STATEMENT: TID TASIGNACION EXPR TPUNTOCOMA  { $$ = crearNodo(N_ASIGN, -1, $1, 0, $3, NULL, yylineno); }
            | METHOD_CALL TPUNTOCOMA           { $$ = $1; }
            | SENTENCE                         { $$ = $1; }
            | BUCLE                            { $$ = $1; }
            | RETURNS                          { $$ = $1; }
            | TPUNTOCOMA                       { $$ = NULL; }
            | BLOCK                            { $$ = $1; }
    ;

    SENTENCE: TIF TPA EXPR TPC BLOCK
                { $$ = crearNodo(N_IF, -1, NULL, 0, $3, $5, yylineno); }
            | TIF TPA EXPR TPC BLOCK TELSE BLOCK
                { Nodo *ramas = crearNodo(N_SEQ, -1, NULL, 0, $5, $7, yylineno);
                $$ = crearNodo(N_IFELSE, -1, NULL, 0, $3, ramas, yylineno); }
    ;


    RETURNS: TRETURN EXPR TPUNTOCOMA  { $$ = crearNodo(N_RETURN, -1, NULL, 0, $2, NULL, yylineno); }
        | TRETURN TPUNTOCOMA       { $$ = crearNodo(N_RETURN, -1, NULL, 0, NULL, NULL, yylineno); }
    ;

    BUCLE: TWHILE TPA EXPR TPC BLOCK  { $$ = crearNodo(N_WHILE, -1, NULL, 0, $3, $5, yylineno); }
    ;
    METHOD_CALL: TID TPA ARGS TPC  { $$ = crearNodo(N_LLAMADA, -1, $1, 0, $3, NULL, yylineno); }
    ;

    ARGS: %empty  {$$ = NULL;}
        | LISTA_ARGS {$$ = $1;}
    ;

    LISTA_ARGS: EXPR {$$ = $1;}
              | LISTA_ARGS TCOMA EXPR  { $$ = crearNodo(N_SEQ, -1, NULL, 0, $1, $3, yylineno); }
    ;

    EXPR: TID                       { $$ = crearNodo(N_ID, -1, $1, 0, NULL, NULL, yylineno); }
        | METHOD_CALL               { $$ = $1; }
        | LITERAL                   { $$ = $1; }
        | EXPR TSUMA EXPR           { $$ = crearNodo(N_SUMA, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TRESTA EXPR          { $$ = crearNodo(N_RESTA, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TMULTIPLICACION EXPR { $$ = crearNodo(N_MULT, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TDIVISION EXPR       { $$ = crearNodo(N_DIV, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TMODULO EXPR         { $$ = crearNodo(N_MOD, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TMENOR EXPR          { $$ = crearNodo(N_MENOR, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TMAYOR EXPR          { $$ = crearNodo(N_MAYOR, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TDOBLEIGUAL EXPR     { $$ = crearNodo(N_COMPARACION, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TAND EXPR            { $$ = crearNodo(N_AND, -1, NULL, 0, $1, $3, yylineno); }
        | EXPR TOR EXPR             { $$ = crearNodo(N_OR, -1, NULL, 0, $1, $3, yylineno); }
        | TRESTA EXPR %prec UMINUS  { $$ = crearNodo(N_NEG, -1, NULL, 0, $2, NULL, yylineno); }
        | TNOT EXPR                 { $$ = crearNodo(N_NOT, -1, NULL, 0, $2, NULL, yylineno); }
        | TPA EXPR TPC              { $$ = $2; }
    ;


    LITERAL: TNUM      { $$ = crearNodo(N_NUM, -1, NULL, $1, NULL, NULL, yylineno); }
           | TFLOATNUM { $$ = crearNodo(N_FLOAT, -1, NULL, 0, NULL, NULL, yylineno); $$->valorFloat = $1; }
           | TTRUE     { $$ = crearNodo(N_BOOL, -1, NULL, 1, NULL, NULL, yylineno); }
           | TFALSE    { $$ = crearNodo(N_BOOL, -1, NULL, 0, NULL, NULL, yylineno); }
    ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error de sintaxis en la linea %d: %s\n", yylineno, s);
}
