%{
#include<stdio.h>
#include<stdlib.h>
#include "spsil.h"
%}
%union
{
	struct tree *n;
}
%token ALIAS DEFINE DO ELSE ENDIF ENDWHILE IF IRETURN LOAD READ STORE THEN WHILE PRINT REG NUM ASG OPER1 OPER2 RELOP LOGOP NEGOP ID STRING
%type<n> IF IRETURN LOAD READ STORE WHILE PRINT REG NUM ASG OPER1 OPER2 RELOP LOGOP NEGOP ID STRING stmtlist stmt expr ids
%left LOGOP
%left RELOP  
%left OPER1		// + and -
%left OPER2		// * , / and %
%right NEGOP
%left UMIN		// unary minus
%%
body:		definelist stmtlist			{}
			;
			
definelist:								{}

			|definelist definestmt		{}
			;

definestmt:	DEFINE STRING NUM ';'		{}
			;

stmtlist:	stmtlist stmt 				{$$=NULL;
										}
			|stmt						{$$=NULL;
										}
			;

stmt:		ids ASG expr ';'	 		{$$=NULL;
										}
			|READ '(' ids ')' ';'		{$$=NULL;
										}
			|PRINT '(' expr ')' ';'		{$$=NULL;
										}			
			|IF expr THEN stmtlist ENDIF ';'					{$$=NULL;
																}
			|IF expr THEN stmtlist ELSE stmtlist ENDIF ';'		{$$=NULL;
																}
			|WHILE expr DO stmtlist ENDWHILE ';'				{$$=NULL;
																}
			|ALIAS REG ID ';'									{$$=NULL;
																}
			;
				
expr:		expr OPER1 expr				{$$=NULL;
										}
			|expr OPER2 expr			{$$=NULL;
										}
			|expr RELOP expr 			{$$=NULL;
										}
			|expr LOGOP expr			{$$=NULL;
										}
			|NEGOP expr					{$$=NULL;
										}
			|'('expr')'					{$$=$2;
										}
			|OPER1 expr	%prec UMIN		{$$=NULL;
										}
			|NUM						{$$=$1;
										}
			|STRING						{$$=NULL;
										}
			|ids						{$$=$1;
										}
			;
			
ids:	ID					{$$=NULL;
							}
		|REG				{$$=NULL;
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
