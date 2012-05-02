%{
#include<stdio.h>
#include<stdlib.h>
#include "spsil.h"
%}
%union
{
	struct tree *n;
}
%token ALIAS DEFINE DO ELSE ENDIF ENDWHILE IF IRETURN LOAD  STORE STRCMP STRCPY THEN WHILE HALT REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID BREAK CONTINUE CHKPT IN OUT SIN SOUT
%type<n> IF IRETURN LOAD STORE STRCMP STRCPY WHILE HALT REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID stmtlist stmt expr ids ifpad whilepad BREAK CONTINUE CHKPT IN OUT SIN SOUT
%left LOGOP
%left RELOP  
%left ARITHOP1		// + and -
%left ARITHOP2		// * , / and %
%right NOTOP		// NOT Operator
%left UMIN		// unary minus
%%
body:		definelistpad stmtlist			{
								codegen($2);
								fprintf(fp,"OVER\n");
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
		|DEFINE ID ARITHOP1 NUM ';'		{
								if($3->nodetype=='-')
									insert_constant($2->name,-1*$4->value);
								else
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

stmt:		STRCPY '(' expr ',' expr ')' ';'	{	
								/*if($3->nodetype!='R' || $5->nodetype!='R')
								{
									printf("\n%d:Invalid operands in strcpy!!\n",linecount);
									exit(0);
								}*/								
								$$=create_tree($1,$3,$5,NULL);
							}	
		|expr ASSIGNOP expr ';'	 		{
								if($1->nodetype=='R' || $1->nodetype=='m')
								{
									$2->value=2;
									$$=create_tree($2,$1,$3,NULL);
								}
								else
								{
									printf("\n%d:Invalid operands in assignment!!\n",linecount);
									exit(0);
								}
							}
		|ifpad expr THEN stmtlist ENDIF ';'	{								
								$$=create_tree($1,$2,$4,NULL);
								pop_alias();
								depth--;
							}								
		|ifpad expr THEN stmtlist
		elsepad stmtlist ENDIF ';'		{	
								$$=create_tree($1,$2,$4,$6);
								pop_alias();
								depth--;
							}
		|whilepad expr DO stmtlist ENDWHILE ';'	{
								$$=create_tree($1,$2,$4,NULL);
								pop_alias();
								depth--;
								flag_break=0;
							}
		|ALIAS ID REG ';'			{	
								push_alias($2->name,$3->value);
								$$=NULL;
							}
		|LOAD '(' expr ',' expr ')' ';'		{
								$$=create_tree($1,$3,$5,NULL);
							}
		|STORE '(' expr ',' expr ')'	';'	{
								$$=create_tree($1,$3,$5,NULL);
							}
		|IRETURN ';'				{
								$$=$1;
							}
		|BREAK ';'				{if(flag_break==0)
							{
								printf("\n%d: break or continue should be used inside while!!\n",linecount);
								exit(0);								
							}
							$$=$1;
							}
		|CONTINUE ';'				{if(flag_break==0)
							{
								printf("\n%d: break or continue should be used inside while!!\n",linecount);
								exit(0);								
							}
							$$=$1;
							}
		|HALT ';'				{	
							$$=$1;
							}
		|CHKPT ';'				{	
							$$=$1;
							}
		|IN ids ';'				{	
								if($2->nodetype!='R')
								{
									printf("\n%d:Invalid operand in IN!!\n",linecount);
									exit(0);
								}							
								$$=create_tree($1,$2,NULL,NULL);
							}
		|OUT expr ';'				{
								$$=create_tree($1,$2,NULL,NULL);
							}
		|SIN ids ';'				{	
								if($2->nodetype!='R')
								{
									printf("\n%d:Invalid operand in SIN!!\n",linecount);
									exit(0);
								}							
								$$=create_tree($1,$2,NULL,NULL);
							}
		|SOUT expr ';'				{
								$$=create_tree($1,$2,NULL,NULL);
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
		|ARITHOP1 NUM	%prec UMIN		{
								if($1->nodetype=='-')
									$2->value=$2->value*-1;
								$$=$2;
							}
		|NOTOP expr				{
								$$=create_tree($1,$2,NULL,NULL);
							}
		|'['expr']'				{
								$$=create_nonterm_node("addr",$2,NULL);
								$$->nodetype='m';
							}					
		|'('expr')'				{
								$$=$2;
							}
		|STRCMP '(' expr ',' expr ')'		{	
								/*if($3->nodetype!='R' || $5->nodetype!='R')
								{
									printf("\n%d:Invalid operands in strcmp!!\n",linecount);
									exit(0);
								}	*/							
								$$=create_tree($1,$3,$5,NULL);
							}
		|NUM					{	
								$$=$1;
							}
		|ids					{
								$$=$1;
							}
		;

ifpad:		IF					{
								depth++;
								$$=$1;
							}
		;
elsepad:	ELSE					{
								pop_alias();
							}
		;

whilepad:	WHILE					{
								depth++;
								flag_break=1;
								$$=$1;
							}
		;
			
ids:		ID					{							
								$$=substitute_id($1);
							}
		|REG					{
								$$=$1;
							}
		;
%%
int main (void)
{	
	fp=fopen("spcode.esim","w");
	 fprintf(fp,"START\n");
	return yyparse();
}

int yyerror (char *msg) 
{
	return fprintf (stderr, "%d: %s\n",linecount,msg);
}
