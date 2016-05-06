
#ifndef INTER_H
#define INTER_H

#include "codegen.h"

/*
  inter_exec is our three address code.
  parameters passed are the initial program counter and a
  pointer to the array containing the code and one containing
  the 'data segment'
*/
extern void inter_exec(int initial_pc,TAC_ENTRY *code, DATA_ENTRY *data);

#endif
