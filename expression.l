%option noyywrap
%{
#include "expression.tab.h"
%}

%%
\| { return OR; }
& { return AND; }
~ { return NOT; }
-> { return THEN; }
\( { return LPAREN; }
\) { return RPAREN; }
[a-zA-Z]+[0-9]* { yylval.var = yytext; return VAR; }
\r\n|\n|\r {return RET; }

%%