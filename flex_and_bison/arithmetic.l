%{
#include "arithmetic.tab.h"
//enum tokenType{
//  PLUS,
//  MINUS,
//  MUL,
//  DIV,
//  MOD,
//  NUMBER,
//  NUM_TOKEN
//};
%}

%%
"+" { return PLUS;}
"-" { return MINUS;}
"*" { return MUL;}
"/" { return DIV;}
"%" { return MOD;}
"(" { return LPAREN;}
")" { return RPAREN;}
\n  { return EOL;}
([0-9]+)(\.[0-9]+)? { yylval = strtod(yytext, NULL); return NUMBER; }
[ \n\r\v\t] {}
. { printf("unrecognized char %s\n", yytext);}
%%

/*
int main(int argc, char** argv){
  while(1) {
    enum tokenType ty =yylex();
    printf("tokenType=%d\n", ty);
    if (ty == NUMBER){
      printf("number_value=%lf\n", number_val);
    }
  }
  return 0;
}
*/
