#include<string.h>
#define LEGAL 0
extern int linecount;
int flag_break=0;
int regcount=0;
struct tree
{
	char nodetype;		/*	+,-,*,/,%,=,<,>
					?-if statement,		I-ireturn,	L-load
					S-store,	P-strcmp,	Y-strcpy,	w-while,
					R-register  //value=0-15,20-SP,21-BP,22-IP
					e-double equals,	l-lessthan or equals
					g-greaterthan or equals		!-not equal
					a-AND		o-OR		x-NOT
					c-number,	i-identifier,
					n-nonterminal
					b-break		t-continue	m-addresing expr
					h-halt		C-checkpoint	I-ireturn		
					1-IN	2-OUT	3-SIN	4-SOUT
						*/
	char *name;
	int value;
	struct define *entry;
	struct tree *ptr1,*ptr2,*ptr3;
};

FILE *fp;
						//start labels
int labelcount=0;
struct label
{
	int i;
	struct label *next; 
};
struct label *root_label=NULL,*root_while=NULL;
void push_label()
{
	struct label *temp;
	temp=malloc(sizeof(struct label));
	temp->i=labelcount;
	temp->next=root_label;
	root_label=temp; 
	labelcount++;
}
int pop_label()
{
	int i;
	struct label *temp;
	temp=root_label;
	root_label=root_label->next;
	i=temp->i;
	free(temp);
	return i;
}
void push_while(int n)
{
	struct label *temp;
	temp=malloc(sizeof(struct label));
	temp->i=n;
	temp->next=root_while;
	root_while=temp; 
}
void pop_while()
{
	struct label *temp;
	temp=root_while;
	root_while=root_while->next;
	free(temp);
}
						///end labels
						///start constants and aliasing
