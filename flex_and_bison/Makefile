arithmetic: arithmetic.l arithmetic.y
	bison -d arithmetic.y
	flex -o arithmetic_lex.c arithmetic.l
	gcc -lfl arithmetic_lex.c arithmetic.tab.c -o arithmetic
