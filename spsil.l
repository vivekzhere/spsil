%{
#include "y.tab.h"
struct define
{
	char *name;
	int value;
	struct define *next;
};
struct tree
{
	char nodetype;		/*	+,-,*,/,%,=,<,>,!
					?-if statement
					c-number,	i-identifier,	r-read,		p-print,
					n-nonterminal,	e-double equals,	l-lessthan or equals
					g-greaterthan or equals		w-while
					b-boolean constants
					a-AND		o-OR		x-NOT
							*/
	char *name;
	int value;
	struct define *entry;
	struct tree *ptr1,*ptr2,*ptr3;
};

void func(int, char, char*, int);
%}

%%
"if"			{
				func(2,'?',NULL,-1);
				return IF;
				}
			
"else"			{
				return ELSE;
				}
			
"then"			{
				return THEN;
				}

"endif"			{
				return ENDIF;		
				}

"while"			{
				func(2,'w',NULL,-1);
				return WHILE;
				}

"do"			{
				return DO;
				}
	
"endwhile"		{
				return ENDWHILE;
				}

			
"read"			{
				func(2,'r',NULL,-1);
				return READ;
				}
			
"write"			{
				func(2,'p',NULL,-1);
				return PRINT;
				}

"AND"			{
				func(1,'a',NULL,-1);
				return LOGOP;
				}

"OR"			{
				func(1,'o',NULL,-1);
				return LOGOP;
				}
			
"NOT"			{
				func(1,'x',NULL,-1);
				return NEGOP;
				}
			
"return"		{
				func(2,'u',NULL,-1);
				return RETURN;
				}

[0-9]+			{
				func(0,'c',NULL,atoi(yytext));
				return NUM;
				}

(=)				{
				func(2,'=',NULL,-1);
				return ASG;
				}
			
[-+]			{
				func(0,*yytext,NULL,-1);
				return OPER1;			
				}
			
[*/%]			{
				func(0,*yytext,NULL,-1);
				return OPER2;			
				}
			
[<>]			{
				func(1,*yytext,NULL,-1);
				return RELOP;
				}
			
(==)			{
				func(1,'e',NULL,-1);
				return RELOP;
				}
			
(<=)			{
				func(1,'l',NULL,-1);
				return RELOP;
				}
			
(>=)			{
				func(1,'g',NULL,-1);
				return RELOP;
				}
			
(!=)			{
				func(1,'!',NULL,-1);
				return RELOP;
				}
			
			
[a-zA-Z][a-zA-Z0-9]*	{
						func(-1,'i',yytext,-1);
						return ID;
						}				
			
[(),;\[\]\{\}&]			{return *yytext;}

[ \t\n]					{}

.						{}


%%
void func(int type,char nodetype, char *name, int value)
{
	/*yylval.n=malloc(sizeof(struct tree));
	yylval.n->type=type;
	yylval.n->nodetype=nodetype;
	if(name!=NULL)
		yylval.n->name=strdup(name);
	yylval.n->value=value;
	yylval.n->Gentry=NULL;
	yylval.n->Lentry=NULL;
	yylval.n->ptr1=NULL;
	yylval.n->ptr2=NULL;
	yylval.n->ptr3=NULL;*/

}
