#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inter.h"
#include "codegen.h"

/* This is the fixed activation record structure and should contain
   whatever needed. In our case just the Return Code */
struct MyStack{
  int ra; /* Return Code */
  struct MyStack *next;
};


/* The stack structure itself */
struct MyStack *stack;

/* File for Interpreter simulation output */
FILE *intout;

/* Put an activation record on stack, displaying it after insertion */
void i_push(int ra) {
  struct MyStack *oldpnt;
  struct MyStack *scan;

  oldpnt=stack;
  stack=(struct MyStack*)malloc(sizeof(struct MyStack));
  stack->ra=ra;
  stack->next=oldpnt;

  /* before exiting display the entire stack */
  fprintf(intout,"\t\t\t\t\t\tCurrent Stack: ");
  scan=stack;
  while (scan) {
    fprintf(intout,"%d ",scan->ra);
    scan=scan->next;
  }
  fprintf(intout,"\n");

}

/* Remove an activation record from stack returning the RC */
int i_pop() {
  int ra;
  struct MyStack *scan;

  /* before POPping display the entire stack */
  fprintf(intout,"\t\t\t\t\t\tCurrent Stack: ");
  scan=stack;
  while (scan) {
    fprintf(intout,"%d ",scan->ra);
    scan=scan->next;
  }
  fprintf(intout,"\n");

  if (stack==NULL) {
    /* End of program reached (or something went wrong) */
    ra=-1;
  } else {
    struct MyStack *temp;
    /* Else get the RC and delete the entry*/
    ra=stack->ra;
    temp=stack;
    stack=stack->next;
    free(temp);
  }
  return ra;
}


/* The interpreter for our limited three address code */
void inter_exec(int initial_pc, TAC_ENTRY *code, DATA_ENTRY *data) {
  int end=0;
  int last_eval=-1;

  /* stack initialization */
  stack=NULL; 

  /* Write the interpreter states to a debug file */
  intout=fopen("out-int.txt","w");
  fprintf(intout,"PC\tDescription\n");

  while (end==0) {
    /* select the codes and act */
    if (code[initial_pc].op==C_MBEGIN) {
      /* 
	 - Method beginning 
	 First instruction of a method. Some preparations should be put
	 here. In our Interpreter: do nothing, just notice it for debug.
      */
      fprintf(intout,"%d:\tEntered in a method\n",initial_pc);
      initial_pc++;
    } else if (code[initial_pc].op==C_MEND) {
      /* 
	 - Method ending 
	 Last instruction of a method. This is just for debugging, since
	 we shouldn't arrive at this point (a return should be found before).
      */
      fprintf(intout,"%d:\tExiting from a method\n",initial_pc);
      initial_pc++;
    } else if (code[initial_pc].op==C_CALL) {
      /* 
	 - Call to a method 
	 First of all we prepare the activation record (which is just the return
	 address in our machine) and put it on the stack. Then we just 
	 jump on the method address, provided as the first argument.
      */
      fprintf(intout,"%d:\tPushing RA %d on stack and jumping to %d\n",initial_pc,initial_pc+1,code[initial_pc].arg1);
      /* generate activation record, return code is the instruction after the
         call, so PC + 1 */
      i_push(initial_pc+1);
      /* jump to the method */
      initial_pc=code[initial_pc].arg1;
    } else if (code[initial_pc].op==C_RETURN) {
      /*
	- Return from a method 
	First of all we retrieve the last activation record (pop it from the
	stack) which is in our case just the return address. Then we just jump
	to the return address. If there is no activation record on the stack then
	we came to end of execution (or something bad happened, most likely because
	the Main::main() function wasn't defined and we just started with PC=0)
      */
      int ra;
      /* get return address from our mini activation record */      
      ra=i_pop();
      fprintf(intout,"%d:\tReturning to %d retrieved from stack, return value should be %d\n",initial_pc,ra,last_eval);      
      /* If stack is empty, end */
      if (ra==-1) {
	fprintf(intout,"END\tReturn address -1: end of program reached\n");      
	end=1;
      } else {
	/* else jump to the return address */
	initial_pc=ra;
      }
    } else if (code[initial_pc].op==C_CONST) {
      /* 
	 - Constant value to evalutate 
	 We will just update our last evalutated (variable last_eval) value with this value.
	 This should be the return code (or anyway the expression value) if we'd implement
	 it.	 
      */
      fprintf(intout,"%d:\tEvalutating constant %d\n",initial_pc,code[initial_pc].arg1);
      last_eval=code[initial_pc].arg1;
      initial_pc++;
    } else if (code[initial_pc].op==C_PRINT) {
      /* 
	 - Print a string to screen 
	 First of all we retrieve the string from the symbol table. The data segment entry
	 is passed as the first argument. Then we just get rid of the enclosing quotes
	 and print the string to the user followed by a newline.
	 After this we update our last evalutated (variable last_eval) value with the
	 length of the string printed.
      */
      char *pr_string;
      pr_string=strdup(data[code[initial_pc].arg1].string);
      fprintf(intout,"%d:\tPrinting string %s to screen\n",initial_pc,pr_string);
      /* get off the enclosing quotes from string before displaying */
      pr_string++;
      pr_string[strlen(pr_string)-1]='\0';
      /* display */
      printf("%s\n",pr_string);
      /* as defined for our type checking (returns Int,strlen) */
      last_eval=strlen(pr_string);
      initial_pc++;
    } else {
      /*
	- Unknown opcode:  
	Error: exit
      */
      fprintf(intout,"\nFatal ERROR: unknown opcode found\n");
      initial_pc=-1;
    }
  }

  fclose(intout);
}
