#include<string.h>
#define LEGAL 0
extern int linecount;
int flag_alias=LEGAL;
int regcount=0;
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
struct define *root_define=NULL;
FILE *fp;
char alias_table[8][30];

int labelcount=0;
struct label
{
	int i;
	struct label *next; 
};
struct label *root_label=NULL;
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
int lookup_alias(char *name)
{
	int i=0;
	while(i<8)
	{
		if(strcmp(alias_table[i],name)==0)
			return(i+8);
		i++;
	}
	return(-1);
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
void insert_alias(char *name,int value)
{
	struct define * temp;
	if(flag_alias!=LEGAL)
	{
		printf("\n%d: Aliasing cannot be done within if and while!!\n",linecount);
		exit(0);
	}
	if(!(value>=8 && value <=15))
	{
		printf("\n%d: Only kernel registers R8 to R15 can be aliased!!\n",linecount);
		exit(0);
	}	
	temp=lookup_constant(name);
	if(temp==NULL)
		strcpy(alias_table[value-8],name);
	else
	{
		printf("\n%d: Alias name %s already used as symbolic contant!!\n",linecount,name);
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
struct tree * substitute_id(struct tree *id)
{
	struct define *temp;
	int n;
	temp=lookup_constant(id->name);
	if(temp!=NULL)
	{
		id->nodetype='c';
		id->name=NULL;
		id->value=temp->value;
		return(id);
	}
	n=lookup_alias(id->name);
	if(n<0)
	{
		printf("\n%d: Unknown identifier %s used!!\n",linecount,id->name);
		exit(0);
	}
	id->nodetype='R';
	id->name=NULL;
	id->value=n;
	return(id);
}
void getreg(struct tree *root,char reg[])
{
	if(root->value>=0 && root->value<=15)
		sprintf(reg,"R%d",root->value);
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
					fprintf(fp,"LT T%d,%s\n",regcount-1,reg1);
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
					fprintf(fp,"GT T%d,%s\n",regcount-1,reg1);
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
		case 'e':
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
		case 'l':
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
					fprintf(fp,"LE T%d,%s\n",regcount-1,reg1);
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
		case 'g':
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
					fprintf(fp,"GE T%d,%s\n",regcount-1,reg1);
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
		case '!':
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
					codegen(root->ptr2);
					fprintf(fp,"SUB T%d,%s\n",regcount-1,reg1);
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
					codegen(root->ptr2);
					fprintf(fp,"DIV T%d,%s\n",regcount-1,reg1);
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
					codegen(root->ptr2);
					fprintf(fp,"MOD T%d,%s\n",regcount-1,reg1);
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
		case '=':
			switch(root->value)
			{
				case 0:			//reg=y
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
					break;
				case 1:			//reg=[y]
					getreg(root->ptr1,reg1);
					if(root->ptr2->nodetype=='R')		//reg=[reg]
					{
						getreg(root->ptr2,reg2);
						fprintf(fp,"MOV %s,[%s]\n",reg1,reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//reg=[no]
					{
						fprintf(fp,"MOV %s,[%d]\n",reg1,root->ptr2->value);
					}
					else					//reg=[expr]
					{
						codegen(root->ptr2);
						fprintf(fp,"MOV %s,[T%d]\n",reg1,regcount-1);
						regcount--;
					}
					break;
				case 2:			//[expr/no]=y
					if(root->ptr1->nodetype=='c')	//[no]=y
					{
						if(root->ptr2->nodetype=='R')		//[no]=reg
						{
							getreg(root->ptr2,reg2);
							fprintf(fp,"MOV [%d],%s\n",root->ptr1->value,reg2);	
						}
						else if(root->ptr2->nodetype=='c')	//[no]=no
						{
							fprintf(fp,"MOV [%d],%d\n",root->ptr1->value,root->ptr2->value);
						}
						else					//[no]=expr
						{
							codegen(root->ptr2);
							fprintf(fp,"MOV [%d],T%d\n",root->ptr1->value,regcount-1);
							regcount--;
						}
					}
					else				//[expr]=y
					{
						codegen(root->ptr1);
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
					break;
				case 3:			//[x]=[y]
					if(root->ptr1->nodetype=='c')	//[no]=[y]
					{
						if(root->ptr2->nodetype=='R')		//[no]=[reg]
						{
							getreg(root->ptr2,reg2);
							fprintf(fp,"MOV [%d],[%s]\n",root->ptr1->value,reg2);	
						}
						else if(root->ptr2->nodetype=='c')	//[no]=[no]
						{
							fprintf(fp,"MOV [%d],[%d]\n",root->ptr1->value,root->ptr2->value);
						}
						else					//[no]=[expr]
						{
							codegen(root->ptr2);
							fprintf(fp,"MOV [%d],[T%d]\n",root->ptr1->value,regcount-1);
							regcount--;
						}
					}
					else				//[expr]=[y]
					{
						codegen(root->ptr1);
						if(root->ptr2->nodetype=='R')		//[expr]=[reg]
						{
							getreg(root->ptr2,reg2);
							fprintf(fp,"MOV [T%d],[%s]\n",regcount-1,reg2);	
						}
						else if(root->ptr2->nodetype=='c')	//[expr]=[no]
						{
							fprintf(fp,"MOV [T%d],[%d]\n",regcount-1,root->ptr2->value);
						}
						else					//[expr]=[expr]
						{
							codegen(root->ptr2);
							fprintf(fp,"MOV [T%d],[T%d]\n",regcount-2,regcount-1);
							regcount--;
						}
						regcount--;
					}
					break;
				default:
					printf("Invalid value in assignment %d.\n",root->value);
//debugging
					break;
			}
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
			break;
		case 'L':
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
		case 'S':
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
		case 'P':
			getreg(root->ptr1,reg1);
			getreg(root->ptr2,reg2);
			fprintf(fp,"MOV T%d,%s\nSTRCMP T%d,%s\n",regcount,reg1,regcount,reg2);
			regcount++;		
			break;
		case 'Y':
			getreg(root->ptr1,reg1);
			getreg(root->ptr2,reg2);
			fprintf(fp,"MOV T%d,%s\nSTRCPY T%d,%s\n",regcount,reg1,regcount,reg2);		
			break;
		case 'I':
			fprintf(fp,"IRET\n");
			break;
		case 'R':
			 getreg(root,reg1);
			 fprintf(fp,"MOV T%d,%s\n",regcount,reg1);
			 regcount++;
			 break;
		default:
			printf("Unknown nodetype %c\n",root->nodetype);		//Debugging
			return;
	}
}
