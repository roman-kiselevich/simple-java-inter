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
 * 		Precendence table parser
 */


#ifndef PRECTAB_H
#define PRECTAB_H

#include "exprtree.h"
#include "boolean.h"

/**
 * Initialize parser for expression
 */
void init_precedence_tab_parser();

/**
 * Converts expression ifj16 language to tree etree_t
 * @param first pointer to the first token expression
 * @param expected a pointer to the variable that the parser sets to true
 * If the variable that used in expression was not in the symbol table
 * @return expression tree
 */
etree_t *expr(token_t *first, bool *expected);

#endif /* PRECTAB_H */
