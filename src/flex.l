%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bison.tab.h"
%}

%option yylineno
%option noyywrap

%%
"//".*              {}
"int"               { return TINT; }
"float"             { return TFLOAT; }
"boolean"              { return TBOOL; }
"void"              { return TVOID; }
"true"              { return TTRUE; }
"false"             { return TFALSE; }
"return"            { return TRETURN; }
"if"                { return TIF; }
"else"              { return TELSE; }
"while"             { return TWHILE; }
"="                 { return TASIGNACION; }
"+"                 { return TSUMA; }
"*"                 { return TMULTIPLICACION; }
"%"                 { return TMODULO; }
"/"                 { return TDIVISION; }
"-"                 { return TRESTA; }
"=="                { return TDOBLEIGUAL; }
"!="                { return TNOIGUAL; }
"<"                 { return TMENOR; }
">"                 { return TMAYOR; }
"<="                { return TMENORIGUAL; }
">="                { return TMAYORIGUAL; }
"!"                { return TNOT; }
"&&"                { return TAND; }
"||"                { return TOR; }
"("                 { return TPA; }
")"                 { return TPC; }
"{"                 { return TLLAVEA; }
"}"                 { return TLLAVEC; }
";"                 { return TPUNTOCOMA; }
","                 { return TCOMA; }
[a-zA-Z][a-zA-Z0-9]*  { yylval.cadena = strdup(yytext); return TID; }
[0-9]+"."[0-9]+     { yylval.valorFloat = atof(yytext); return TFLOATNUM; }
[0-9]+              { yylval.valor = atoi(yytext); return TNUM; }
\/\*([^*]|\*+[^*/])*\*\/      { }
[ \t\r\n]+          { }
.                   { fprintf(stderr, "Error lexico en la linea %d: caracter '%s' no reconocido\n", yylineno, yytext); }

%%
