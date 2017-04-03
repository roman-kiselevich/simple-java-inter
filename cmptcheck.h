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
 *    The module checks the expression tree, 
 *    if the operations are carried out in expression of the types 
 *    defined for these types
 */

#ifndef COMPATIBILITY_CHECK_H
#define COMPATIBILITY_CHECK_H

#include "symtab.h"
#include "exprtree.h"

/**
 * Checks expression tree, 
 * if the operations are carried out in expression of the types 
 * defined for these types.
 * @param Etree pointer to the tree
 * @param Res_t resultant value type
 */
void compatibility_check_var_etree(enum type_variable res_t, etree_t* etree);

/**
 * Check the compatibility of the two types
 * @param Left the left-type value
 * @param Right the right type value
 */
void compatibility_check_var_var(enum type_variable left,
                                 enum type_variable right);

#endif /* COMPATIBILITY_CHECK_H */
