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
 * Description:
 * 		Stack of unsigned integers 
 */


#ifndef STACK_OF_UINT_H
#define STACK_OF_UINT_H

#include "boolean.h"

typedef struct stack_of_uint soui_t;

/**
 * Initialize stack
 * @return returns pointer to a new stack
 */
soui_t *soui_init();

/**
 * Push element into stack
 * @param soui poiter to the stack
 * @param value element which we push
 */
void soui_push(soui_t *soui, unsigned value);

/**
 * @param pointer to the stack
 * @return element from the top of the stack
 */
unsigned soui_pop(soui_t *soui);

/**
 * @param pointer to the stack
 * @return true if stack is empty false if stack is not empty
 */
bool soui_empty(soui_t *soui);

#endif /* STACK_OF_UINT_H */
