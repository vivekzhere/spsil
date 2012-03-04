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
					r-read,		S-store,		w-while,	p-write
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

void func(int, char, char*, int);
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
				makeNode(2,'?',NULL,-1);
				return IF;
			}	
						
"ireturn"		{
				makeNode(2,'I',NULL,-1);
				return IRETURN;
			}

"load"			{
				makeNode(2,'L',NULL,-1);
				return LOAD;
			}
			

"store"			{
				makeNode(2,'S',NULL,-1);
				return STORE;
			}
				
"then"			{
				return THEN;
			}

"while"			{
				makeNode(2,'w',NULL,-1);
				return WHILE;
			}
			

"R0"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R1"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R2"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R3"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R4"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R5"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R6"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R7"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R8"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
					
"R9"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R10"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R11"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R12"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R13"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R14"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
"R15"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
				
"BP"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}

"SP"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}

"IP"			{
				makeNode(0,'R',NULL,1);
				return REG;
			}
						
[0-9]+			{
				makeNode(0,'c',NULL,atoi(yytext));
				return NUM;
			}

(=)			{
				makeNode(2,'=',NULL,-1);
				return ASSIGNOP;
			}
			
[-+]			{
				makeNode(0,*yytext,NULL,-1);
				return ARITHOP1;			
			}
			
[*/%]			{
				makeNode(0,*yytext,NULL,-1);
				return ARITHOP2;			
			}
			
[<>]			{
				makeNode(1,*yytext,NULL,-1);
				return RELOP;
			}
			
(==)			{
				makeNode(1,'e',NULL,-1);
				return RELOP;
			}
			
(<=)			{
				makeNode(1,'l',NULL,-1);
				return RELOP;
			}
			
(>=)			{
				makeNode(1,'g',NULL,-1);
				return RELOP;
			}
			
(!=)			{
				makeNode(1,'!',NULL,-1);
				return RELOP;
			}
			
"&&"			{
				makeNode(1,'a',NULL,-1);
				return LOGOP;
			}

"||"			{
				makeNode(1,'o',NULL,-1);
				return LOGOP;
			}
			
"!"			{
				makeNode(1,'x',NULL,-1);
				return NOTOP;
			}
		
[a-zA-Z][a-zA-Z0-9]*	{
				makeNode(-1,'i',yytext,-1);
				return ID;
			}	
						
[\"].*[\"]		{
				makeNode(3,'s',yytext,-1);
				return STRING;
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
void makeNode(int type,char nodetype, char *name, int value)
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
