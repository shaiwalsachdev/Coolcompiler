#ifndef	NODES_H
#define	NODES_H

/*
  Class-list management
*/

/* 
   class list structure
*/
struct ClassList {
  int st_pnt;   /* pointer to symbol table for this class*/
  int *methods; /* pointer to an array containing the references to the ST of the methods */
  int *attribs; /* pointer to an array containing the references to the ST of the attributes */
  int attnr;    /* number of attributes present */
  int metnr;    /* number of methods present */
  struct ClassList *next; /* next element */
};

/*
  cl_addclass: add a class in our classlist. the class is the one referenced by the symbol
  table index st_pnt
*/
void cl_addclass(int st_pnt);

/*
  cl_addattmet: add an attribute or a method(depending on objtype value) to the current class.
  the attribute or a method to be added is referenced as a symbol table entry by st_pnt.
*/
void cl_addattmet(int st_pnt,int objtype);

/*
  cl_inclass: check if the method id_name is in the class classnr (symbol table index)
*/
int cl_inclass(char *id_name,int objtype,int classnr);

/*
  cl_current: returns the current class (as a symbol table index reference)
*/
int cl_current();

/*
  cl_showall: print out the classlist in a quasi readable form
*/
void cl_showall();

/*
  cl_kill: erases totally the contents of the classlist
*/
void cl_kill();

/* the class list herself */
extern struct ClassList *classlist;


/*	
 it is actually used just for compile-time
  type-checking
*/

#define NODE_IDE 0
#define NODE_VAL 1
#define NODE_OP  2

/* Generic node of our tree */
struct node{
  int nodetype; /* value, identifier or code */
  int type; 	/* type of data this thing returns (Int, Bool...) as a ref to the ST */
  int value;	/* calculated value if possible */
  int st_pnt;   /* pointer to symbol table entry*/
  char *name;   /* symbolic name */
  char *op_name; /* operator string for code generator */
  struct node *op1; /* + 3 operators */
  struct node *op2;
  struct node *op3;
};


struct node *mk_node_ide(int type,int st_idx);
struct node *mk_node_value(int type,int value);
struct node *mk_node_oper(char *name_op,struct node *op1,struct node *op2,struct node *op3);

#endif
