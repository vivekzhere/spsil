%{
#include<stdio.h>
#include<stdlib.h>
#include "spl.h"
extern FILE *yyin;
%}
%union
{
	struct tree *n;
}
%token ALIAS DEFINE DO ELSE ENDIF ENDWHILE IF IRETURN LOAD  STORE THEN WHILE HALT REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID BREAK CONTINUE CHKPT READ PRINT STRING INLINE
%type<n> IF IRETURN LOAD STORE WHILE HALT REG NUM ASSIGNOP ARITHOP1 ARITHOP2 RELOP LOGOP NOTOP ID stmtlist stmt expr ids ifpad whilepad BREAK CONTINUE CHKPT READ PRINT STRING INLINE
%left LOGOP
%left RELOP  
%left ARITHOP1		// + and -
%left ARITHOP2		// * , / and %
%right NOTOP		// NOT Operator
%left UMIN		// unary minus
%%
body:		definelistpad stmtlist			{
								codegen($2);
								out_linecount++;
								fprintf(fp,"HALT");
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
									insert_constant($2->name,$4->value);
							}
		;

stmtlist:	stmtlist stmt 				{
								$$=create_nonterm_node("Body",$1,$2);
							}
		|stmt					{
								$$=$1;
							}
		;

stmt:		expr ASSIGNOP expr ';'	 		{
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
								flag_break--;
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
		|INLINE STRING ';'			{
							$2->name++;
							int temp=strlen($2->name);
							$2->name[temp-1]='\0';
							$$=create_tree($1,$2,NULL,NULL);
							}
		|CHKPT ';'				{	
							$$=$1;
							}
		|READ ids ';'				{	
								if($2->nodetype!='R')
								{
									printf("\n%d:Invalid operand in read!!\n",linecount);
									exit(0);
								}							
								$$=create_tree($1,$2,NULL,NULL);
							}
		|PRINT expr ';'				{
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
		|NUM					{	
								$$=$1;
							}
		|ids					{
								$$=$1;
							}
		|STRING					{
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
								flag_break++;
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
int main (int argc,char **argv)
{	
	FILE *input_fp;
	char filename[200],ch;
	char op_name[200];
	if(argc < 3)
	{
		printf("Incorrect Usage.\nSee usage manual\n");
		exit(0);
	}
	strcpy(filename,argv[2]);
	expandpath(filename);
	input_fp = fopen(filename,"r");
	if(!input_fp)
	{
		printf("Invalid input file\n");
		return 0;
	}
	yyin = input_fp;
	remfilename(filename);
	strcpy( op_name, filename );
	if(strcmp(argv[1],"--os") == 0)
	{
		strcat(op_name,"os_startup.xsm");
		addrBaseVal = SCRATCHPAD;
	}
	else if(strcmp(argv[1],"--exhandler") == 0)
	{
		strcat(op_name,"exhandler.xsm");
		addrBaseVal = EX_HANDLER;
	}
	else if(strcmp(argv[1],"--int=timer") == 0)
	{
		strcat(op_name,"timer.xsm");
		addrBaseVal = T_INTERRUPT;
	}
	else if(strcmp(argv[1],"--int=1") == 0)
	{
		strcat(op_name,"int1.xsm");
		addrBaseVal = T_INTERRUPT + 1*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=2") == 0)
	{
		strcat(op_name,"int2.xsm");
		addrBaseVal = T_INTERRUPT + 2*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=3") == 0)
	{
		strcat(op_name,"int3.xsm");
		addrBaseVal = T_INTERRUPT + 3*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=4") == 0)
	{
		strcat(op_name,"int4.xsm");
		addrBaseVal = T_INTERRUPT + 4*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=5") == 0)
	{
		strcat(op_name,"int5.xsm");
		addrBaseVal = T_INTERRUPT + 5*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=6") == 0)
	{
		strcat(op_name,"int6.xsm");
		addrBaseVal = T_INTERRUPT + 6*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else if(strcmp(argv[1],"--int=7") == 0)
	{
		strcat(op_name,"int7.xsm");
		addrBaseVal = T_INTERRUPT + 7*PAGE_PER_INTERRUPT*PAGE_SIZE;
	}
	else
	{
		printf("Invalid arguement %s\n", argv[1]);
		fclose(input_fp);
		exit(0);
	}
	fp=fopen(".temp","w");
	out_linecount++;
	fprintf(fp,"START\n");
	yyparse();
	fclose(input_fp);
	fclose(fp);
	input_fp = fopen(".temp","r");
	if(!input_fp)
	{
		printf("Writing compiled code to file failed\n");
		return 0;
	}
	fp = fopen(op_name,"w");
	if(!fp)
	{
		fclose(input_fp);
		printf("Writing compiled code to file failed\n");
		return 0;
	}
	while( ( ch = fgetc(input_fp) ) != EOF )
		fputc(ch, fp);
	fclose(input_fp);
	fclose(fp);	
	return 0;
}
int yyerror (char *msg) 
{
	return fprintf (stderr, "%d: %s\n",linecount,msg);
}
