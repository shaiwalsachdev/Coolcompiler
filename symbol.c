/*	
*	Symbol table management
*/
#include <stdio.h>	
#include <string.h>	
#include <stdlib.h>
#include "symbol.h"
#include "nodes.h"

/* Definition of the static symbol table */
struct ST_ENTRY* symbol_table[MAX_HASH]; 
/* Support structure for the symbol table containing Conflicts number for each
   symbol table hash to generate OIDs */
int st_conflicts[MAX_HASH];

/* Current environment depth */
static int curr_env=0; 

/* Dynamic structure for mantaining the current scope definitions */
static unsigned int *curr_scope;
static unsigned int cs_entry;

/* Initialization of the symbol table */
void st_init() {
  int i;
  for (i=0;i<MAX_HASH;i++) {
    symbol_table[i]=NULL;
    st_conflicts[i]=0;
  }
}

/* Insert an entry in the symbol table */
int st_update(char *id_name, int objtype,int type,int offset) {
  int try_lookup;

  unsigned int cal_hash,oid;
  struct ST_ENTRY *ste;

  /* Check if the object is already present (the scope depends on the objtype,
     as defined later in st_lookup */
  try_lookup=st_lookup(id_name,objtype);

  if ((try_lookup!=-1) && ((objtype==TYPE_CLASS))) {
    /* symbol already present + it's a class:
       classes can't be redefined: ERROR
    */
    return ERROR ;
  }      
  if ((try_lookup!=-1) && ((objtype==TYPE_METHOD) || (objtype==TYPE_ATTRIB)) && (objtype==st_getobjtype(try_lookup)) ) {
    /*
       symbol already present in the same scope + it's a method/attrib
       methods/attribs can't be redefined: ERROR
       The check is a bit strange because we must leave the possibility of a method
       to have the same name as an attribute and viceversa as Cool Manual says
    */
    return ERROR;
  } else {
    /* Is it already defined and it's a constant string? */
    if (objtype==TYPE_STRING && try_lookup!=-1) {
      /* little optimization for constant strings:
 	  if we are inserting a constant string and it's already present, then
	  don't reinsert, just return the reference to the old one!
      */
      return try_lookup;
    }

    /* Calculate the HASH for this entry */    
    cal_hash=st_createhash(id_name);

    oid=cal_hash*MAX_SYM+st_conflicts[cal_hash];
    st_conflicts[cal_hash]++;

    /* The OID calculation is done considering a maximum number of entryes 
       possible per bucket. The OID is simply (HASH*MAX_SYM + position in bucket) 
    */
    if (oid-(cal_hash*MAX_SYM)>MAX_SYM) {
      printf("Error: bucket overlow\n");
      exit(-1);
    }

    /* Create an entry, fill it and prepend to the hashed entry */
    ste=(struct ST_ENTRY*)malloc(sizeof(struct ST_ENTRY));
    ste->id_name=strdup(id_name);
    ste->objtype=objtype;
    ste->nn=curr_env;
    ste->type=type;
    ste->offset=offset;
    ste->oid=oid;
    ste->next=symbol_table[cal_hash];
    symbol_table[cal_hash]=ste;

    /* Add this entry to current scope list */
    cs_entry++;
    curr_scope=(int *)realloc(curr_scope,sizeof(unsigned int)*cs_entry);
    curr_scope[cs_entry-1]=oid;

    /* If it's a class we must update our class list */
    if (objtype==TYPE_CLASS) {
      cl_addclass(oid);
      ste->type=oid;
      /* If it is a class attribute or method then we must update the class properties */
    } else if (objtype==TYPE_ATTRIB || objtype==TYPE_METHOD) {
      cl_addattmet(oid,objtype);
    }
  }  
  return oid;
}


/* Search for an entry in the symbol table 
   -> If the search type is TYPE_CLASS, search in entire table (type definitions are global)
   -> Otherwise it will be searched in the actual scope
   For expressions we should need to implement a browsing downward the nesting number to get the
   outermost definition
*/
int st_lookup(char* id_name,int objtype) {
  struct ST_ENTRY **runner;
  int cal_hash=st_createhash(id_name);

  /* Check in the calculated symbol table entry all the
     bucket entries */
  runner=&symbol_table[cal_hash];
  while (*runner!=NULL) {
    if (strcmp((*runner)->id_name,id_name)==0) {
      if (objtype==TYPE_CLASS || objtype==TYPE_STRING) {
	/* for CLASS/STRINGS no scope check*/
	if ((*runner)->objtype==objtype) {
	  return (*runner)->oid;
	}
      } else {
	/* Otherwise we must check if it is part of the current scope or not */
	int i;
	for (i=0;i<cs_entry;i++) {
	  if (((*runner)->oid)==curr_scope[i]) {
	    /* it IS in the current environment, so return the OID */
	    if ((*runner)->objtype==objtype) {
	      return (*runner)->oid;	    
	    }
	  }
	  /* Else it is a conflict that has nothing to do with the entry we
	     are searching for */
	}
      }
    }
    runner=&(*runner)->next;
  }

  /* otherwise not found */
  return -1;

}

/* Enter a new class */
void st_enterclass(){
  curr_env++;
  /* Clean current scope */
  curr_scope=NULL;
  cs_entry=0;
}


/* Exit current class */
void st_exitclass(){
  curr_env--;
  /* Clean current scope */
  cs_entry=0;
}


/* Helper routines */
int st_gettype(unsigned int st_oid) {
  return (*st_findoid(st_oid))->type;
}

int st_getobjtype(unsigned int st_oid) {
  return (*st_findoid(st_oid))->objtype;

}

int st_getoff(unsigned int st_oid) {
  return (*st_findoid(st_oid))->offset;

}
 
char *st_getname(unsigned int st_oid) {
  return (*st_findoid(st_oid))->id_name;

}

unsigned int st_createhash(char *id_name) {
  /* 
     Our hash function is quite simple:
      - Add char by char all the ASCII values of the string
      - Modulo MAX_HASH to stay in the table range
   */
  int temp_hash=0,i;
  for (i=0;i<strlen(id_name);i++) {
    temp_hash+=id_name[i];
  }
  return temp_hash%MAX_HASH;
}


struct ST_ENTRY **st_findoid(unsigned int oid) {
  struct ST_ENTRY **runner;
  unsigned int cal_hash=oid/MAX_SYM;
  runner=&symbol_table[cal_hash];
  while ((*runner)->oid!=oid) {          
    runner=&(*runner)->next;
  }
  return runner;
}

/* deletes all the entries (and conflicts) in the symbol table */
void st_kill() {
  int i;
  struct ST_ENTRY *runner,*deleter;
  for (i=0;i<MAX_HASH;++i) {
    runner=symbol_table[i];
    while (runner!=NULL) {
      deleter=runner;
      runner=runner->next;
      free(deleter);
    }
    symbol_table[i]=NULL;
  }
}

/* just write everything to the user */
void st_dumpall() {
  int	i;  
  struct ST_ENTRY **runner;
  printf("Generated symbol table\n");
  printf("STOID\tN.Nr\tObjT\tType\tSymbol\n");
  for (i=0;i<MAX_HASH;++i) {
    runner=&symbol_table[i];
    while (*runner!=NULL) {
      printf("%d\t%d\t%d\t%d\t%s\n",(*runner)->oid,(*runner)->nn,(*runner)->objtype,(*runner)->type,(*runner)->id_name);
      runner=&(*runner)->next;
    }
  }
  
}


