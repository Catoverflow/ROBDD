%option noyywrap
%{
#include "expression.tab.h"
%}

%%
\| { yylval.op = OP_OR; return OR; }
& { yylval.op = OP_AND; return AND; }
~ { yylval.op = OP_NOT; return NOT; }
-> { yylval.op = OP_THEN; return THEN; }
\( { return LRPAREN; }
\) { return RRPAREN; }
[a-zA-Z]+[0-9]* { yylval.var = yytext; return VAR; }
\r\n|\n|\r {return RET; }
%%