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
 */


#include "ilist.h"
#include "restab.h"

struct instruction_list {
   instr_t *first_instr;
   instr_t *last_instr;
   instr_t ***arriaddr;
   unsigned count;
} *ilist_sg;


enum { ilist_realloc_cap = 8 };

void init_list_instructions() {
   instr_t main_func;
   main_func.t_instr = instr_call;
   instr_t *main_instr_ptr = ilist_init_instr(&main_func);
   ilist_sg = rtab_malloc(sizeof(*ilist_sg));
   ilist_sg->last_instr = main_instr_ptr;
   ilist_sg->first_instr = main_instr_ptr;
   ilist_sg->count = 0;
   main_instr_ptr->next = NULL;
   main_instr_ptr->num_params = 0;

}

void ilist_insert_first(instr_t *new_instr) {
   new_instr->next = ilist_sg->first_instr;
   ilist_sg->first_instr = new_instr;
}

void ilist_save_instr_addr(instr_t **i_addr) {
   if (ilist_sg->count % ilist_realloc_cap == 0) {
      unsigned new_cap = ilist_sg->count + ilist_realloc_cap;
      size_t new_size = sizeof(*ilist_sg->arriaddr)*new_cap;
      ilist_sg->arriaddr = rtab_realloc(ilist_sg->arriaddr, new_size);
   }

   ilist_sg->arriaddr[ilist_sg->count] = i_addr;
   ++ilist_sg->count;
}


instr_t **ilist_get_saved_instr_addr()
{
   if (ilist_sg->count == 0) {
      return NULL;
   }

   --ilist_sg->count;
   return ilist_sg->arriaddr[ilist_sg->count];
}

instr_t *ilist_init_instr(instr_t *item_to_append) {
   instr_t *new_instr = rtab_malloc(sizeof(*new_instr));
   new_instr->t_instr = item_to_append->t_instr;
   new_instr->next = NULL;
   switch (item_to_append->t_instr) {
   case instr_assignment_expr:
      new_instr->left_operand = item_to_append->left_operand;
      new_instr->right_etree = item_to_append->right_etree;
      new_instr->next = item_to_append->next;
      break;
   case instr_assignment_call:
      new_instr->num_params = item_to_append->num_params;
      new_instr->left_operand = item_to_append->left_operand;
      new_instr->arretrees = item_to_append->arretrees;
      new_instr->func_info = item_to_append->func_info;
      new_instr->next = item_to_append->next;
      break;
   case instr_call:
      new_instr->num_params = item_to_append->num_params;
      new_instr->arretrees = item_to_append->arretrees;
      new_instr->func_info = item_to_append->func_info;
      new_instr->next = item_to_append->next;   
      break;
   case instr_condition_gt:
   case instr_condition_geq:
   case instr_condition_lt:
   case instr_condition_leq:
   case instr_condition_eq:
   case instr_condition_noeq:
      new_instr->left_etree = item_to_append->left_etree;
      new_instr->right_etree = item_to_append->right_etree;
      new_instr->next_true = item_to_append->next_true;
      new_instr->next_false = item_to_append->next_false;
      break;
   case instr_ret:
      new_instr->right_etree = item_to_append->right_etree;
      break;
   }

   return new_instr;
}

instr_t *ilist_get_first() {
   return ilist_sg->first_instr;
}

void ilist_append_main_run(func_info_t *main_info) {
   ilist_sg->last_instr->func_info = main_info;
}
