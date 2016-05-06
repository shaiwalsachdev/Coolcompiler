%{
/*
*	Parser specification 
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"symbol.h"
#include	"nodes.h"
#include	"codegen.h"
#include	"inter.h"

int		lineno	= 1;
extern int	yylex();	
extern char	*yytext;

int initial_pc;
struct ClassList *classlist;
/* the code and data segments */
TAC_ENTRY code[MAX_CODE];
DATA_ENTRY data[MAX_DATA];
int error=0;

void yyerror(char *s)
{
	fprintf(stderr,"Syntax error on line #%d: %s\n",lineno,s);
	fprintf(stderr,"Last token was \"%s\"\n",yytext);
	exit(1);
}

%}

%union{
	int		iValue;
	char		*cValue;
	struct node	*nptr;
};

%token	LPAREN RPAREN LBRACE RBRACE EQUAL NUMBER
%token	SEMICOLON DOTS COLON PLUS MINUS ASSIGN
%token	CLASS IS END TYPEID ID
%token	IF THEN ELSE FI WHILE LOOP POOL
%token	NOT BBEGIN DOT TRUE FALSE PRINT STRING

%type	<nptr>		class feature_list feature expr expr_list formal formal_list program param_list
%type 	<iValue>	TRUE FALSE NUMBER
%type	<cValue>	ID TYPEID STRING

%%
program		: c_list { 
		}
		;

c_list		: class SEMICOLON { 
			if ($1->type==ERROR) {
				error=1;
			}
		}
		| c_list class SEMICOLON {
			/* Our tree finished here */
			if ($2->type==ERROR) {
				error=1;
			}
		}
		;

class		: feature_list END { 
			st_exitclass();
			$$=mk_node_oper("CLASS",$1,NULL,NULL);
			$$->type=$1->type;
		}
		;

feature_list	: CLASS TYPEID IS {
			int st_idx;
			/* Add the class. Error if already present */
			st_idx=st_update($2,TYPE_CLASS,0,0);
			/* Create the current environment */
			st_enterclass();
			$$=mk_node_oper("FLIST-END",NULL,NULL,NULL);
			if (st_idx==ERROR) {
				printf("Error(%d): Redefinition of Class %s\n",lineno-1,(char*)$2);
				$$->type=ERROR;
			}
		}
		| feature_list feature SEMICOLON {
			$$=mk_node_oper("FLIST",$1,$2,NULL);
			if (($2->type)==ERROR) {
				$$->type=ERROR;
			}
			if (($1->type)==ERROR) {
				$$->type=ERROR;
			}
		}
		;

feature		: ID LPAREN formal_list RPAREN DOTS TYPEID IS {
			int lpos;
			/* Emit the method beginning and mark it
			   into the symbol table */
			lpos=code_emit(C_MBEGIN,0,0,0);

			/* Check that it's not already defined */
			if (st_update($1,TYPE_METHOD,st_lookup($6,TYPE_CLASS),lpos)==ERROR) {
				printf("Error(%d): Redefinition of Method %s::%s\n",lineno-1,st_getname(cl_current()),(char*)$1);
				error=1;
			}

			/* Is the main() method of Main? If so mark the initial PC */
			if ((strcmp($1,"main")==0) && (strcmp(st_getname(cl_current()),"Main")==0)) {
				initial_pc=lpos;
/*				printf("Found Main::main()\n"); */
			}
			
		 } expr_list END {
			/* Node with Name and parameters tree and code */
			$$=mk_node_oper($1,$3,$9,NULL);

/* Test code: prints out the parameters and their types 
			{	struct node *p=$3;
				printf("Method %s has:\n",$1);
				while(p) {	
					if (p->op2) printf("  argument %d of type %d\n",p->op2->st_pnt,p->op2->type);
					p=p->op1;
				}
				printf("\n");
			}
*/
			if ($9->type==ERROR) {
				$$->type=ERROR;
			} else {
				/*
				   Method type checking:
				   Last expression in the method code MUST have the
				   same type as method definition. To complete we
				   should do also check on parameters type and number (not implemented)
				*/
				if (st_lookup($6,TYPE_CLASS)==$9->type) {
					$$->type=$9->type;
				} else {
					printf("Error(%d): %s::%s returns different type from declaration\n",lineno-1,st_getname(cl_current()),$1);
					$$->type=ERROR;
				}
			}

			/* Emit end of method */
			code_emit(C_RETURN,0,0,0);
			code_emit(C_MEND,0,0,0);

		}
		| ID DOTS TYPEID {
			/* Add the attribute in the symbol table (and consequently to class definition) 
			   Just notify an error if already present.
			*/
			$$=mk_node_ide(st_gettype(st_lookup($3,TYPE_CLASS)),st_lookup($1,TYPE_ATTRIB));
			if (st_update($1,TYPE_ATTRIB,st_lookup($3,TYPE_CLASS),0)==ERROR) {
				printf("Error(%d): Redefinition of Attribute %s::%s\n",lineno-1,st_getname(cl_current()),(char*)$1);
				$$->type=ERROR;
			}

		}
		| ID DOTS TYPEID ASSIGN expr {
			/* Add the attribute in the symbol table (and consequently to class definition) 
			   Just notify an error if already present.
			*/
			$$=mk_node_ide(st_gettype(st_lookup($3,TYPE_CLASS)),st_lookup($1,TYPE_ATTRIB));
			if (st_update($1,TYPE_ATTRIB,st_lookup($3,TYPE_CLASS),0)==ERROR) {
				printf("Error(%d): Redefinition of Attribute %s::%s\n",lineno-1,st_getname(cl_current()),(char*)$1);
				$$->type=ERROR;
			}

		}
		;