struct define
{
	char *name;
	int value;
	struct define *next;
};
struct define *root_define=NULL;
char alias_table[8][30];
struct define* lookup_constant(char *name)
{
	struct define *temp=root_define;	
	while(temp!=NULL)
	{
		if(strcmp(name,temp->name)==0)
			return temp;
		temp=temp->next;
	}
	return NULL;
}
int depth=0;
struct alias
{
	char name[30];
	int no,depth;
	struct alias *next;
};
struct alias *root_alias=NULL;
struct alias * lookup_alias(char *name)
{
	struct alias *temp=root_alias;
	while(temp!=NULL)
	{
		if(strcmp(temp->name,name)==0)
			return(temp);
		temp=temp->next;
	}
	return(NULL);
}
struct alias * lookup_alias_reg(int no)
{
	struct alias *temp=root_alias;
	while(temp!=NULL)
	{
		if(no==temp->no)
			return(temp);
		temp=temp->next;
	}
	return(NULL);
}
void push_alias(char *name,int no)
{
	struct alias *temp;		
	if(lookup_constant(name)!=NULL)
	{
		printf("\n%d: Alias name %s already used as symbolic contant!!\n",linecount,name);
		exit(0);
	}
	temp=lookup_alias(name);
	if(temp!=NULL && temp->depth == depth)
	{
		printf("\n%d: Alias name %s already used as in the current block!!\n",linecount,name);
		exit(0);
	}
	else
	{	temp=lookup_alias_reg(no);
		if(temp!=NULL && temp->depth==depth)
			strcpy(temp->name,name);
		else
		{
			temp=malloc(sizeof(struct alias));
			strcpy(temp->name,name);
			temp->no=no;
			temp->depth=depth;
			temp->next=root_alias;
			root_alias=temp;
		}		
	}
}
void pop_alias()
{
	struct alias *temp;
	temp=root_alias;
	while(temp!=NULL && temp->depth==depth)
	{
		root_alias=temp->next;
		free(temp);
		temp=root_alias;
	}
}
void insert_constant(char *name,int value)
{
	struct define * temp;
	temp=lookup_constant(name);
	if(temp==NULL)
	{
		temp=malloc(sizeof(struct define));
		temp->name=name;
		temp->value=value;
		temp->next=root_define;
		root_define=temp;
	}
	else
	{
		printf("\n%d: Multiple definition of symbolic contant %s !!\n",linecount,name);
		exit(0);
	}
}      
void add_predefined_constants()
{
	struct define * temp;
	char name[15];
	strcpy(name,"SCRATCHPAD");
	if(lookup_constant(name)==NULL)
		insert_constant(name,256);
		
	strcpy(name,"PAGE_TABLE");
	if(lookup_constant(name)==NULL)
		insert_constant(name,512);
		
	strcpy(name,"MEM LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name,576);
		
	strcpy(name,"FILE TABLE");
	if(lookup_constant(name)==NULL)
		insert_constant(name,640);
		
	strcpy(name,"READY LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name,736);
		
	strcpy(name,"PROC TABLE");
	if(lookup_constant(name)==NULL)
		insert_constant(name,767);
		
	strcpy(name,"FAT");
	if(lookup_constant(name)==NULL)
		insert_constant(name,1024);
		
	strcpy(name,"DISK LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name,1536);
	strcpy(name,"USER PROG");
	if(lookup_constant(name)==NULL)
		insert_constant(name,1792);
		
	strcpy(name,"INTERRUPT");
	if(lookup_constant(name)==NULL)
		insert_constant(name,13824);	
}
struct tree * substitute_id(struct tree *id)
{
	struct define *temp;
	struct alias *temp2;
	temp=lookup_constant(id->name);
	if(temp!=NULL)
	{
		id->nodetype='c';
		id->name=NULL;
		id->value=temp->value;
		return(id);
	}
	temp2=lookup_alias(id->name);
	if(temp2==NULL)
	{
		printf("\n%d: Unknown identifier %s used!!\n",linecount,id->name);
		exit(0);
	}
	id->nodetype='R';
	id->name=NULL;
	id->value=temp2->no;
	return(id);
}
							///end of constants and alias
							///start tree create fns
struct tree * create_nonterm_node(char *name,struct tree *a,struct tree *b)
{
	struct tree *temp=malloc(sizeof(struct tree));
	temp->nodetype='n';
	temp->name=name;
	temp->entry=NULL;
	temp->ptr1=a;
	temp->ptr2=b;
	temp->ptr3=NULL;
	return temp;
}
struct tree * create_tree(struct tree *a,struct tree *b,struct tree *c,struct tree *d)
{	
	a->ptr1=b;
	a->ptr2=c;
	a->ptr3=d;	
}
							///end tree create fns
void getreg(struct tree *root,char reg[])
{
	if(root->value>=0 && root->value<=7)
		sprintf(reg,"R%d",root->value);
	else if(root->value>=8 && root->value<=15)
		sprintf(reg,"S%d",(root->value)-8);
	else if(root->value==20)
		sprintf(reg,"BP");
	else if(root->value==21)
		sprintf(reg,"SP");
	else if(root->value==22)
		sprintf(reg,"IP");
	else
		 sprintf(reg,"PID"); 
}
void codegen(struct tree * root)
{
	int n;
	char reg1[4],reg2[4];
	if(root==NULL)
		return;	
	switch(root->nodetype)
	{
		case '<':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nLT T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"GT T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"LT T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LT T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '>':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nGT T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LT T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"GT T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"GT T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'e':		//double equals
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nEQ T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"EQ T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"EQ T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"EQ T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'l':		//lessthan or equals
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nLE T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"GE T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"LE T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LE T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'g':		//greaterthan or equals	
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nGE T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LE T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"GE T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"GE T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '!':		//not equal
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nNE T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"NE T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"NE T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"NE T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'a':	//AND operator
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nMUL T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"MUL T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MUL T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"MUL T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'o':	//OR operator
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nADD T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"ADD T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"ADD T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"ADD T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case 'x':	//NOT operator
			fprintf(fp,"MOV T%d,1\n",regcount);
			regcount++;
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				fprintf(fp,"SUB T%d,%s\n",regcount-1,reg1);
			}
			else
			{
				codegen(root->ptr1);
				fprintf(fp,"SUB T%d,T%d\n",regcount-2,regcount-1);
				regcount--;
			}
			break;		
		case 'n':	//statement list
			codegen(root->ptr1);
			codegen(root->ptr2);			
			break;
		case '+':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nADD T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"ADD T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"ADD T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"ADD T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '-':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nSUB T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
					regcount++;
					codegen(root->ptr2);
					fprintf(fp,"SUB T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"SUB T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"SUB T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '*':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nMUL T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"MUL T%d,%s\n",regcount-1,reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MUL T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"MUL T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '/':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nDIV T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
					regcount++;
					codegen(root->ptr2);
					fprintf(fp,"DIV T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"DIV T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"DIV T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '%':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nMOD T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;	
				}
				else
				{
					fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
					regcount++;
					codegen(root->ptr2);
					fprintf(fp,"MOD T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOD T%d,%s\n",regcount-1,reg2);	
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"MOD T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}			
			break;
		case '=':		//assignment
			if(root->ptr1->nodetype=='m') //[expr/no]=*
			{
				
				if(root->ptr1->ptr1->nodetype=='c')	//[no]=*
				{
					if(root->ptr2->nodetype=='R')		//[no]=reg
					{
						getreg(root->ptr2,reg2);
						fprintf(fp,"MOV [%d],%s\n",root->ptr1->ptr1->value,reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//[no]=no
					{
						fprintf(fp,"MOV [%d],%d\n",root->ptr1->ptr1->value,root->ptr2->value);
					}
					else					//[no]=expr
					{
						codegen(root->ptr2);
						fprintf(fp,"MOV [%d],T%d\n",root->ptr1->ptr1->value,regcount-1);
						regcount--;
					}
				}
				else				//[expr]=*
				{
					codegen(root->ptr1->ptr1);
					if(root->ptr2->nodetype=='R')		//[expr]=reg
					{
						getreg(root->ptr2,reg2);
						fprintf(fp,"MOV [T%d],%s\n",regcount-1,reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//[expr]=no
					{
						fprintf(fp,"MOV [T%d],%d\n",regcount-1,root->ptr2->value);
					}
					else					//[expr]=expr
					{
						codegen(root->ptr2);
						fprintf(fp,"MOV [T%d],T%d\n",regcount-2,regcount-1);
						regcount--;
					}
					regcount--;
				}
			}
			else				//reg=*
			{			
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')		//reg=reg
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV %s,%s\n",reg1,reg2);	
				}
				else if(root->ptr2->nodetype=='c')	//reg=no
				{
					fprintf(fp,"MOV %s,%d\n",reg1,root->ptr2->value);
				}
				else					//reg=expr
				{
					codegen(root->ptr2);
					fprintf(fp,"MOV %s,T%d\n",reg1,regcount-1);
					regcount--;
				}			
			}			
			break;
		case 'm':	//addresing
			codegen(root->ptr1);
			fprintf(fp,"MOV T%d,[T%d]\n",regcount-1,regcount-1);
			break;
		case 'c':	//constants
			fprintf(fp,"MOV T%d,%d\n",regcount,root->value);
			regcount++;
			break;
		case '?':	//IF statement , IF-ELSE statements
			push_label();			
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				fprintf(fp,"JZ %s,la%d\n",reg1,root_label->i);
			}
			else
			{				
				codegen(root->ptr1);
				fprintf(fp,"JZ T%d,la%d\n",regcount-1,root_label->i);
				regcount--;
			}			
			codegen(root->ptr2);
			fprintf(fp,"JMP lb%d\n",root_label->i);
			fprintf(fp,"la%d:\n",root_label->i);
			codegen(root->ptr3);
			fprintf(fp,"lb%d:\n",pop_label());
			break;
		case 'w':	//WHILE loop
			push_label();
			push_while(root_label->i);
			fprintf(fp,"la%d:\n",root_label->i);
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				fprintf(fp,"JZ %s,lb%d\n",reg1,root_label->i);
			}
			else
			{				
				codegen(root->ptr1);
				fprintf(fp,"JZ T%d,lb%d\n",regcount-1,root_label->i);
				regcount--;
			}			
			codegen(root->ptr2);
			fprintf(fp,"JMP la%d\n",root_label->i);
			fprintf(fp,"lb%d:\n",pop_label());
			pop_while();
			break;
		case 'b':	//BREAK loop
			fprintf(fp,"JMP lb%d\n",root_while->i);
			break;
		case 't':	//CONTINUE loop
			fprintf(fp,"JMP la%d\n",root_while->i);
			break;
		case 'L':	//Load
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"LOAD %s,%s\n",reg1,reg2);						
				}
				else if(root->ptr2->nodetype=='c')
					fprintf(fp,"LOAD %s,%d\n",reg1,root->ptr2->value);
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LOAD %s,T%d\n",reg1,regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"LOAD T%d,%s\n",regcount-1,reg2);	
				}
				else if(root->ptr2->nodetype=='c')
					fprintf(fp,"LOAD T%d,%d\n",regcount-1,root->ptr2->value);
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"LOAD T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
				regcount--;
			}			
			break;
		case 'S':	//Store
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"STORE %s,%s\n",reg1,reg2);						
				}
				else if(root->ptr2->nodetype=='c')
					fprintf(fp,"STORE %s,%d\n",reg1,root->ptr2->value);
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"STORE %s,T%d\n",reg1,regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"STORE %s,T%d\n",reg2,regcount-1);	
				}
				else if(root->ptr2->nodetype=='c')
					fprintf(fp,"STORE %d,T%d\n",root->ptr2->value,regcount-1);
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"STORE T%d,T%d\n",regcount-1,regcount-2);
					regcount--;
				}
				regcount--;
			}			
			break;
		case 'P':	//strcmp
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nSTRCMP T%d,%s\n",regcount,reg1,regcount,reg2);
					regcount++;
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"STRCMP T%d,%s\n",regcount-1,reg1);
				}
			}
			else
			{
				codegen(root->ptr1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"STRCMP T%d,%s\n",regcount-1,reg2);
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"STRCMP T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
			}		
			break;
		case 'Y':	//strcpy
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1,reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"MOV T%d,%s\nSTRCPY T%d,%s\n",regcount,reg1,regcount,reg2);
				}
				else
				{
					fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
					regcount++;
					codegen(root->ptr2);
					fprintf(fp,"STRCPY T%d,T%d\n",regcount-2,regcount-1);
					regcount-=2;
				}
			}
			else
			{
				codegen(root->ptr1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2,reg2);
					fprintf(fp,"STRCPY T%d,%s\n",regcount-1,reg2);
				}
				else
				{
					codegen(root->ptr2);
					fprintf(fp,"STRCMP T%d,T%d\n",regcount-2,regcount-1);
					regcount--;
				}
				regcount--;
			}		
			break;			
		case 'I':	//Ireturn
			fprintf(fp,"IRET\n");
			break;
		case 'R':	//register
			 getreg(root,reg1);
			 fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
			 regcount++;
			 break;
		case 'h':	//halt
			fprintf(fp,"HALT\n");
			break;
		case 'C':	//checkpoint
			fprintf(fp,"BRKP\n");
			break;
		case '1':	//IN
			getreg(root->ptr1,reg1);
			fprintf(fp,"IN %s\n",reg1);
			break;
		case '2':	//OUT
			codegen(root->ptr1);
			fprintf(fp,"OUT T%d\n",regcount-1);
			regcount--;
			break;
		case '3':	//SIN
			getreg(root->ptr1,reg1);
			fprintf(fp,"SIN %s\n",reg1);
			break;
		case '4':	//SOUT
			codegen(root->ptr1);
			fprintf(fp,"SOUT T%d\n",regcount-1);
			regcount--;
			break;
		default:
			printf("Unknown nodetype %c\n",root->nodetype);		//Debugging
			return;
	}
}
