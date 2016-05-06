
/*
  Trees and class-list managing routines
*/

#include	<stdio.h>	
#include	<string.h>	
#include	<stdlib.h>	
#include	"coolc.tab.h"	
#include	"symbol.h"
#include	"nodes.h"

/*
  Class-list managing
*/

/* add a class to our classlist structure. simply puts it at the top and
   initializes various data */
void cl_addclass(int st_pnt) {
  struct ClassList *oldpnt;
  oldpnt=classlist;
  classlist=(struct ClassList*)malloc(sizeof(struct ClassList));
  classlist->st_pnt=st_pnt;
  classlist->attribs=NULL;
  classlist->methods=NULL;
  classlist->attnr=0;
  classlist->metnr=0;
  classlist->next=oldpnt;
}


/* adds an attribute or a method definition to a class  */
void cl_addattmet(int st_pnt,int type) {
  /* Assumes that we are inserting to "classlist", which is true for how the
     cl_addclass is written */
  if (type==TYPE_METHOD) {
    classlist->metnr++;
    classlist->methods=(int *)realloc(classlist->methods,(sizeof (unsigned int)*classlist->metnr));
    classlist->methods[classlist->metnr-1]=st_pnt;
  } else if (type==TYPE_ATTRIB) {
    classlist->attnr++;
    classlist->attribs=(int *)realloc(classlist->attribs,(sizeof (unsigned int)*classlist->attnr));
    classlist->attribs[classlist->attnr-1]=st_pnt;
  }
}


/* check if a method (passed as a string) is in a class (passed as a symbol table
   integer reference) or not */
int cl_inclass(char *id_name,int objtype, int classnr) {
  int i;
  struct ClassList *oldpnt=classlist;

  /* find the class we are interested in */
  while(oldpnt && oldpnt->st_pnt!=classnr) oldpnt=oldpnt->next;

  if (oldpnt==NULL) return ERROR;

  /* see if the requested method is in this class. if so return the symbol table index,
     otherwise return error  */
  if (objtype==TYPE_METHOD) {
    for (i=0;i<oldpnt->metnr;i++){
      if (strcmp(st_getname(oldpnt->methods[i]),id_name)==0) return oldpnt->methods[i];
    }
  }
  return ERROR;
}

/* returns the current class as a symbol table reference index */
int cl_current() {
	return classlist->st_pnt;
}


/* shows the class list in a readable form */
void cl_showall() {
  struct ClassList *pnt;
  int i;

  pnt=classlist;
  printf("\n\nClasslist structure\n");
  while(pnt) {
    printf("Class %s (OID=%d) has ",st_getname(pnt->st_pnt),pnt->st_pnt);
    if (pnt->attnr==0) printf("no ");
    printf("attributes ");
    for (i=0;i<pnt->attnr;i++) { printf ("%d ",pnt->attribs[i]); }
    printf("and ");
    if (pnt->metnr==0) printf("no ");
    printf("methods ");
    for (i=0;i<pnt->metnr;i++) { printf ("%d ",pnt->methods[i]); }
    printf("\n");
    pnt=pnt->next;
  }
}

/* deletes all the entries in classlist  */
void cl_kill() {
  struct ClassList  *runner,*deleter;
  
  runner=classlist;
  while (runner!=NULL) {
    deleter=runner;
    runner=runner->next;
    free(deleter);
  }
  classlist=NULL;
}


/*
  Trees managing.
*/

/* creates a node for an identifier */
struct node *mk_node_ide(int type,int st_idx) {
	struct node *p;
	p=(struct node *)malloc(sizeof(struct node));
	p->nodetype=NODE_IDE;
	p->st_pnt=st_idx;
	p->type=type;
	return p;
}

/* creates a node for a constant value of some type  */
struct node *mk_node_value(int type,int value) {
	struct node *p;
	p=(struct node *)malloc(sizeof(struct node));
	p->nodetype=NODE_VAL;
	p->value=value;
	p->type=type;
	return p;
}

/* creates a node for an operator */
struct node *mk_node_oper(char *name_op,struct node *op1,struct node *op2,struct node *op3) {
	struct node *p;
	p=(struct node *)malloc(sizeof(struct node));
	p->nodetype=NODE_OP;
	p->op_name=name_op;
	p->op1=op1;
	p->op2=op2;
	p->op3=op3;
	return p;
}
