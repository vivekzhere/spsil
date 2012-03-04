%{
#include "y.tab.h"
int linecount=1;
struct define
{
	char *name;
	int value;
	struct define *next;
};
struct tree
{
	char nodetype;		/*	+,-,*,/,%,=,<,>
					?-if statement,		I-ireturn,	L-load
					S-store,		w-while,	p-write
					R-register  //value=0-15,20-SP,21-BP,22-IP
					e-double equals,	l-lessthan or equals
					g-greaterthan or equals		!-not equal
					a-AND		o-OR		x-NOT
					c-number,	i-identifier,	s-string
					n-nonterminal
						*/
	char *name;
	int value;
	struct define *entry;
	struct tree *ptr1,*ptr2,*ptr3;
};

void create_node(char, char*, int);	//nodetype,name,value
%}

%%

"alias"			{				
				return ALIAS;
			}

"define"		{
				return DEFINE;
			}

"do"			{
				return DO;
			}
							
"else"			{
				return ELSE;
			}

"endif"			{
				return ENDIF;		
			}
	
"endwhile"		{
				return ENDWHILE;
			}
							
"if"			{
				create_node('?',NULL,-1);
				return IF;
			}	
						
"ireturn"		{
				create_node('I',NULL,-1);
				return IRETURN;
			}

"load"			{
				create_node('L',NULL,-1);
				return LOAD;
			}
			

"store"			{
				create_node('S',NULL,-1);
				return STORE;
			}
				
"then"			{
				return THEN;
			}

"while"			{
				create_node('w',NULL,-1);
				return WHILE;
			}
			

[R]([1][0-5]|[0-9])  	{
				create_node('R',NULL,atoi(yytext+1));
				return REG;
			}
				
"BP"			{
				create_node('R',NULL,20);
				return REG;
			}

"SP"			{
				create_node('R',NULL,21);
				return REG;
			}

"IP"			{
				create_node('R',NULL,22);
				return REG;
			}
						
[-]?[0-9]+		{
				create_node('c',NULL,atoi(yytext));
				return NUM;
			}

(=)			{
				create_node('=',NULL,-1);
				return ASSIGNOP;
			}
			
[-+]			{
				create_node(*yytext,NULL,-1);
				return ARITHOP1;			
			}
			
[*/%]			{
				create_node(*yytext,NULL,-1);
				return ARITHOP2;			
			}
			
[<>]			{
				create_node(*yytext,NULL,-1);
				return RELOP;
			}
			
(==)			{
				create_node('e',NULL,-1);
				return RELOP;
			}
			
(<=)			{
				create_node('l',NULL,-1);
				return RELOP;
			}
			
(>=)			{
				create_node('g',NULL,-1);
				return RELOP;
			}
			
(!=)			{
				create_node('!',NULL,-1);
				return RELOP;
			}
			
"&&"			{
				create_node('a',NULL,-1);
				return LOGOP;
			}

"||"			{
				create_node('o',NULL,-1);
				return LOGOP;
			}
			
"!"			{
				create_node('x',NULL,-1);
				return NOTOP;
			}
		
[a-zA-Z][a-zA-Z0-9]*	{
				create_node('i',yytext,-1);
				return ID;
			}	

[(),;\[\]\{\}&]		{	
				return *yytext;
			}

[/][/].*\n		{
				linecount++;
			}

[\n]			{	
				linecount++;
			}

[ \t]			{
			}

.			{
			}

%%
void create_node(char nodetype, char *name, int value)
{
	yylval.n=malloc(sizeof(struct tree));
	yylval.n->nodetype=nodetype;
	if(name!=NULL)
		yylval.n->name=strdup(name);
	yylval.n->value=value;
	yylval.n->entry=NULL;
	yylval.n->ptr1=NULL;
	yylval.n->ptr2=NULL;
	yylval.n->ptr3=NULL;
}
