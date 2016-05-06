#ifndef	SYMBOL_H
#define	SYMBOL_H
/*	
*	Symbol table management
*/


/* 
   Defines for the possible types of entries in the symbol table
*/
#define TYPE_CLASS  0x00 /* class */
#define TYPE_METHOD 0x01 /* method of a class */ 
#define TYPE_ATTRIB 0x02 /* attribute of a class */
#define TYPE_VAR    0x03 /* common variable */
#define TYPE_ARG    0x04 /* method argument */
#define TYPE_STRING 0x05 /* constant string */
#define ERROR -1

/* 
   Maximum number of symbols in each bucket (is needed for the calculation
   of the OID integer) and maximum dimension of the hash table
*/
#define MAX_SYM 50
#define MAX_HASH 37

/*
  An entry of the symbol table is defined as this:
*/
struct ST_ENTRY{
  char *id_name; /* id string */
  int objtype;   /* type of the object described */
  int type;      /* data type (if applicable) */		
  int nn;        /* nesting number */
  int offset;    /* offset where code is placed if needed */
  unsigned int oid; /* OID for the entry */
  struct ST_ENTRY *next; /* pointer for hash conflicts */
};


/*
  st_update: insert a symbol in the symbol table. the name of the symbol
  is id_name, the object type is objtype (one of the defines previously
  described), the type of data is type (this is a reference to a class
  in the symbol table itself) and offset is the offset in the code of
  the generated code (just used for methods in our case).
*/
extern int st_update(char* id_name, int objtype,int type,int offset);

/*
  st_lookup: search for the symbol id_name in the symbol table. the objtype
  (one of the defines previously described) tells us which kind of object
  we are searching and thus defines where we will look for it (for example
  class names are searched in the entire symbol table while the attributes
  just in the local class)
*/
extern int st_lookup(char* id_name, int objtype);


/*
  st_getname: returns the symbol name given the symbol table OID
*/
extern char *st_getname(unsigned int st_oid);


/*
  st_gettype: returns the type (data type!) of a symbol table entry given
  its OID
*/
extern int st_gettype(unsigned int st_oid);

/*
  st_objgettype: returns the type (object type!) of a symbol table entry given
  its OID
*/
extern int st_getobjtype(unsigned int st_oid);

/*
  st_getoff: returns the offset value of an entry in the symbol table given
  its OID
*/
extern int st_getoff(unsigned int st_oid);


/*
  st_createhash: given an identifier calculates an integer hash value for 
  symbol table storing
*/
extern unsigned int st_createhash(char *id_name);

/*
  st_findoid: given an OID (object ID integer) returns the pointer to that 
  physical object (does a HASH jump + a scan for conflicts).
*/
extern struct ST_ENTRY **st_findoid(unsigned int oid);

/*
  st_enterclass/st_exitclass: simulate the class entering/exiting. This means
  in our case taking note of the increasing nesting number and taking note of
  what has become now the local scope (this is, everything declared between an
  enter and and exit is the local scope).
*/
extern void st_enterclass();
extern void st_exitclass();

/*
  st_kill: erases the contents of the symbol table
*/
extern void st_kill();


/*
  st_init: initializes the symbol table
*/
extern void st_init();


/* 
   st_dumpall: dumps the entire symbol table in a semireadable form. Just for help 
   and debugging 
*/
extern void st_dumpall();

#endif
