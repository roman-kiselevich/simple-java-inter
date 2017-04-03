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

#include "inter.h"
#include "restab.h"
#include "symtab.h"
#include "itrerror.h"
#include "ilist.h"
#include "exprtree.h"
#include "fcallstack.h"
#include "soexpitems.h"
#include <string.h>

static void interpret_assignment_expr(instr_t *instr);

static union var_info_val interpret_call(instr_t *instr_call,
                                         enum type_variable *tretv_p);

static instr_t *interpret_condition(instr_t *instr);

static bool string_is_in_expr(etree_node_t *root);

static void interpret_ifj16_print_all_to_str(etree_node_t *root, 
                                             char **str_addr);

static union var_info_val interpret_ifj16_funcs(instr_t *instr, 
                                                enum type_variable *tretv_p);

static char *interpret_ifj16_substr(etree_t **arg_tree);

static int interpret_ifj16_compare(etree_t **arg_tree);

static int interpret_ifj16_find(etree_t **arg_tree);

static char *interpret_ifj16_sort(etree_t **arg_tree);

static char *interpret_ifj16_read_string();

void run_interpret() {
   char *main_key = "Main.run";
   func_info_t *main_info = symtab_get_item_info(t_item_func, main_key);
   if (!main_info) {
      itrerrno_g = semant_undef_err;
      fatal_error("cannot find main method \'%s\'", main_key);
   }

   ilist_append_main_run(main_info);
   instr_t *next_instr = ilist_get_first();
   for ( ; next_instr->t_instr != instr_call; next_instr = next_instr->next) {
      enum type_variable res_type;
      union var_info_val result;
      result = etree_get_expr_result(next_instr->right_etree->root, &res_type);

      var_info_t *left_op = next_instr->left_operand;
      if (!left_op->var_ptr) {
         left_op->var_ptr = rtab_malloc(sizeof(*left_op->var_ptr));
      }

      switch (res_type) {
      case t_var_int:
         if (left_op->t_var == t_var_float) {
            left_op->var_ptr->flpoint = (double) result.integer;
         }
         else if (left_op->t_var == t_var_int) {
            left_op->var_ptr->integer = result.integer;
         }
         break;
      case t_var_float:
         if (left_op->t_var == t_var_float) {
            left_op->var_ptr->flpoint = result.flpoint;
         }
         else if (left_op->t_var == t_var_int) {
            left_op->var_ptr->integer = (int)result.flpoint;
         }         
         break;
      case t_var_string:
         left_op->var_ptr->string = result.string;
         break;
      case t_var_void:
         soeitems_add_item(NULL, NULL, false, next_instr);
         continue;
      }
   }

   instr_t *exp_instr;
   while (NULL != (exp_instr = soeitems_pop_instr())) {
      enum type_variable res_type;
      union var_info_val result;
      result = etree_get_expr_result(exp_instr->right_etree->root, &res_type);

      var_info_t *left_op = exp_instr->left_operand;
      if (!left_op->var_ptr) {
         left_op->var_ptr = rtab_malloc(sizeof(*left_op->var_ptr));
      }

      switch (res_type) {
      case t_var_int:
         if (left_op->t_var == t_var_float) {
            left_op->var_ptr->flpoint = (double) result.integer;
         }
         else if (left_op->t_var == t_var_int) {
            left_op->var_ptr->integer = result.integer;
         }
         break;
      case t_var_float:
         if (left_op->t_var == t_var_float) {
            left_op->var_ptr->flpoint = result.flpoint;
         }
         else if (left_op->t_var == t_var_int) {
            left_op->var_ptr->integer = (int)result.flpoint;
         }         
         break;
      case t_var_string:
         left_op->var_ptr->string = result.string;
         break;
      case t_var_void:
         itrerrno_g = runtime_other_err;
         fatal_error("smth goes worng in interpret");
         continue;
      }      
   }

   enum type_variable main_ret_type = next_instr->func_info->ret_type;
   if (main_ret_type != t_var_void) {
      itrerrno_g = semant_match_err;
      fatal_error("Main.run return value must be void");
   }

   interpret_call(next_instr, NULL);
}

