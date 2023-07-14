%{
enum tokenType{
  PLUS = 1,
  MINUS,
  MUL,
  DIV,
  MOD,
  LPAREN,
  RPAREN,
  NUMBER,
  EOL,
  NUM_TOKEN
};
double yylval = 0;
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

int main(int argc, char** argv){
  int ty = 0;
  yy_scan_string("1+2");
  while(ty =yylex()){
    printf("tokenType=%d\n", ty);
    if (ty == NUMBER){
      printf("number_value=%lf\n", yylval);
    }
  }
  return 0;
}
