all:	spl
lex.yy.c:	spl.l
		lex spl.l

y.tab.c:	spl.y spl.h	
		yacc -d spl.y
spl:		lex.yy.c y.tab.c	
		gcc lex.yy.c y.tab.c -lfl -o spl
clean:
	rm -rf spl *~ y.* lex.*
