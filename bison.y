
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int yylex();
extern FILE *yyin;
void yyerror(char *s);



typedef struct{
    char *tipo;
    char *nombre;
}TS;

int CANTSimbolos = 0;
TS tablaSimbolos[100];
void agregarSimbolo(char *tipo, char *nombre);
char *buscarTipo(char *nombre);


%}

%union {
    int numero;
    char *cadena;
}


%type <cadena> E

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
%token <numero> TNUM        273   
%token TVACIO      274
%token TBLANCO     276
%token TERROR      277

%%


    P: P DECLARACION
     | P ASIGNACION
     | P E TPUNTOCOMA  { printf("Expresion: %s\n", $2); }
     | DECLARACION
     | ASIGNACION
     | E TPUNTOCOMA    { printf("Expresion: %s\n", $1); } ;
    E: E TSUMA E {
        if (strcmp($1, "int") != 0 || strcmp($3, "int") != 0)
            printf("Error: '+' solo funciona con int\n");
        $$ = "int";
    }
        | E TMULTIPLICACION E {
            if (strcmp($1, "int") != 0 || strcmp($3, "int") != 0)
                printf("Error: '*' solo funciona con int\n");
            $$ = "int";
        }
        | TPA E TPC   { $$ = $2; }
        | TNUM         { $$ = "int"; }
        | TTRUE        { $$ = "bool"; }
        | TFALSE       { $$ = "bool"; }
        | TID          { $$ = buscarTipo($1); };

    DECLARACION: TINT TID TPUNTOCOMA{agregarSimbolo("int", $2);}
        | TBOOL TID TPUNTOCOMA{agregarSimbolo("bool", $2);};

    ASIGNACION: TID TASIGNACION E TPUNTOCOMA
    {
      char *tipoVar = buscarTipo($1);
      if (!tipoVar)
          printf("Error: '%s' no declarada\n", $1);
      else if (strcmp(tipoVar, $3) != 0)
          printf("Error: tipo %s esperado, %s recibido\n", tipoVar, $3);
    };
%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

void agregarSimbolo(char *tipo, char *nombre) {
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipo = tipo;
    CANTSimbolos++;
}

char *buscarTipo(char *nombre) {
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            return tablaSimbolos[i].tipo;
        }
    }
    return NULL;
}


int main(int argc, char **argv) {
    ++argv; --argc;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yyparse();
    return 0;
}
