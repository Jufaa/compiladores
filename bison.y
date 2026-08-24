
%{
#include <stdio.h>
#include <stdlib.h>
extern int yylex();
extern FILE *yyin;
void yyerror(char *s);
%}

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

P: E TPUNTOCOMA TSALTO{ printf("Expresion valida\n"); }
 ;
E: E TSUMA E        { $$ = $1 + $3; printf("Suma detectada\n"); }
 | E TMULTIPLICACION E  { $$ = $1 * $3; printf("Multiplicación detectada\n"); }
 | TPA E TPC        { $$ = $2; printf("Parentesis abierto y cerrado detectado con el numero %d\n ", $2); }
 | TNUM             { $$ = $1; printf("Numero: %d\n", $1); }
 ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    return 0;
}
