all:	spsil
lex.yy.c:	spsil.l
		lex spsil.l

y.tab.c:	spsil.y spsil.h	
		yacc -d spsil.y
spsil:		lex.yy.c y.tab.c	
		gcc lex.yy.c y.tab.c -lfl -o spsil
clean:
	rm -rf spsil *~ y.* lex.*
