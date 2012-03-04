#include<string.h>
#define LEGAL 0
extern int linecount;
int flag_alias=LEGAL;
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
struct define *define_root=NULL;
FILE *fp;
char alias_table[8][30];

struct define* lookup_constant(char *name)
{
	struct define *temp=define_root;	
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
		strcpy(temp->name,name);
		temp->value=value;
		temp->next=define_root;
		define_root=temp;
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













