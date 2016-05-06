#ifndef CODEGEN_H
#define CODEGEN_H


/* 
   The Three address code entry structure
*/
typedef struct {
  int op;
  int arg1;
  int arg2;
  int res;   
} TAC_ENTRY;

typedef struct {
  char *string;
} DATA_ENTRY;

/* Maximum number of entries */
#define MAX_CODE 1000
#define MAX_DATA 1000

/* The structure itself */
extern TAC_ENTRY code[MAX_CODE];

/*  The data segment */
extern DATA_ENTRY data[MAX_DATA];

/* 
   Defines for the opcodes defined. Please see comments in inter.c for
   the description of each
*/
#define C_PARAM   1
#define C_CALL    2
#define C_MBEGIN  3
#define C_MEND    4
#define C_RETURN  5
#define C_CONST   6
#define C_PRINT   7


/* 
   code_emit: emits a quadruple (inserts it in our structure)
*/
extern int code_emit(int op, int arg1, int arg2, int res);

/* 
   code_data: emits a data segment entry (this is string) and
   returns the offset where it is placed
*/
extern int code_data(char *entry);


/*
  code_showall: display code in a readable form. code_getcode is a helper
  function that transforms integer definitions into readable strings.
*/
extern void code_showall();
extern char *code_getcode(int op);


#endif
