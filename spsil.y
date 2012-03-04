%{
#include<stdio.h>
#include<stdlib.h>
#include "spsil.h"
%}
%union
{
	struct tree *n;
}
%token ALIAS DEFINE DO ELSE ENDIF ENDWHILE IF IRETURN LOAD  STORE THEN WHILE PRINT REG NUM ASG ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID
%type<n> IF IRETURN LOAD STORE WHILE PRINT REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID STRING stmtlist stmt expr ids
%left LOGOP
%left RELOP  
%left ARITHOP1		// + and -
%left ARITHOP2		// * , / and %
%right NOTOP		// NOT Operator
%left UMIN		// unary minus
%%
body:		definelist stmtlist			{
							}
		;
			
definelist:						{
							}
		|definelist definestmt			{
							}
		;

definestmt:	DEFINE STRING NUM ';'			{
							}
		;

stmtlist:	stmtlist stmt 				{
								$$=NULL;
							}
		|stmt					{
								$$=NULL;
							}
		;

stmt:		ids ASSIGNOP expr ';'	 		{
								$$=NULL;
							}
		|PRINT '(' expr ')' ';'			{
								$$=NULL;
							}			
		|IF expr THEN stmtlist ENDIF ';'	{
								$$=NULL;
							}								
		|IF expr THEN stmtlist
		ELSE stmtlist ENDIF ';'			{	
								$$=NULL;
							}
		|WHILE expr DO stmtlist ENDWHILE ';'	{
								$$=NULL;
							}
		|ALIAS REG ID ';'			{
								$$=NULL;
							}
			;
				
expr:		expr ARITHOP1 expr			{
								$$=NULL;
							}
		|expr ARITHOP2 expr			{
								$$=NULL;
							}
		|expr RELOP expr 			{
								$$=NULL;
							}
		|expr LOGOP expr			{
								$$=NULL;
							}
		|NOTOP expr				{
								$$=NULL;
							}
		|'('expr')'				{
								$$=$2;
							}
		|ARITHOP1 expr	%prec UMIN		{
								$$=NULL;
							}
		|NUM					{
								$$=$1;
							}
		|ids					{
								$$=$1;
							}
		;
			
ids:		ID					{
								$$=NULL;
							}
		|REG					{
								$$=NULL;
							}
		;
%%
int main (void)
{	
	fp=fopen("./sim/sim.asm","w");
	fprintf(fp,"START\n");	
	return yyparse();
}

int yyerror (char *msg) 
{
	return fprintf (stderr, "YACC: %s\n", msg);
}