formal_list	: formal_list formal {
			$$=mk_node_oper("Formal List",$1,$2,NULL);
		}
		| formal_list formal COLON {
			$$=mk_node_oper("Formal List",$1,$2,NULL);
		}
		| {
			$$=NULL;
		}
		;

formal		: ID DOTS TYPEID {
			st_update($1,TYPE_ARG,st_lookup($3,TYPE_CLASS),0);
			$$=mk_node_ide(st_gettype(st_lookup($3,TYPE_CLASS)),st_lookup($1,TYPE_ARG));
		}
		;

expr		: ID ASSIGN expr { 
			$$=mk_node_ide(st_gettype(st_lookup($1,TYPE_ATTRIB)),st_lookup($1,TYPE_CLASS));
			if (st_lookup($1,TYPE_ATTRIB)==ERROR) {
				printf("Error(%d): Undeclared variable %s used\n",lineno-1,$1);
				$$->type=ERROR;
			} else {
				if ($$->type == $3->type) {
					$$->value=$3->value;
				} else {
					printf("Error(%d): Incompatible variable assignment\n",lineno-1);
					$$->type=ERROR;
				}
			}
		}
		| ID DOT ID LPAREN param_list RPAREN {
			int st_pointer;
			/* Dispatch - object, method, params */
			$$=mk_node_oper("PROCEDURE CALL",$5,NULL,NULL);
			st_pointer=st_lookup($1,TYPE_ATTRIB);

			/* First check if the object is present in the running scope */
			if (st_pointer == -1) {
				printf("Error(%d): Variable %s not found\n",lineno-1,$1);
				$$->type=ERROR;
			} else {
				/* see in such an object there is such a method */
				int m_id=cl_inclass($3,TYPE_METHOD,st_gettype(st_pointer));
				if (m_id == ERROR) {
					printf("Error(%d): Method %s not in class %s\n",lineno-1,$3,$1);
					$$->type=ERROR;
				} else {
					/* Everything is ok, so emit the call to the code and
					   all the other needed stuff */
					code_emit(C_CALL,st_getoff(m_id),0,0);
		
					/* used for type-checking of the program (incomplete !!!) */
					$$->type=st_gettype(m_id);
				}
			}
		}
		| IF expr THEN expr ELSE expr FI { 
			$$=mk_node_oper("IF THEN ELSE",$2,$4,$6);
			if ($2->type!=st_lookup("Bool",TYPE_CLASS)) {
				printf("Error(%d): IF condition is not a boolean\n",lineno-1);
				$$->type=ERROR;
			} else {
				/* not dealing with supertipes, so just assume they are equal */
				$$->type=$4->type;
			}

		}
		| WHILE expr LOOP expr POOL { 
			$$=mk_node_oper("WHILE LOOP",$2,$4,NULL);
			if ($2->type!=st_lookup("Bool",TYPE_CLASS)) {
				printf("Error(%d): WHILE condition is not a boolean\n",lineno-1);
				$$->type=ERROR;
			} else {
				$$->type=$4->type;
			}
		}
		| BBEGIN expr_list END {
			$$=mk_node_oper("BLOCK",$2,NULL,NULL);
			$$->type=$2->type;
		}
		| NOT expr	{ 
			$$=mk_node_oper("NOT",$2,NULL,NULL);
			if ($2->type == ERROR) {
				$$->type=ERROR;
			} else {
				if ($2->type == st_lookup("Bool",TYPE_CLASS)) {
					$$->value=!($2->value);
					$$->type=st_lookup("Bool",TYPE_CLASS);
				} else {
					printf("Error(%d): NOT applied to a non boolean\n",lineno-1);
					$$->type=ERROR;
				}
			}
		}
		| ID		{ 
			if (st_lookup($1,TYPE_ATTRIB)==-1) {
				printf("Error(%d): Symbol %s undeclared before\n",lineno-1,$1);
			}
			$$=mk_node_ide(st_gettype(st_lookup($1,TYPE_ATTRIB)),st_lookup($1,TYPE_ATTRIB));
		}
		| NUMBER	{ 
			$$=mk_node_value(st_lookup("Int",TYPE_CLASS),$1);
			code_emit(C_CONST,$1,0,0);
		}
		| TRUE 		{ 
			$$=mk_node_value(st_lookup("Bool",TYPE_CLASS),1);
			code_emit(C_CONST,1,0,0);
		}
		| FALSE		{
			$$=mk_node_value(st_lookup("Bool",TYPE_CLASS),0);
			code_emit(C_CONST,0,0,0);
		}
		| PRINT STRING {
			/* first we insert the string in the symbol table and then
			   emit the code with the reference to it (not reference to
			   symbol table but to our data segment  */
			int str_mof;
			int st_nr=st_lookup($2,TYPE_STRING);
			if (st_nr==-1) {
				/* New string, must first insert  */
				str_mof=code_data($2);
				st_nr=st_update($2,TYPE_STRING,-1,str_mof);
			}
			/* Get offset in data segment (may be optimized) */
			str_mof=st_getoff(st_nr);
			code_emit(C_PRINT,str_mof,0,0);
			/* print is NOT a Cool standard expression. Let's assume
			   it's type is *always* Int and the value is the string
			   lenght excluding the quotes */
			$$=mk_node_value(st_lookup("Int",TYPE_CLASS),strlen($2)-2);
		}
		;