static union var_info_val interpret_call(instr_t *instr, 
                                         enum type_variable *tretv_p)
{
   union var_info_val res;
   if (instr->func_info->ifj16_func != ifj16_not_ifj16) {
      res = interpret_ifj16_funcs(instr, tretv_p);
      return res;
   }

   fcalls_push_func(instr);
   instr_t *instr_p = instr->func_info->first_instruction;
   enum type_variable res_type;
   var_info_t *left_op;
   while (instr_p) {
      switch (instr_p->t_instr) {
      case instr_assignment_expr:
         interpret_assignment_expr(instr_p);
         instr_p = instr_p->next;
         break;
      case instr_assignment_call:
         res = interpret_call(instr_p, &res_type);
         if (instr_p->func_info->ifj16_func == ifj16_not_ifj16) {
            instr_p = fcalls_pop_func();
         }         
         if (instr_p->t_instr == instr_assignment_call) {
            left_op = instr_p->left_operand;
            if (!left_op->var_ptr) {
               left_op->var_ptr = rtab_malloc(sizeof(*left_op->var_ptr));
            }

            switch (res_type) {
            case t_var_int:
               if (left_op->t_var == t_var_float) {
                  left_op->var_ptr->flpoint = (double) res.integer;
               }
               else if (left_op->t_var == t_var_int) {
                  left_op->var_ptr->integer = res.integer;
               }
               break;
            case t_var_float:
               if (left_op->t_var == t_var_float) {
                  left_op->var_ptr->flpoint = res.flpoint;
               }
               else if (left_op->t_var == t_var_int) {
                  left_op->var_ptr->integer = (int)res.flpoint;
               }         
               break;
            case t_var_string:
               left_op->var_ptr->string = res.string;
               break;
            case t_var_void:
               itrerrno_g = runtime_other_err;
               fatal_error("smth goes worng in interpret expr");
            }
         } /* if instr_assignment_call */
         instr_p = instr_p->next;
         break;
      case instr_call:
         interpret_call(instr_p, NULL);
         if (instr_p->func_info->ifj16_func == ifj16_not_ifj16) {
            instr_p = fcalls_pop_func();
         }
         instr_p = instr_p->next;
         break;
      case instr_condition_gt:
      case instr_condition_geq:
      case instr_condition_lt:
      case instr_condition_leq:
      case instr_condition_eq:
      case instr_condition_noeq:
         instr_p = interpret_condition(instr_p);
         break;
      case instr_ret:
         if (tretv_p) {
            res = etree_get_expr_result(instr_p->left_etree->root, tretv_p);
         }
         return res;
      }
   }

   if (tretv_p) {
      itrerrno_g = runtime_uninit_err;
      fatal_error("expected return operator in non void function");
   }

   return res;
}

