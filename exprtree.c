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
#include "exprtree.h"
#include "restab.h"
#include "itrerror.h"
#include <string.h>

static enum type_variable get_expr_type(etree_node_t *root);
static enum type_variable get_token_type(token_t *tok);

etree_node_t *etree_create_node(token_t *tok_to_node) {
   etree_node_t *new_node = rtab_malloc(sizeof(*new_node));
   new_node->tok.t_type = tok_to_node->t_type;
   new_node->tok.val = tok_to_node->val;
   new_node->tok.type_val = tok_to_node->type_val;
   new_node->left = NULL;
   new_node->right = NULL;
   return new_node;
}

etree_node_t *etree_connect_nodes(token_t *tok_to_node,
                                  etree_node_t *left, 
                                  etree_node_t *right) 
{
   etree_node_t *new_node = etree_create_node(tok_to_node);
   new_node->left = left;
   new_node->right = right;
   return new_node;
}

etree_t *etree_init(etree_node_t *init_root) {
   etree_t *new_tree = rtab_malloc(sizeof(*new_tree));
   if (init_root) {
      new_tree->root = init_root;
   }
   else {
      new_tree = NULL;
   }
   return new_tree;
}

enum type_variable etree_get_result_type(etree_t *etree) {
   enum type_variable res_type;
   res_type = get_expr_type(etree->root);
   return res_type;
}

void etree_var_control(etree_node_t *root) {
   if (!root->left && !root->right) {
      if (root->tok.t_type == t_id_simple || root->tok.t_type == t_id_complex) {
         char *key = root->tok.type_val.id_p;
         var_info_t *var_info = symtab_get_item_info(t_item_var, key);
         if (NULL == var_info) {
            itrerrno_g = semant_undef_err;
            fatal_error("cannot find symbol \'%s\' in symtab", key);
         }
         if (var_info->init == false) {
            itrerrno_g = runtime_uninit_err;
            fatal_error("cannot use uninitialized vars in exprs");
         }
      }   

      return ;
   }

   etree_var_control(root->left);
   etree_var_control(root->right);
}

static enum type_variable get_expr_type(etree_node_t *root) {
   if (!root->left && !root->right) {
      return get_token_type(&root->tok);
   }

   enum type_variable left_type = get_expr_type(root->left);
   enum type_variable right_type = get_expr_type(root->right);
   int operator;
   if (root->tok.type_val.op == plus_o) {
      operator = '+';
   }
   else {
      operator = 0;
   }

   switch (left_type) {
   case t_var_int:
      if (right_type == t_var_int)         return t_var_int;
      else if (right_type == t_var_float)  return t_var_float;
      else if (right_type == t_var_string) return t_var_void;
      else                                 return t_var_void;
   case t_var_float:
      if (right_type == t_var_int)         return t_var_float;
      else if (right_type == t_var_float)  return t_var_float;
      else if (right_type == t_var_string) return t_var_void;
      else                                 return t_var_void;      
   case t_var_string:
      if (right_type == t_var_int)         return t_var_void;
      else if (right_type == t_var_float)  return t_var_void;
      else if (right_type == t_var_string) {
         if (operator != '+') {
            itrerrno_g = semant_match_err;
            fatal_error("attempt to use strings with operators "
                           "that are not defined for String type.");
         }
         return t_var_string;
      }
      else 
         return t_var_void;
      break;
   case t_var_void:
      return t_var_void;
   }

   return t_var_void;
}

static enum type_variable get_token_type(token_t *tok) {
   if (tok->t_type == t_literal) {
      switch(tok->type_val.literal) {
      case int_l:     return t_var_int;
      case float_l:   return t_var_float;
      case string_l:  return t_var_string;   
      }
   }
   else if (tok->t_type == t_id_simple || tok->t_type == t_id_complex) {
      char *key = tok->type_val.id_p;
      var_info_t *var_info = symtab_get_item_info(t_item_var, key);
      if (var_info) {
         return var_info->t_var;       
      }
      else {
         itrerrno_g = semant_undef_err;
         fatal_error("cannot find symbol \'%s\' in symtab", key);
      }
   }
   else {
      itrerrno_g = iternal_compile_err;
      fatal_error("smth goes wrong in get_token_type second :(");
   }

   return t_var_void;
}

union var_info_val etree_get_expr_result(etree_node_t *root, 
                                         enum type_variable *tvar_p)
{
   if (!root->left && !root->right) {
      union var_info_val res;
      res = etree_get_var_tok(&root->tok, tvar_p);
      return res;
   }

   union var_info_val result;
   result.string = NULL;
   enum type_variable left_type;
   union var_info_val left = etree_get_expr_result(root->left, &left_type);
   enum type_variable right_type;
   union var_info_val right = etree_get_expr_result(root->right, &right_type);
   if (left_type == t_var_void || right_type == t_var_void) {
      return result;
   }

