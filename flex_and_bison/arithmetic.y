%{
#include <stdio.h>
double ylval = 0.0;
%}

%token PLUS
%token MINUS
%token MUL
%token DIV
%token MOD
%token NUMBER
%token LPAREN
%token RPAREN
%token EOL

%%

calc : /*nothing*/
     | calc expr EOL { printf ("=%d\n", $2);}

expr : expr PLUS factor {$$ = $1 + $3;}
     | expr MINUS factor {$$ = $1 - $3;}
     | factor
     ;

factor : factor MUL item {$$ = $1 * $3;}
       | factor DIV item {$$ = $1 / $3;}
       | factor MOD item {$$ = $1 % $3;}
       | item
       ;

item : NUMBER
     | LPAREN expr RPAREN {$$ = $2;}
     ;
%%

int main(int argc, char** argv) {
  while(1){
    yyparse();
  }
}

void yyerror(char* s) {
  fprintf(stderr, "error:%s\n", s);
}
