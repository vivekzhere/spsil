#include<string.h>
#include "data.h"
#define LEGAL 0

//Pre-defined constants in SPL
#define SCRATCHPAD 	512
#define PAGE_TABLE	1024
#define MEM_LIST	1280
#define FILE_TABLE	1344
#define READY_LIST	1536
#define FAT		2560
#define DISK_LIST	3072
#define EX_HANDLER	3584
#define T_INTERRUPT	4608
#define INTERRUPT	5632
#define USER_PROG	12800

#define PAGE_PER_INTERRUPT	2
#define PAGE_SIZE 		512

extern int linecount;
unsigned long temp_pos; //temporary lseek
int out_linecount=0; //no of lines of code generated
int addrBaseVal;	//Starting Address where the compiled code will be loaded
int flag_break=0;
int regcount=0;
struct tree
{
	char nodetype;		/*	+, -, *, /, %, =, <, >
					?-if statement, 		I-ireturn, 	L-load
					S-store, 	P-strcmp, 	Y-strcpy, 	w-while, 
					R-register  //value=0-15, 20-SP, 21-BP, 22-IP 23-PTBR 24-PTLR
					e-double equals, 	l-lessthan or equals
					g-greaterthan or equals		!-not equal
					a-AND		o-OR		x-NOT
					c-number, 	i-identifier, 
					n-nonterminal
					b-break		t-continue	m-addresing expr
					h-halt		C-checkpoint	I-ireturn		
					1-IN	2-OUT	3-INLINE
						*/
	char *name;
	int value;
	struct define *entry;
	struct tree *ptr1, *ptr2, *ptr3;
};

FILE *fp;
						//start labels
