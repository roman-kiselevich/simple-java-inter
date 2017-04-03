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
 *    Module for work with insructions
 */


#ifndef ILIST_H
#define ILIST_H

#include "exprtree.h"
#include "symtab.h"
#include "boolean.h"

enum type_inst {
   instr_assignment_expr,
   instr_assignment_call,
   instr_call,

   instr_condition_gt,
   instr_condition_geq,
   instr_condition_lt,
   instr_condition_leq,
   instr_condition_eq,
   instr_condition_noeq,

   instr_ret,
};

typedef struct instruction {
   enum type_inst t_instr;
   
   unsigned num_params;

   union {
      struct variable_information *left_operand;
      etree_t *left_etree;
   };

   union {
      struct function_information *func_info;
      etree_t *right_etree;
   };

   union {
      etree_t **arretrees;
      struct instruction *next_false;
   };
   
   union {
      struct instruction *next_true;
      struct instruction *next;
   };

} instr_t;


/**
 * Initializes a list of instructions
 */
void init_list_instructions();

/**
 * Return first instruction
 * @return first instruction
 */
instr_t *ilist_get_first();

/**
 * Add in the end of the list of instuction function call Main.run()
 * @param information about function Main.run() 
 */
void ilist_append_main_run(func_info_t *main_info);

/**
 * сохраняет адрес инструкции в буфере 
 * @param адрес инструкции который хотим сохранить
 */
void ilist_save_instr_addr(instr_t **i_addr);

/**
 * return address of saved instruction
 * @return address of saved instruction
 */
instr_t **ilist_get_saved_instr_addr();

/**
 * Initializes a new instruction in dynamic memory and returns a pointer to it
 * @param pointer to instruction that we want to initialize
 * @return pointer to a new instuction
 */
instr_t *ilist_init_instr(instr_t *item_to_append);

/**
 * 
 * Insert the instructions at the beginning of the list of instructions
 * @param Pointer to instructions
 */
void ilist_insert_first(instr_t *item_to_append);

#endif /* ILIST_H */
