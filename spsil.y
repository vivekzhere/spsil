%{
#include<stdio.h>
#include<stdlib.h>
#include "spsil.h"
%}
%union
{
	struct tree *n;
}
%token NUM OPER1 OPER2 ID END ASG READ PRINT RELOP LOGOP NEGOP IF ELSE THEN ENDIF WHILE DO ENDWHILE RETURN
%type<n> stmtlist stmt
%left LOGOP
%left RELOP  
%left OPER1		// + and -
%left OPER2		// * , / and %
%right NEGOP
%left UMIN		// unary minus
%%
body:		stmtlist					{}
			;
stmtlist:	stmtlist stmt 				{$$=NULL;
										}
			|stmt						{$$=NULL;
										}
			;
stmt:		NUM ';'						{$$=NULL;
										}
			;
%%
int main (void)
{	
	fp=fopen("./sim/sim.asm","w");
	fprintf(fp,"START\n");
	fprintf(fp,"MOV SP,0\n");
	fprintf(fp,"MOV BP,0\n");	
	return yyparse();
}

int yyerror (char *msg) 
{
	return fprintf (stderr, "YACC: %s\n", msg);
}
