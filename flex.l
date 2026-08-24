%{
#include <stdio.h>
#include <stdlib.h>
extern int yylval;

#define TINT        256
#define TBOOL       257
#define TVOID       258
#define TTRUE       259
#define TFALSE      260
#define TMAIN       261
#define TRETURN     262
#define TASIGNACION 263
#define TSUMA       264
#define TMULTIPLICACION 265
#define TPA         266
#define TPC         267
#define TLLAVEA     268
#define TLLAVEC     269
#define TPUNTOCOMA  270
#define TCOMA       271
#define TID         272
#define TNUM        273
#define TVACIO      274
#define TSALTO      275
#define TBLANCO     276
#define TERROR      277
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
[a-z][a-z0-9]*      { return TID; }
[0-9]+ {
    yylval = atoi(yytext);
    return TNUM;
}
["\"" "\""]        { return TVACIO; }         
[\t]               { return TBLANCO; }
.                 { return TERROR; }

%%