static instr_t *interpret_condition(instr_t *instr) {
   enum type_variable left;
   enum type_variable right;
   etree_node_t *left_root = instr->left_etree->root;
   etree_node_t *right_root = instr->right_etree->root; 
   union var_info_val left_val = etree_get_expr_result(left_root, &left);
   union var_info_val right_val = etree_get_expr_result(right_root, &right);
   switch (left) {
   case t_var_int:
      if (right == t_var_int) {
         switch (instr->t_instr) {
         case instr_condition_gt:
            if (left_val.integer > right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;
         case instr_condition_geq:
            if (left_val.integer >= right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;            
         case instr_condition_lt:
            if (left_val.integer < right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;           
         case instr_condition_leq:
            if (left_val.integer <= right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_eq:
            if (left_val.integer == right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_noeq:
            if (left_val.integer != right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;         
         default:
            ;
         }
      }
      else if (right == t_var_float) {
         switch (instr->t_instr) {
         case instr_condition_gt:
            if ((double)left_val.integer > right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;
         case instr_condition_geq:
            if ((double)left_val.integer >= right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;            
         case instr_condition_lt:
            if ((double)left_val.integer < right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;           
         case instr_condition_leq:
            if ((double)left_val.integer <= right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_eq:
            if ((double)left_val.integer == right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_noeq:
            if ((double)left_val.integer != right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;         
         default:
            ;
         }
      }
      else {
         /* nothing */
      }
   case t_var_float:
      if (right == t_var_float) {
         switch (instr->t_instr) {
         case instr_condition_gt:
            if (left_val.flpoint > right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;
         case instr_condition_geq:
            if (left_val.flpoint >= right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;            
         case instr_condition_lt:
            if (left_val.flpoint < right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;           
         case instr_condition_leq:
            if (left_val.flpoint <= right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_eq:
            if (left_val.flpoint == right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_noeq:
            if (left_val.flpoint != right_val.flpoint) 
               return instr->next_true;
            else 
               return instr->next_false;         
         default:
            ;
         }
      }
      else if (right == t_var_int) {
         switch (instr->t_instr) {
         case instr_condition_gt:
            if (left_val.flpoint > (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;
         case instr_condition_geq:
            if (left_val.flpoint >= (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;            
         case instr_condition_lt:
            if (left_val.flpoint < (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;           
         case instr_condition_leq:
            if (left_val.flpoint <= (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_eq:
            if (left_val.flpoint == (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;          
         case instr_condition_noeq:
            if (left_val.flpoint != (double)right_val.integer) 
               return instr->next_true;
            else 
               return instr->next_false;         
         default:
            ;
         }
      }
      else {
         /* nothing */
      }
   default:
      ; /* nothing */
   }

   return NULL;
}

static void interpret_assignment_expr(instr_t *instr) {
   enum type_variable res_type;
   union var_info_val result;
   result = etree_get_expr_result(instr->right_etree->root, &res_type);

   var_info_t *left_op = instr->left_operand;
   if (!left_op->var_ptr) {
      left_op->var_ptr = rtab_malloc(sizeof(*left_op->var_ptr));
   }

   switch (res_type) {
   case t_var_int:
      if (left_op->t_var == t_var_float) {
         left_op->var_ptr->flpoint = (double) result.integer;
      }
      else if (left_op->t_var == t_var_int) {
         left_op->var_ptr->integer = result.integer;
      }
      break;
   case t_var_float:
      if (left_op->t_var == t_var_float) {
         left_op->var_ptr->flpoint = result.flpoint;
      }
      else if (left_op->t_var == t_var_int) {
         left_op->var_ptr->integer = (int)result.flpoint;
      }         
      break;
   case t_var_string:
      left_op->var_ptr->string = result.string;
      break;
   case t_var_void:
      itrerrno_g = runtime_other_err;
      fatal_error("smth goes worng in interpret expr");
   }    
}

static union var_info_val interpret_ifj16_funcs(instr_t *instr, 
                                                enum type_variable *tretv_p)
{
   union var_info_val res;
   enum type_variable type_res;
   func_info_t *func_info = instr->func_info;
   switch (func_info->ifj16_func) {
   case ifj16_read_int:
      if (instr->num_params != 0) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.readInt no params");
      }

      scanf("%d", &res.integer);
      *tretv_p = t_var_int;
      break;
   case ifj16_read_double:
      if (instr->num_params != 0) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.readDouble no params");
      }

      scanf("%lf", &res.flpoint);
      *tretv_p = t_var_float;
      break;
   case ifj16_read_string:
      if (instr->num_params != 0) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.String no params");
      }

      res.string = interpret_ifj16_read_string();
      *tretv_p = t_var_string; 
      break;
   case ifj16_print:
      if (instr->num_params != 1) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.print requare term or concat as "
                           "parametr");
      }

      char *print_str = rtab_malloc(32);
      *print_str = '\0';
      if (string_is_in_expr(instr->arretrees[0]->root)) {
         interpret_ifj16_print_all_to_str(instr->arretrees[0]->root, &print_str);
         printf("%s", print_str);
      }
      else {
         enum type_variable t_var;
         union var_info_val val;
         val = etree_get_expr_result(instr->arretrees[0]->root, &t_var);
         if (t_var == t_var_int) {
            snprintf(print_str, 32, "%d", val.integer);
         }
         else {
            snprintf(print_str, 32, "%g", val.flpoint);   
         }
         printf("%s\n", print_str);
      }
      rtab_free(print_str);
      break;
   case ifj16_length:
      if (instr->num_params != 1) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.length requare String param");
      }

      res = etree_get_expr_result(instr->arretrees[0]->root, &type_res);
      if (type_res != t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.length requare String param");
      }

      res.integer = strlen(res.string);
      *tretv_p = t_var_int;
      break;
   case ifj16_substr:
      if (instr->num_params != 3) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.substr requare 3 params");
      }

      res.string = interpret_ifj16_substr(instr->arretrees);
      *tretv_p = t_var_string;
      break;
   case ifj16_compare:
      if (instr->num_params != 2) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.compare requare 2 params");
      }

      res.integer = interpret_ifj16_compare(instr->arretrees);
      *tretv_p = t_var_int;
      break;
   case ifj16_find:
      if (instr->num_params != 2) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.find requare 2 params");
      }

      res.integer = interpret_ifj16_find(instr->arretrees);
      *tretv_p = t_var_int;
      break;
   case ifj16_sort:
      if (instr->num_params != 1) {
         itrerrno_g = semant_match_err;
         fatal_error("method ifj16.sort requare 1 params");
      }

      res.string = interpret_ifj16_sort(instr->arretrees);
      *tretv_p = t_var_string;
      break;
   default:
      ; /* nothing to do */
   }

   return res;
}

static char *interpret_ifj16_read_string() {
   char buff[300];
   char *str;
   str = fgets(buff, sizeof(buff), stdin);
   str[strlen(str)-1] = '\0';
   char *new_str = rtab_malloc(strlen(str) + 1);
   strncpy(new_str, str, strlen(str) + 1);
   return new_str;
}

static char *interpret_ifj16_sort(etree_t **arg_tree) {
   enum type_variable str_tvar;
   union var_info_val str_val;
   char *str;
   char *func_name = "ifj16.sort";

   str_val = etree_get_expr_result(arg_tree[0]->root, &str_tvar);
   if (str_tvar == t_var_string) {
      str = str_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s require String as first param", func_name);
   }

   char *new_str = rtab_malloc(strlen(str) + 1);
   strncpy(new_str, str, strlen(str) + 1);
   ifj16_merge_sort_method(new_str, strlen(new_str));
   return new_str;
}

static int interpret_ifj16_find(etree_t **arg_tree) {
   enum type_variable s_tvar, search_tvar;
   union var_info_val s_val, search_val;
   char *s, *search;
   char *func_name = "ifj16.find";

   s_val = etree_get_expr_result(arg_tree[0]->root, &s_tvar);
   if (s_tvar == t_var_string) {
      s = s_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s method require String as first param", func_name);
   }

   search_val = etree_get_expr_result(arg_tree[1]->root, &search_tvar);
   if (search_tvar == t_var_string) {
      search = search_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s method require String as first param", func_name);
   }

   int ret = ifj16_find_method(s, search);
   return ret;
}

static int interpret_ifj16_compare(etree_t **arg_tree) {
   enum type_variable s1_tvar, s2_tvar;
   union var_info_val s1_val, s2_val;
   char *s1, *s2;
   char *func_name = "ifj16.compare";

   s1_val = etree_get_expr_result(arg_tree[0]->root, &s1_tvar);
   if (s1_tvar == t_var_string) {
      s1 = s1_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s method require String as first param", func_name);
   }

   s2_val = etree_get_expr_result(arg_tree[1]->root, &s2_tvar);
   if (s2_tvar == t_var_string) {
      s2 = s2_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s method require String as first param", func_name);
   }

   int ret = strcmp(s1, s2);
   return (ret > 0) - (ret < 0);
}

static char *interpret_ifj16_substr(etree_t **arg_tree) {
   char *func_name = "ifj16.substr";
   int i, n;
   char *str;
   enum type_variable i_tvar, n_tvar, str_tvar;
   union var_info_val i_val, n_val, str_val;

   str_val = etree_get_expr_result(arg_tree[0]->root, &str_tvar);
   if (str_tvar == t_var_string) {
      str = str_val.string;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s require String as first param", func_name);
   }

   i_val = etree_get_expr_result(arg_tree[1]->root, &i_tvar);
   if (i_tvar == t_var_int) {
      i = i_val.integer;
   }
   else if (i_tvar == t_var_float) {
      i = (int) i_val.flpoint;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s require int as second param", func_name);
   }

   n_val = etree_get_expr_result(arg_tree[2]->root, &n_tvar);
   if (n_tvar == t_var_int) {
      n = n_val.integer;
   }
   else if (n_tvar == t_var_float) {
      n = (int) n_val.flpoint;
   }
   else {
      itrerrno_g = semant_match_err;
      fatal_error("%s require int as third param", func_name);
   }

   if (i < 0 || i > n || n > (int)strlen(str)) {
      itrerrno_g = runtime_other_err;
      fatal_error("bad params in function %s", func_name);
   }

   char *beg_substr = str + i;
   char *end_substr = str + n;
   char *new_str = rtab_malloc(n - i  + 1);
   for (char *tmp = new_str; end_substr != beg_substr; ++beg_substr, ++tmp) {
      *tmp = *beg_substr;
   }

   new_str[n - i] = '\0';
   return new_str;
}

static bool string_is_in_expr(etree_node_t *root) {
   if (!root->left && !root->right) {
      var_info_t *var_info;
      char *key = root->tok.type_val.id_p;
      switch (root->tok.t_type) {
      case t_id_simple:
      case t_id_complex:
         var_info = symtab_get_item_info(t_item_var, key);
         if (var_info->t_var == t_var_string)
            return true;
         else
            return false;
      case t_literal:
         if (root->tok.type_val.literal == string_l)
            return true;
         else
            return false;
      default:
         return false;
      }
   }

   bool left = string_is_in_expr(root->left);
   bool right = string_is_in_expr(root->right);
   if (left || right)
      return true;
   else
      return false;
}

static void interpret_ifj16_print_all_to_str(etree_node_t *root, 
                                             char **str_addr)
{
   if (!root->left && !root->right) {
      enum type_variable t_var;
      union var_info_val res = etree_get_var_tok(&root->tok, &t_var);

      size_t add_str_size = 0;
      if (t_var == t_var_string) {
         add_str_size = strlen(res.string);
      }
      size_t add_size = strlen(*str_addr);

      *str_addr = rtab_realloc(*str_addr, add_str_size + add_size + 32);
      char *string = *str_addr;
      string += add_size;
      switch (t_var) {
      case t_var_int:
         snprintf(string, add_size + 32, "%d", res.integer);
         break;
      case t_var_float:
         snprintf(string, add_size + 32, "%g", res.flpoint);
         break;
      case t_var_string:
         snprintf(string, add_str_size + add_size + 32, "%s", res.string);
         break;
      default:
         ;
      }

      return;
   }

   if (root->tok.type_val.op != plus_o) {
      itrerrno_g = semant_match_err;
      fatal_error("method ifj16.print requare term or concat as "
                                                   "parametr");
   }

   interpret_ifj16_print_all_to_str(root->left, str_addr);
   interpret_ifj16_print_all_to_str(root->right, str_addr);
}
