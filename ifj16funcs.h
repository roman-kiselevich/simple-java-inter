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
 *         Inhliziian Bohdan
 * Description:
 *    Module for work with standard ifj16 language functions
 */

#ifndef IFJ16_FUNCS_H
#define IFJ16_FUNCS_H

#include <stdio.h>

enum ifj16_funcs {
   ifj16_read_int,
   ifj16_read_double,
   ifj16_read_string,
   ifj16_print,
   ifj16_length,
   ifj16_substr,
   ifj16_compare,
   ifj16_find,
   ifj16_sort,
 
   ifj16_not_ifj16, /* means that function is not ifj16 function */
};

/**
 * Detects if the identifier is a standard feature
 * @param Probable_ifj16 ID(identifier) for verification
 * @return standard function in the enum ifj16_funcs
 */
enum ifj16_funcs ifj16funcs_recognizer(char *probable_ifj16);

/**
 * Returns the ID of the appropriate(соотвествующей) function
 * @param func function represented by enum ifj16_funcs
 * @return Identifier of the function
 */
char *ifj16_get_func_id(enum ifj16_funcs func);

/**
 * adds standard ifj16 language functions in the symbol table
 */
void ifj16_add_funcs_to_symtab();

/**
 * Standard function ifj16.find ()
 * @param s string where we looking substring
 * @param search substring we are looking
 * @return If the substring was found it returns index of the first element 
 * in the string s, 
 * if the substring was not found, return -1
 */
int ifj16_find_method(char *s, char *search);

/**
 * Standard function ifj16.sort ()
 * @param str the string in which the characters are sort
 * @param n the number of characters
 */
void ifj16_merge_sort_method(char *str, size_t n);

#endif /* IFJ16_FUNCS_H */
