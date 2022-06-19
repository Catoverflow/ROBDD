%{
#include <iostream>
#include <string>
#include "ROBDD.hpp"
int yylex(void);
void yyerror(const char *s);
extern ROBDD *T;
%}

%code requires {
#include "ROBDD.hpp"
}

%token RET LPAREN RPAREN
%token OR AND NOT THEN
%token <var> VAR
%type <node> boolean_expr expr dnf cnf term atom

%start boolean_expr

%union {
    char *var;
    BDD_node *node;
}

%%

boolean_expr
: expr {T->set_root($1); /*T->trim();*/ YYABORT;}

expr
: expr THEN dnf {$$ = T->apply(OP_THEN, $1, $3);}
| dnf {$$ = $1;}

dnf
: dnf OR cnf {$$ = T->apply(OP_OR, $1, $3);}
| cnf {$$ = $1;}

cnf
: cnf AND term {$$ = T->apply(OP_AND, $1, $3);}
| term {$$ = $1;}

term
: NOT atom {$$ = $2; auto tmp = $$->high; $$->high = $$->low; $$->low = tmp;}
| atom {$$ = $1;}

atom
: LPAREN expr RPAREN {$$ = $2;}
| VAR {unsigned int ID = T->get_ID(std::string($1)); $$ = T->make_node(ID, T->zero, T->one);}

%%

void yyerror (const char *s)
{
    std::cout << s;
}