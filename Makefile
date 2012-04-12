all:	a.out
lex.yy.c:	spsil.l
		lex spsil.l

y.tab.c:	spsil.y spsil.h	
		yacc -d spsil.y
a.out:		lex.yy.c y.tab.c	
		gcc lex.yy.c y.tab.c -lfl
clean:
	rm -rf *.out *~ y.* lex.*
