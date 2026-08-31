%{
#include <stdio.h>
#include <stdlib.h>
#include "bison.tab.h"
%}


%%
"int"               { return TINT; }
"bool"              { return TBOOL; }
"void"              { return TVOID; }
"true"              { return TTRUE; }
"false"             { return TFALSE; }
"main"              { return TMAIN; }
"return"            { return TRETURN; }
"="                 { return TASIGNACION; }
"+"                 { return TSUMA; }
"*"                 { return TMULTIPLICACION; }
"("                 { return TPA; }
")"                 { return TPC; }
"{"                 { return TLLAVEA; }
"}"                 { return TLLAVEC; }
";"                 { return TPUNTOCOMA; }
","                 { return TCOMA; }
[a-z][a-z0-9]*      { yylval.bloque.cadena = strdup(yytext); return TID; }
[0-9]+              {yylval.bloque.valor = atoi(yytext); return TNUM;}
["\"" "\""]        { }         
[\t]               { }
.                 {  }

%%
