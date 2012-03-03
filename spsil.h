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
struct define *root=NULL;
FILE *fp;
