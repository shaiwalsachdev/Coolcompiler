/*
  Code generation
*/

#include <stdio.h>
#include <string.h>

#include "codegen.h"

/* pointer to current code insertion point */
static int n_code_pointer=0;

/* pointer to current data entry point */
static int n_data_pointer=0;

/* Simply emit a quadruple */
int code_emit(int op, int arg1, int arg2, int res) {
  code[n_code_pointer].op=op;
  code[n_code_pointer].arg1=arg1;
  code[n_code_pointer].arg2=arg2;
  code[n_code_pointer].res=res;
  return(n_code_pointer++);  
}


/* Inserts a DATA entry in the .data segment of the code (just strings) */
int code_data(char *entry) {
  data[n_data_pointer].string=strdup(entry);
  return(n_data_pointer++);
}


/* Show all the code in a readable way to the screen + the all the data accumulated */
void code_showall() {
  int lc=0;

  printf("\nGenerated code segment:\n");

  for (lc=0;lc<n_code_pointer;lc++) {
    printf("%d: %s, %d, %d, %d\n",lc,code_getcode(code[lc].op),code[lc].arg1,code[lc].arg2,code[lc].res);
    /* make it a bit more readable  */
    if (code[lc].op==C_MEND) { printf("\n"); }
  }

  printf("\nGenerated data segment:\n");
  for (lc=0;lc<n_data_pointer;lc++) {
    printf("%d: _string_ %s\n",lc,data[lc].string);
  }
  printf("\n");
}
  

/* returns a readable string from the OPcode constant value passed */
char *code_getcode(int op) {
  if (op==C_PARAM) { return("_param_"); }
  else if (op==C_CALL) { return("_call_"); }
  else if (op==C_MBEGIN) { return("_entermethod_"); }
  else if (op==C_MEND) { return("_exitmethod_"); }
  else if (op==C_RETURN) { return("_return_"); }
  else if (op==C_CONST) { return("_const_"); }
  else if (op==C_PRINT) { return("_print_"); }
  
  return("_unknown opcode_");
}
