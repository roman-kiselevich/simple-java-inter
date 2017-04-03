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


#include "soexpitems.h"
#include "restab.h"
#include "exprtree.h"
#include "cmptcheck.h"
#include "symtab.h"
#include "itrerror.h"

static void param_control_ifj16(instr_t *instr);

static void check_var_etree(etree_node_t *root);

static struct stack_of_expected_items {
   struct selem *arrel;
   unsigned count;
} *soexpitems_sg;

struct selem {
   char *var_key;
   char *func_key;
   bool expected;
   instr_t *instr;
};

void init_stack_exp_items() {
   soexpitems_sg = rtab_malloc(sizeof(*soexpitems_sg));
   soexpitems_sg->arrel = NULL;
   soexpitems_sg->count = 0;
}

enum { soei_realloc_cap = 32 };

void soeitems_add_item(char *new_var_key,
                       char *new_func_key,
                       bool expected,
                       instr_t *new_instr)
{

   if (soexpitems_sg->count % soei_realloc_cap == 0) {
      unsigned new_cap = soexpitems_sg->count + soei_realloc_cap;
      size_t new_size = sizeof(*soexpitems_sg->arrel)*new_cap;
      soexpitems_sg->arrel = rtab_realloc(soexpitems_sg->arrel, new_size);
   }

   soexpitems_sg->arrel[soexpitems_sg->count].var_key = new_var_key;
   soexpitems_sg->arrel[soexpitems_sg->count].func_key = new_func_key;
   soexpitems_sg->arrel[soexpitems_sg->count].expected = expected;
   soexpitems_sg->arrel[soexpitems_sg->count].instr = new_instr;
   ++soexpitems_sg->count;
}

static struct selem *pop_item() {
   if (soexpitems_sg->count == 0) {
      return NULL;
   }

   --soexpitems_sg->count;
   return &soexpitems_sg->arrel[soexpitems_sg->count];
}

instr_t *soeitems_pop_instr() {
   if (soexpitems_sg->count == 0) {
      return NULL;
   }

   --soexpitems_sg->count;
   return soexpitems_sg->arrel[soexpitems_sg->count].instr;   
}

void soeitems_stack_items_control() {
   struct selem *next_elem;
   char *var_key;
   char *func_key;
   bool expected;
   instr_t *instr;
   var_info_t *var_info;
   func_info_t *func_info;
   enum type_variable left_result;
   enum type_variable right_result;
   while (NULL != (next_elem = pop_item())) {
      var_key = next_elem->var_key;
      func_key = next_elem->func_key;
      expected = next_elem->expected;
      instr = next_elem->instr;
      switch (instr->t_instr) {
      case instr_assignment_expr:
         var_info = symtab_get_item_info(t_item_var, var_key);
         if (NULL == var_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find symbol \'%s\' in symtab", var_key);
         }

         var_info->init = true;
         instr->left_operand = var_info;

         if (expected) {
            etree_var_control(instr->right_etree->root);
            compatibility_check_var_etree(var_info->t_var, instr->right_etree);
         }

         var_info->init = true;
         break;
      case instr_assignment_call:
         var_info = symtab_get_item_info(t_item_var, var_key);
         if (NULL == var_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find symbol \'%s\' in symtab", var_key);            
         }
         var_info->init = true;
         instr->left_operand = var_info;

         func_info = symtab_get_item_info(t_item_func, func_key);
         if (NULL == func_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find function \'%s\'", var_key);              
         }

         compatibility_check_var_var(func_info->ret_type, var_info->t_var);
         if (func_info->num_args != instr->num_params) {
            itrerrno_g = semant_match_err;
            fatal_error("to few arguments");
         }

         if (func_info->ifj16_func == ifj16_not_ifj16) {
            for (unsigned i = 0; i < func_info->num_args; ++i) {
               etree_t *etree = instr->arretrees[i];
               var_info_t *arg_info = func_info->arrargs[i];
               compatibility_check_var_etree(arg_info->t_var, etree);
            }
         }
         else {
            param_control_ifj16(instr);
         }

         instr->func_info = func_info;
         break;
      case instr_call:
         func_info = symtab_get_item_info(t_item_func, func_key);
         if (NULL == func_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find function \'%s\'", var_key);              
         }

         if (func_info->num_args != instr->num_params) {
            itrerrno_g = semant_match_err;
            fatal_error("to few arguments");
         }

         if (func_info->ifj16_func == ifj16_not_ifj16) {
            for (unsigned i = 0; i < func_info->num_args; ++i) {
               etree_t *etree = instr->arretrees[i];
               var_info_t *arg_info = func_info->arrargs[i];
               compatibility_check_var_etree(arg_info->t_var, etree);
            }
         }
         else {
            param_control_ifj16(instr);
         }

         instr->func_info = func_info;      
         break;
      case instr_condition_gt:
      case instr_condition_geq:
      case instr_condition_lt:
      case instr_condition_leq:
      case instr_condition_eq:
      case instr_condition_noeq:
         etree_var_control(instr->left_etree->root);
         etree_var_control(instr->right_etree->root);
         left_result = etree_get_result_type(instr->left_etree);
         right_result = etree_get_result_type(instr->right_etree);
         if (left_result == t_var_string || right_result == t_var_string) {
            itrerrno_g = semant_other_err;
            fatal_error("cannot use strings in conditions");
         }
         break;
      case instr_ret:
         etree_var_control(instr->left_etree->root);
         left_result = instr->func_info->ret_type;
         compatibility_check_var_etree(left_result, instr->left_etree);
         break;
      default:
         printf("debug");
         break;
      } /* switch */
   }
}

static void param_control_ifj16(instr_t *instr) {
   switch (instr->func_info->ifj16_func) {
   case ifj16_read_int:
      break;
   case ifj16_read_double:
      break;
   case ifj16_read_string:
      break;
   case ifj16_print:
      check_var_etree(instr->arretrees[0]->root);
      break;
   case ifj16_length:
      compatibility_check_var_etree(t_var_string, instr->arretrees[0]);
      break;
   case ifj16_substr:
      break;
   case ifj16_compare:
      break;
   case ifj16_find:
      break;
   case ifj16_sort:
      break;
   default:
      ;
   }
}

static void check_var_etree(etree_node_t *root) {
   if (root && !root->left && !root->right) {
      token_t *tok = &root->tok;
      if (tok->t_type == t_id_simple || tok->t_type == t_id_complex) {
         char *key = tok->type_val.id_p;
         var_info_t *var_info = symtab_get_item_info(t_item_var, key);
         if (!var_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find symbol \'%s\' in symtab", key);
         }
      }

      return;
   }

   check_var_etree(root->left);
   check_var_etree(root->right);
}
