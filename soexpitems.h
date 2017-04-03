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
 *    Stack of expected items
 */



#ifndef STACK_OF_EXPECTED_ITEMS
#define STACK_OF_EXPECTED_ITEMS

#include "symtab.h"
#include "exprtree.h"
#include "ilist.h"

/**
 * Initialize stack of expected elements
 */
void init_stack_exp_items();

/**
 * push element to stack
 * @param new_var_key ID of element if element - variable push
 * element to stack
 * @param new_func_key ID of element if element - function
 * @param new_instr pointer to instraction to which this element belong
 */
void soeitems_add_item(char *new_var_key,
                       char *new_func_key,
                       bool expected,
                       instr_t *new_instr);

/**
 * returns instruction to which element from top of the stack belongs
 * @return
 */
instr_t *soeitems_pop_instr();

/**
 * Check the compatibility of each type of element
 */
void soeitems_stack_items_control();

#endif
