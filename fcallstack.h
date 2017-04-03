/*
 * IFJ project
 * Team leader: 
 *         Kiselevich Roman   xkisel00@stud.fit.vutbr.cz
 * Team:
 *         But Andrii         xbutan00@stud.fit.vutbr.cz
 *         Inhliziian Bohdan  xinhli00@stud.fit.vutbr.cz
 *         Niahodkin Pavel    xniaho00@stud.fit.vutbr.cz
 *         Kharytonov Danylo  xkhary00@stud.fit.vutbr.cz
 * 
 * The module was created by:
 *         Kiselevich Roman
 * Decription:
 *    Module for work with the stack of function calls 
 */

#ifndef FUNCTION_CALL_STACK_H
#define FUNCTION_CALL_STACK_H

#include "ilist.h"

/**
 * Initializes stack for work with functions
 */
void init_function_call_stack();

/**
 * Push function into the stack
 * @param Func_call pointer to a function call instruction
 */
void fcalls_push_func(instr_t *func_call);

/**
 * delete function from top of the stack
 * @return returns a pointer to the instruction of the previous function call
 */
instr_t *fcalls_pop_func();

#endif /* FUNCTION_CALL_STACK_H */
