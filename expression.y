%{
#include <iostream>
#include <string>
#include "ROBDD.hpp"
int yylex(void);
void yyerror(const char *s);
extern ROBDD *T;
extern bool frontend_err;
%}

%code requires {
#include "ROBDD.hpp"
}

%token RET LPAREN RPAREN
%token OR AND NOT THEN XNOR XOR
%token <var> VAR
%type <node> boolean_expr expr func term atom
%type <op> binary_op

%start boolean_expr

%union {
    char *var;
    BDD_node *node;
    char op;
}

%%

boolean_expr
: expr {T->set_root($1);} RET {YYABORT;} //ret work as EOF

expr
: expr THEN func {$$ = T->apply(OP_THEN, $1, $3);}
| func {$$ = $1;}

func
: func binary_op term {
    switch($2){
    case '|': $$ = T->apply(OP_OR, $1, $3); break;
    case '&': $$ = T->apply(OP_AND, $1, $3); break;
    case '=': $$ = T->apply(OP_XNOR, $1, $3); break;
    case 'x': $$ = T->apply(OP_XOR, $1, $3); break;
    }
}
| term {$$ = $1;}

term
: NOT atom {$$ = $2; auto tmp = $$->high; $$->high = $$->low; $$->low = tmp;}
| atom {$$ = $1;}

atom
: LPAREN expr RPAREN {$$ = $2;}
| VAR {unsigned int ID = T->get_ID(std::string($1)); $$ = T->make_node(ID, T->get_zero(), T->get_one());}

binary_op
: OR {$$ = '|';}
| AND {$$ = '&';}
| XNOR {$$ = '=';}
| XOR {$$ = 'x';}
%%

void yyerror (const char *s)
{
    std::cout << s << std::endl;
    frontend_err = true;
}