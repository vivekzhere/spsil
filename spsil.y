%{
#include<stdio.h>
#include<stdlib.h>
#include "spsil.h"
%}
%union
{
	struct tree *n;
}
%token ALIAS DEFINE DO ELSE ENDIF ENDWHILE IF IRETURN LOAD  STORE THEN WHILE REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID
%type<n> IF IRETURN LOAD STORE WHILE REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID stmtlist stmt expr ids ifpad whilepad
%left LOGOP
%left RELOP  
%left ARITHOP1		// + and -
%left ARITHOP2		// * , / and %
%right NOTOP		// NOT Operator
%%
body:		definelistpad stmtlist			{
							}
		;

definelistpad:	definelist				{
								add_predefined_constants();
							}
		;
definelist:						{
							}
		|definelist definestmt			{
							}
		;

definestmt:	DEFINE ID NUM ';'			{
								insert_constant($2->name,$3->value);
							}
		;

stmtlist:	stmtlist stmt 				{
								$$=create_nonterm_node("Body",$1,$2);
							}
		|stmt					{
								$$=$1;
							}
		;

stmt:		ids ASSIGNOP expr ';'	 		{
								$$=create_tree($2,$1,$3,NULL);
							}
		|ifpad expr THEN stmtlist ENDIF ';'	{								
								$$=create_tree($1,$2,$4,NULL);
								flag_alias--;
							}								
		|ifpad expr THEN stmtlist
		ELSE stmtlist ENDIF ';'			{	
								$$=create_tree($1,$2,$4,$6);
								flag_alias--;
							}
		|whilepad expr DO stmtlist ENDWHILE ';'	{
								$$=create_tree($1,$2,$4,NULL);
								flag_alias--;
							}
		|ALIAS ID REG ';'			{	
								insert_alias($2->name,$3->value);
								$$=NULL;
							}
		;
				
expr:		expr ARITHOP1 expr			{
								$$=create_tree($2,$1,$3,NULL);
							}
		|expr ARITHOP2 expr			{
								$$=create_tree($2,$1,$3,NULL);
							}
		|expr RELOP expr 			{
								$$=create_tree($2,$1,$3,NULL);
							}
		|expr LOGOP expr			{
								$$=create_tree($2,$1,$3,NULL);
							}
		|NOTOP expr				{
								$$=create_tree($1,$2,NULL,NULL);
							}
		|'('expr')'				{
								$$=$2;
							}
		|NUM					{	
								$$=$1;
							}
		|ids					{
								$$=$1;
							}
		;

ifpad:		IF					{
								flag_alias++;
								$$=$1;
							}
		;

whilepad:	WHILE					{
								flag_alias++;
								$$=$1;
							}
		;
			
ids:		ID					{							
								$$=$1;//substitute_id($1);
							}
		|REG					{
								$$=$1;
							}
		;
%%
int main (void)
{	
	//fp=fopen("./sim/sim.asm","w");
	//fprintf(fp,"START\n");	
	return yyparse();
}

int yyerror (char *msg) 
{
	return fprintf (stderr, "%d: %s\n",linecount,msg);
}