int labelcount=0;
struct jmp_point
{
	unsigned long pos;
	char instr[32];
	struct jmp_point *next;
	struct jmp_point *points; 
};
struct label
{
	int i;
	unsigned long pos1,pos2;
	char instr1[32],instr2[32];
	struct label *next;
	struct jmp_point *points;  
};
struct label *root_label=NULL, *root_while=NULL;
void push_label()
{
	struct label *temp;
	temp=malloc(sizeof(struct label));
	temp->i=labelcount;
	temp->pos1=0;
	temp->pos2=0;
	bzero(temp->instr1,32);
	bzero(temp->instr2,32);
	temp->points=NULL;
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
	temp->pos1=0;
	temp->pos2=0;
	bzero(temp->instr1,32);
	bzero(temp->instr2,32);
	temp->points=NULL;
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
void add_jmp_point(char instr[32])
{
	struct jmp_point *temp;
	temp=malloc(sizeof(struct jmp_point)); 
	fflush(fp);
	temp->pos = ftell(fp);
	strcpy(temp->instr,instr);
	temp->next = root_while->points;
	root_while->points = temp;	
}
void use_jmp_points(struct jmp_point *root)
{
	if(root == NULL)
		return;
	else
	{
		fflush(fp);
		temp_pos = ftell(fp);
		fseek(fp,root->pos,SEEK_SET);
		fprintf(fp,"%s %05d",root->instr,addrBaseVal + out_linecount*2);
		fseek(fp,temp_pos,SEEK_SET);
		use_jmp_points(root->next);
		free(root);
	}
}
						///end labels
						///start constants and aliasing
struct define
{
	char name[30];
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
		if(strcmp(name, temp->name)==0)
			return temp;
		temp=temp->next;
	}
	return NULL;
}
int depth=0;
struct alias
{
	char name[30];
	int no, depth;
	struct alias *next;
};
struct alias *root_alias=NULL;
struct alias * lookup_alias(char *name)
{
	struct alias *temp=root_alias;
	while(temp!=NULL)
	{
		if(strcmp(temp->name, name)==0)
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
void push_alias(char *name, int no)
{
	struct alias *temp;		
	if(lookup_constant(name)!=NULL)
	{
		printf("\n%d: Alias name %s already used as symbolic contant!!\n", linecount, name);
		exit(0);
	}
	temp=lookup_alias(name);
	if(temp!=NULL && temp->depth == depth)
	{
		printf("\n%d: Alias name %s already used as in the current block!!\n", linecount, name);
		exit(0);
	}
	else
	{	temp=lookup_alias_reg(no);
		if(temp!=NULL && temp->depth==depth)
			strcpy(temp->name, name);
		else
		{
			temp=malloc(sizeof(struct alias));
			strcpy(temp->name, name);
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
void insert_constant(char *name, int value)
{
	struct define * temp;
	temp=lookup_constant(name);
	if(temp==NULL)
	{
		temp=malloc(sizeof(struct define));
		strcpy(temp->name,name);
		temp->value=value;
		temp->next=root_define;
		root_define=temp;
	}
	else
	{
		printf("\n%d: Multiple Definition for Contant %s \n", linecount, name);
		exit(0);
	}
}      
void add_predefined_constants()
{
	struct define * temp;
	char name[15];
	
	bzero(name,15);
	strcpy(name, "SCRATCHPAD");
	if(lookup_constant(name)==NULL)
		insert_constant(name, SCRATCHPAD);
		
	bzero(name,15);
	strcpy(name, "PAGE_TABLE");
	if(lookup_constant(name)==NULL)
		insert_constant(name, PAGE_TABLE);
		
	bzero(name,15);
	strcpy(name, "MEM_LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name, MEM_LIST);
		
	bzero(name,15);
	strcpy(name, "FILE_TABLE");
	if(lookup_constant(name)==NULL)
		insert_constant(name, FILE_TABLE);
		
	bzero(name,15);
	strcpy(name, "READY_LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name, READY_LIST);
		
	bzero(name,15);
	strcpy(name, "FAT");
	if(lookup_constant(name)==NULL)
		insert_constant(name, FAT);
		
	bzero(name,15);
	strcpy(name, "DISK_LIST");
	if(lookup_constant(name)==NULL)
		insert_constant(name, DISK_LIST);
		
	bzero(name,15);
	strcpy(name, "EX_HANDLER");
	if(lookup_constant(name)==NULL)
		insert_constant(name, EX_HANDLER);	
			
	bzero(name,15);
	strcpy(name, "T_INTERRUPT");
	if(lookup_constant(name)==NULL)
		insert_constant(name, T_INTERRUPT);	
		
	bzero(name,15);
	strcpy(name, "INTERRUPT");
	if(lookup_constant(name)==NULL)
		insert_constant(name, INTERRUPT);	
		
	bzero(name,15);
	strcpy(name, "USER_PROG");
	if(lookup_constant(name)==NULL)
		insert_constant(name, USER_PROG);
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
		printf("\n%d: Unknown identifier %s used!!\n", linecount, id->name);
		exit(0);
	}
	id->nodetype='R';
	id->name=NULL;
	id->value=temp2->no;
	return(id);
}
							///end of constants and alias
							///start tree create fns
struct tree * create_nonterm_node(char *name, struct tree *a, struct tree *b)
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
struct tree * create_tree(struct tree *a, struct tree *b, struct tree *c, struct tree *d)
{	
	a->ptr1=b;
	a->ptr2=c;
	a->ptr3=d;	
}
							///end tree create fns
void getreg(struct tree *root, char reg[])
{
	if(root->value >= R0 && root->value <= R7)
		sprintf(reg, "R%d", root->value - R0);
	else if(root->value >= S0 && root->value <= S15)
		sprintf(reg, "S%d", root->value - S0);
	else if(root->value == BP_REG)
		sprintf(reg, "BP");
	else if(root->value == SP_REG)
		sprintf(reg, "SP");
	else if(root->value == IP_REG)
		sprintf(reg, "IP");
	else if(root->value == PTBR_REG)
		sprintf(reg, "PTBR");
	else if(root->value == PTLR_REG)
		sprintf(reg, "PTLR");
	else if(root->value == EFR_REG)
		sprintf(reg, "EFR");		
}
void codegen(struct tree * root)
{
	int n;
	char reg1[5], reg2[5];
	if(root==NULL)
		return;	
	switch(root->nodetype)
	{
		case '<':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d,  %s\nLT T%d,  %s\n", regcount,  reg1,  regcount,  reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp,  "GT T%d,  %s\n",  regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "LT T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "LT T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '>':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nGT T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "LT T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "GT T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "GT T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'e':		//double equals
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nEQ T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "EQ T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "EQ T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "EQ T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'l':		//lessthan or equals
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nLE T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "GE T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "LE T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "LE T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'g':		//greaterthan or equals	
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nGE T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "LE T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "GE T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "GE T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '!':		//not equal
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nNE T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "NE T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "NE T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "NE T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'a':	//AND operator
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nMUL T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MUL T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "MUL T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MUL T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'o':	//OR operator
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nADD T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "ADD T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "ADD T%d, %s\n", regcount-1, reg2);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "ADD T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case 'x':	//NOT operator
			out_linecount++;
			fprintf(fp, "MOV T%d, 1\n", regcount);
			regcount++;
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				out_linecount++;
				fprintf(fp, "SUB T%d, %s\n", regcount-1, reg1);
			}
			else
			{
				codegen(root->ptr1);
				out_linecount++;
				fprintf(fp, "SUB T%d, T%d\n", regcount-2, regcount-1);
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
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nADD T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nADD T%d, %d\n", regcount, reg1, regcount, root->ptr2->value);
					regcount++;	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "ADD T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "ADD T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "ADD T%d, %d\n", regcount-1, root->ptr2->value);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "ADD T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '-':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nSUB T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nSUB T%d, %d\n", regcount, reg1, regcount, root->ptr2->value);	
					regcount++;	
				}
				else
				{
					out_linecount++;
					fprintf(fp, "MOV T%d, %s\n", regcount, reg1);
					regcount++;
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "SUB T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "SUB T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "SUB T%d, %d\n", regcount-1, root->ptr2->value);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "SUB T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '*':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nMUL T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nMUL T%d, %d\n", regcount, reg1, regcount, root->ptr2->value);
					regcount++;		
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MUL T%d, %s\n", regcount-1, reg1);
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "MUL T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "MUL T%d, %d\n", regcount-1, root->ptr2->value);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MUL T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '/':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nDIV T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nDIV T%d, %d\n", regcount, reg1, regcount, root->ptr2->value);
					regcount++;		
				}
				else
				{
					out_linecount++;
					fprintf(fp, "MOV T%d, %s\n", regcount, reg1);
					regcount++;
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "DIV T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}		
							
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "DIV T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "DIV T%d, %d\n", regcount-1, root->ptr2->value);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "DIV T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
			}			
			break;
		case '%':
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nMOD T%d, %s\n", regcount, reg1, regcount, reg2);
					regcount++;	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount+=2;
					fprintf(fp, "MOV T%d, %s\nMOD T%d, %d\n", regcount, reg1, regcount, root->ptr2->value);
					regcount++;		
				}
				else
				{
					out_linecount++;
					fprintf(fp, "MOV T%d, %s\n", regcount, reg1);
					regcount++;
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MOD T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "MOD T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "MOD T%d, %d\n", regcount-1, root->ptr2->value);	
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MOD T%d, T%d\n", regcount-2, regcount-1);
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
						getreg(root->ptr2, reg2);
						out_linecount++;
						fprintf(fp, "MOV [%d], %s\n", root->ptr1->ptr1->value, reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//[no]=no
					{
						out_linecount++;
						fprintf(fp, "MOV [%d], %d\n", root->ptr1->ptr1->value, root->ptr2->value);
					}
					else if(root->ptr2->nodetype=='s')	//[no]=string
					{
						out_linecount++;
						fprintf(fp, "MOV [%d], %s\n", root->ptr1->ptr1->value, root->ptr2->name);
					}
					else					//[no]=expr
					{
						codegen(root->ptr2);
						out_linecount++;
						fprintf(fp, "MOV [%d], T%d\n", root->ptr1->ptr1->value, regcount-1);
						regcount--;
					}
				}
				else if(root->ptr1->ptr1->nodetype=='R')				//[reg]=*
				{
					getreg(root->ptr1->ptr1, reg1);
					if(root->ptr2->nodetype=='R')		//[reg]=reg
					{
						getreg(root->ptr2, reg2);
						out_linecount++;
						fprintf(fp, "MOV [%s], %s\n", reg1, reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//[reg]=no
					{
						out_linecount++;
						fprintf(fp, "MOV [%s], %d\n", reg1, root->ptr2->value);
					}
					else if(root->ptr2->nodetype=='s')	//[reg]=string
					{
						out_linecount++;
						fprintf(fp, "MOV [%s], %s\n", reg1, root->ptr2->name);
					}
					else					//[reg]=expr
					{
						codegen(root->ptr2);
						out_linecount++;
						fprintf(fp, "MOV [%s], T%d\n", reg1, regcount-1);
						regcount--;
					}
				}
				else				//[expr]=*
				{
					codegen(root->ptr1->ptr1);
					if(root->ptr2->nodetype=='R')		//[expr]=reg
					{
						getreg(root->ptr2, reg2);
						out_linecount++;
						fprintf(fp, "MOV [T%d], %s\n", regcount-1, reg2);	
					}
					else if(root->ptr2->nodetype=='c')	//[expr]=no
					{
						out_linecount++;
						fprintf(fp, "MOV [T%d], %d\n", regcount-1, root->ptr2->value);
					}
					else if(root->ptr2->nodetype=='s')	//[expr]=string
					{
						out_linecount++;
						fprintf(fp, "MOV [T%d], %s\n", regcount-1, root->ptr2->name);
					}
					else					//[expr]=expr
					{
						codegen(root->ptr2);
						out_linecount++;
						fprintf(fp, "MOV [T%d], T%d\n", regcount-2, regcount-1);
						regcount--;
					}
					regcount--;
				}
			}
			else				//reg=*
			{			
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')		//reg=reg
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "MOV %s, %s\n", reg1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')	//reg=no
				{
					out_linecount++;
					fprintf(fp, "MOV %s, %d\n", reg1, root->ptr2->value);
				}
				else if(root->ptr2->nodetype=='s')	//reg=string
				{
					out_linecount++;
					fprintf(fp, "MOV %s, %s\n", reg1, root->ptr2->name);
				}
				else					//reg=expr
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "MOV %s, T%d\n", reg1, regcount-1);
					regcount--;
				}			
			}			
			break;
		case 'm':	//addresing
			codegen(root->ptr1);
			out_linecount++;
			fprintf(fp, "MOV T%d, [T%d]\n", regcount-1, regcount-1);
			break;
		case 'c':	//constants
			out_linecount++;
			fprintf(fp, "MOV T%d, %d\n", regcount, root->value);
			regcount++;
			break;
		case 's':	//string
			out_linecount++;
			fprintf(fp, "MOV T%d,  %s\n", regcount, root->name);
			regcount++;
			break;
		case '?':	//IF statement ,  IF-ELSE statements
			push_label();			
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				fflush(fp);
				root_label->pos1 = ftell(fp);
				out_linecount++;
				fprintf(fp, "JZ %s, 00000\n", reg1);
				sprintf(root_label->instr1, "JZ %s,", reg1);
			}
			else
			{				
				codegen(root->ptr1);
				fflush(fp);
				root_label->pos1 = ftell(fp);
				out_linecount++;
				fprintf(fp, "JZ T%d, 00000\n", regcount-1);
				sprintf(root_label->instr1, "JZ T%d,", regcount-1);
				regcount--;
			}			
			codegen(root->ptr2);
			fflush(fp);
			root_label->pos2 = ftell(fp);
			out_linecount++;
			fprintf(fp, "JMP 00000\n");
			sprintf(root_label->instr2, "JMP");
			fflush(fp);
			temp_pos = ftell(fp);
			fseek(fp,root_label->pos1,SEEK_SET);
			fprintf(fp,"%s %05d",root_label->instr1,addrBaseVal +  out_linecount*2);
			fseek(fp,temp_pos,SEEK_SET);
			codegen(root->ptr3);
			fflush(fp);
			temp_pos = ftell(fp);
			fseek(fp,root_label->pos2,SEEK_SET);
			fprintf(fp,"%s %05d",root_label->instr2,addrBaseVal +  out_linecount*2);
			fseek(fp,temp_pos,SEEK_SET);
			pop_label();
			break;
		case 'w':	//WHILE loop
			push_label();
			push_while(root_label->i);
			root_label->pos1=addrBaseVal +  out_linecount*2;
			root_while->pos1=addrBaseVal +  out_linecount*2;
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				fflush(fp);
				root_label->pos2 = ftell(fp);
				out_linecount++;
				fprintf(fp, "JZ %s, 00000\n", reg1);
				sprintf(root_label->instr2, "JZ %s,", reg1);
			}
			else
			{				
				codegen(root->ptr1);
				fflush(fp);
				root_label->pos2 = ftell(fp);
				out_linecount++;
				fprintf(fp, "JZ T%d, 00000\n", regcount-1);
				sprintf(root_label->instr2, "JZ T%d,", regcount-1);
				regcount--;
			}			
			codegen(root->ptr2);
			out_linecount++;
			fprintf(fp, "JMP %ld\n", root_label->pos1);
			fflush(fp);
			temp_pos = ftell(fp);
			fseek(fp,root_label->pos2,SEEK_SET);
			fprintf(fp,"%s %05d",root_label->instr2,addrBaseVal +  out_linecount*2);
			fseek(fp,temp_pos,SEEK_SET);
			use_jmp_points(root_while->points);
			pop_while();
			pop_label();
			break;
		case 'b':	//BREAK loop
			add_jmp_point("JMP");
			out_linecount++;
			fprintf(fp, "JMP 00000\n");
			break;
		case 't':	//CONTINUE loop
			out_linecount++;
			fprintf(fp, "JMP %ld\n", root_while->pos1);
			break;
		case 'L':	//Load
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "LOAD %s, %s\n", reg1, reg2);						
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "LOAD %s, %d\n", reg1, root->ptr2->value);
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "LOAD %s, T%d\n", reg1, regcount-1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++; fprintf(fp, "LOAD T%d, %s\n", regcount-1, reg2);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "LOAD T%d, %d\n", regcount-1, root->ptr2->value);
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++; fprintf(fp, "LOAD T%d, T%d\n", regcount-2, regcount-1);
					regcount--;
				}
				regcount--;
			}			
			break;
		case 'S':	//Store
			if(root->ptr1->nodetype=='R')
			{
				getreg(root->ptr1, reg1);
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++;
					fprintf(fp, "STORE %s, %s\n", reg2, reg1);						
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "STORE %d, %s\n", root->ptr2->value, reg1 );
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++; fprintf(fp, "STORE T%d, %s\n", regcount-1, reg1);
					regcount--;
				}					
			}
			else
			{
				codegen(root->ptr1);			
				if(root->ptr2->nodetype=='R')
				{
					getreg(root->ptr2, reg2);
					out_linecount++; fprintf(fp, "STORE %s, T%d\n", reg2, regcount-1);	
				}
				else if(root->ptr2->nodetype=='c')
				{
					out_linecount++;
					fprintf(fp, "STORE %d, T%d\n", root->ptr2->value, regcount-1);
				}
				else
				{
					codegen(root->ptr2);
					out_linecount++;
					fprintf(fp, "STORE T%d, T%d\n", regcount-1, regcount-2);
					regcount--;
				}
				regcount--;
			}			
			break;			
		case 'I':	//Ireturn
			out_linecount++;
			fprintf(fp, "IRET\n");
			break;
		case 'R':	//register
			 getreg(root, reg1);
			 out_linecount++;
			 fprintf(fp, "MOV T%d, %s\n", regcount, reg1);
			 regcount++;
			 break;
		case 'h':	//halt
			out_linecount++;
			fprintf(fp, "HALT\n");
			break;
		case 'C':	//checkpoint
			out_linecount++;
			fprintf(fp, "BRKP\n");
			break;
		case '1':	//read
			getreg(root->ptr1, reg1);
			out_linecount++;
			fprintf(fp, "IN %s\n", reg1);
			break;
		case '2':	//print
			codegen(root->ptr1);
			out_linecount++;
			fprintf(fp, "OUT T%d\n", regcount-1);
			regcount--;
			break;
		case '3':	//INLINE
			out_linecount++;
			fprintf(fp, "%s\n",root->ptr1->name);
			break;
		default:
			printf("Unknown Command %c\n", root->nodetype);		//Debugging
			return;
	}
}

void expandpath(char *path) // To expand environment variables in path
{
	char *rem_path = strdup(path);
	char *token = strsep(&rem_path, "/");
	if(rem_path!=NULL)
		sprintf(path,"%s/%s",getenv(++token)!=NULL?getenv(token):token-1,rem_path);
	else
		sprintf(path,"%s",getenv(++token)!=NULL?getenv(token):token-1);
}

void remfilename(char *pathname)
{
	int l = strlen(pathname);
	int i = l-1;	
	while(pathname[i] != '/' && i>=0)
	{
		i--;
	}
	pathname[i+1]='\0';	
}
