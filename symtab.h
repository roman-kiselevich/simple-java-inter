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
 *   Module for work with symbol table
 */


#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "boolean.h"
#include "ifj16funcs.h"

/**
 * types of elements which stored in symbol table
 */
enum type_item { t_item_func, t_item_var, t_item_class };

/**
 * types of variables in the language ifj16
 */
enum type_variable { t_var_void, t_var_int, t_var_float, t_var_string };

typedef struct function_information {
   enum ifj16_funcs ifj16_func;

   struct variable_information **arrargs;
   unsigned num_args;
   struct instruction *first_instruction;
   enum type_variable ret_type;

   unsigned num_defvars;
   struct variable_information **arrvar_ptrs;
} func_info_t;

typedef struct variable_information {
   enum type_variable t_var;
   bool init;
   union var_info_val *var_ptr; /* when var is local (ptr to stack) */
} var_info_t;

/**
 * The value of any variable is represented as a union
 */
union var_info_val {
   char *string;
   int integer;
   double flpoint;
};

/**
 * ganerate key for identifier id
 * @param class_id id class to which the identifier id is referred
 * @param func_id if the identifier is a function
 * @param id the identifier for which we generate key
 * @return new key
 */
char *symtab_keygen(char *class_id, char *func_id, char *id);

/**
 * Initialize symbol table
 */
void init_symbol_table();

/**
 * @param t_search_item type of element
 * @param key_search_item key of element
 * @return pointer to the structure which contains information about element
 */
void *symtab_get_item_info(enum type_item t_search_item,
                           char *key_search_item);

/**
 * Put element into the table
 * @param t_new_item type of element
 * @param new_key key of element
 * @param item_info structure with information about element
 * @return pointer to the structure which contains information about element
 */
void *symtab_insert_item(enum type_item t_new_item, 
                         char *new_key, 
                         void *item_info);

#endif /* SYMBOL_TABLE_H */
