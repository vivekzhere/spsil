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
				func(2,'?',NULL,-1);
				return IF;
				}	
						
"ireturn"		{
				func(2,'I',NULL,-1);
				return IRETURN;
				}

"load"			{
				func(2,'L',NULL,-1);
				return LOAD;
				}
			
"read"			{
				func(2,'r',NULL,-1);
				return READ;
				}

"store"			{
				func(2,'S',NULL,-1);
				return STORE;
				}
				
"then"			{
				return THEN;
				}

"while"			{
				func(2,'w',NULL,-1);
				return WHILE;
				}
			
"write"			{
				func(2,'p',NULL,-1);
				return PRINT;
				}

"R0"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R1"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R2"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R3"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R4"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R5"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R6"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R7"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R8"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R9"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R10"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R11"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R12"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R13"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R14"			{
				func(0,'R',NULL,1);
				return REG;
				}
						
"R15"			{
				func(0,'R',NULL,1);
				return REG;
				}
				
"BP"			{
				func(0,'R',NULL,1);
				return REG;
				}

"SP"			{
				func(0,'R',NULL,1);
				return REG;
				}

"IP"			{
				func(0,'R',NULL,1);
				return REG;
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
			
"&&"			{
				func(1,'a',NULL,-1);
				return LOGOP;
				}

"||"			{
				func(1,'o',NULL,-1);
				return LOGOP;
				}
			
"!"				{
				func(1,'x',NULL,-1);
				return NEGOP;
				}
			
[a-zA-Z][a-zA-Z0-9]*	{
						func(-1,'i',yytext,-1);
						return ID;
						}	
						
[\"].*[\"]				{
						func(3,'s',yytext,-1);
						return STRING;
						}
			
[(),;\[\]\{\}&]			{return *yytext;}

[/][/].*\n				{linecount++;}

[\n]					{linecount++;}

[ \t]					{}

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