param_list	: param_list expr {}
		| param_list expr COLON {}
		| {}
		;

expr_list	: expr SEMICOLON { 
			$$=$1;
		}
		| expr_list expr SEMICOLON {
			/* the type of the expression list is the type of
			   the last expression or ERROR */
			$$=mk_node_oper("EXPRLIST",$1,$2,NULL);
			if ($1->type!=ERROR) {
				$$->type=$2->type;
			} else {
				$$->type=ERROR;
			}
		}
		;

%%

int main(int argc,char *argv[])
{
	classlist=NULL;
	
	/* clean up the symbol table */
	st_init();
	
	/* add the very basic types */
	st_update("Int",TYPE_CLASS,0,0);
	st_update("Bool",TYPE_CLASS,0,0);

	yyparse();

	if (error!=0) {
		printf("Errors during compilation, aborting execution\n");
		return -1;
	}

	/* at the end dump:
		the symbol table
		the classlist info
		the generated chunks of code
	*/
	st_dumpall();
	cl_showall();
	code_showall();

	if (initial_pc==0) {
		printf("main() in object Main not found, trying to execute from PC=0\n");
	} else {
		printf("Main::main() found at PC %d, executing\n",initial_pc);
	}

	printf("\nCleaning up internal structures:");
	printf(" Symbol table,");
	st_kill();
	printf(" Classlist\n");
	cl_kill();

	printf("\nExecuting the Compiler... The result are\n\n");
	inter_exec(initial_pc,code,data);

	return 0;
}