   int operator;
   switch (root->tok.type_val.op) {
   case plus_o:
      operator = '+';
      break;
   case minus_o:
      operator = '-';
      break;
   case mult_o:
      operator = '*';
      break;
   case div_o:
      operator = '/';
      break;
   }

   switch (left_type) {
   case t_var_int:
      if (right_type == t_var_int) {
         *tvar_p = t_var_int;
         switch (operator) {
         case '+':
            result.integer = left.integer + right.integer;
            break;
         case '-':
            result.integer = left.integer - right.integer;
            break;
         case '*':
            result.integer = left.integer * right.integer;
            break;
         case '/':
            if (right.integer == 0) {
               itrerrno_g = runtime_zerodiv_err;
               fatal_error("divide by zero"); 
            }
            result.integer = left.integer / right.integer;
            break;
         }
      }
      else if (right_type == t_var_float) {
         *tvar_p = t_var_float;
         switch (operator) {
         case '+':
            result.flpoint = (double)left.integer + right.flpoint;
            break;
         case '-':
            result.flpoint = (double)left.integer - right.flpoint;
            break;
         case '*':
            result.flpoint = (double)left.integer * right.flpoint;
            break;
         case '/':
            if (right.flpoint == 0.0) {
               itrerrno_g = runtime_zerodiv_err;
               fatal_error("divide by zero");
            }
            result.flpoint = (double)left.integer / right.flpoint;
            break;
         }
      }
      else {
         tvar_p = t_var_void;
         return result;
      }
      break;
   case t_var_float:
      if (right_type == t_var_int) {
         *tvar_p = t_var_float;
         switch (operator) {
         case '+':
            result.flpoint = left.flpoint + (double)right.integer;
            break;
         case '-':
            result.flpoint = left.flpoint - (double)right.integer;
            break;
         case '*':
            result.flpoint = left.flpoint * (double)right.integer;
            break;
         case '/':
            if (right.integer == 0) {
               itrerrno_g = runtime_zerodiv_err;
               fatal_error("divide by zero");
            }
            result.flpoint = left.flpoint / (double)right.integer;
            break;
         }
      }
      else if (right_type == t_var_float) {
         *tvar_p = t_var_float;
         switch (operator) {
         case '+':
            result.flpoint = left.flpoint + right.flpoint;
            break;
         case '-':
            result.flpoint = left.flpoint - right.flpoint;
            break;
         case '*':
            result.flpoint = left.flpoint * right.flpoint;
            break;
         case '/':
            if (right.flpoint == 0.0) {
               itrerrno_g = runtime_zerodiv_err;
               fatal_error("divide by zero");
            }
            result.flpoint = left.flpoint / right.flpoint;
            break;
         }
      }
      else {
         *tvar_p = t_var_void;
         return result;
      }      
      break;
   case t_var_string:
      *tvar_p = t_var_string;
      if (right_type == t_var_string) {
         size_t len_first = strlen(left.string);
         size_t len_second = strlen(right.string);
         size_t result_len = len_first + len_second + 1;
         result.string = rtab_realloc(result.string, result_len);
         snprintf(result.string, result_len, "%s%s", left.string, right.string);
      }
      else {
         tvar_p = t_var_void;
         return result;
      }      
      break;
   case t_var_void:
      tvar_p = t_var_void;
      return result;
      break;
   }

   return result;
}

union var_info_val etree_get_var_tok(token_t *tok, enum type_variable *tvar_p) {
   char *key;
   union var_info_val val;
   var_info_t *var_info;
   switch (tok->t_type) {
   case t_literal:
      if (tok->type_val.literal == int_l) {
         *tvar_p = t_var_int;
         val.integer = tok->val.integer;
      }
      else if (tok->type_val.literal == float_l) {
         *tvar_p = t_var_float;
         val.flpoint = tok->val.f_pnt;
      }
      else {
         *tvar_p = t_var_string;
         val.string = tok->val.str;         
      }
      break;
   case t_id_simple:
   case t_id_complex:
      key = tok->type_val.id_p;
      var_info = symtab_get_item_info(t_item_var, key);
      if (var_info->var_ptr) {
         *tvar_p = var_info->t_var;
         if (*tvar_p == t_var_int) {
            val.integer = var_info->var_ptr->integer;
         }
         else if (*tvar_p == t_var_float) {
            val.flpoint = var_info->var_ptr->flpoint;  
         }
         else {
            val.string = var_info->var_ptr->string;
         }
      }
      else {
         *tvar_p = t_var_void;
      }
      break;
   default:
      /* nothing */
      ;
   }

   return val;
}