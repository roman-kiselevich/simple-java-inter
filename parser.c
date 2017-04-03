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
 *        Kiselevich Roman
 */

#include "ilist.h"
#include "parser.h"
#include "token.h"
#include "lexer.h"
#include "restab.h"
#include "boolean.h"
#include "symtab.h"
#include "prectab.h"
#include "itrerror.h"
#include "soexpitems.h"
#include "ifj16funcs.h"
#include "cmptcheck.h"

static struct parser {
   char *curr_class_id;
   char *curr_func_id;
   func_info_t *curr_func;

   token_t saved_tok;
   char *saved_id;

   instr_t **buf;
   unsigned count;
} *parser_sg;

static bool program();
static bool class_list();
static bool class_def();
static bool class_instance_list();
static bool var_global_def();
static bool var_global_init();
static bool func_def();
static instr_t *func_body();
static instr_t *func_call(bool id_is_complex);
static void arg_list(func_info_t *func_info);
static void arg(func_info_t *func_info);
static void arg_next(func_info_t *func_info);
static instr_t *assignment_expr(bool id_is_complex);
static void var_local_def();
static instr_t *statement_list();
static instr_t *condition();
static instr_t *loop();
static instr_t *var_local_init();
static instr_t *statement_return();
static instr_t *assignment_func_call(bool id_is_complex);
static instr_t *statement_seq();
static void param_list(instr_t *func_call, bool *expected);
static void param(instr_t *func_call, bool *expected);
static void param_next(instr_t *func_call, bool *expected);

static void save_last_instrs(instr_t **instr_addr);

static bool token_is_id(token_t *tok) {
   if (tok->t_type == t_id_simple || tok->t_type == t_id_complex) {
      return true;
   }
   else {
      return false;
   }
}

char *parser_get_curr_class_id() {
   return parser_sg->curr_class_id;
}

char *parser_get_curr_func_id() {
   return parser_sg->curr_func_id;
}

void run_parsing() {
   parser_sg = rtab_malloc(sizeof(*parser_sg));
   parser_sg->curr_class_id = NULL;
   parser_sg->curr_func_id = NULL;
   parser_sg->saved_id = NULL;
   parser_sg->buf = NULL;
   parser_sg->count = 0;
   symtab_insert_item(t_item_class, "ifj16", NULL);
   if (!program()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: program");
   }
}

static bool program() {
   if (!class_list()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: class_list");
      return false;
   }
   else {
      return true;
   }
}

static bool class_list() {
   if (lex_token_get_next() == NULL)
      return true;

   if (!class_def()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: class_def or class_list err");
   }
   
   if (!class_list()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: bad class list");
   }

   return true;
}

static bool class_def() {
   if (lex_token_get_current()->t_type != t_keyword) {
      itrerrno_g = synt_err;
      fatal_error("missing class keyword in class definition\n");
   }

   if (lex_token_get_current()->type_val.keyword != tkw_class) {
      itrerrno_g = synt_err;
      fatal_error("missing class keyword in class definition\n");
   }

   if (lex_token_get_next()->t_type != t_id_simple) {
      itrerrno_g = synt_err;
      fatal_error("missing id in class definition\n");
   }
   else {
      parser_sg->curr_class_id = lex_token_get_current()->type_val.id_p;
      symtab_insert_item(t_item_class, parser_sg->curr_class_id, NULL);
   }

   if (lex_token_get_next()->t_type != t_left_brace) {
      itrerrno_g = synt_err;
      fatal_error("missing left brace in class definition");
   }

   if (!class_instance_list()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: class_instance_list in class_def");
   }

   return true;
}

static bool class_instance_list() {
   if (lex_token_get_next()->t_type == t_right_brace) {
      return true;
   }

   if (lex_token_get_current()->t_type != t_keyword) {
      itrerrno_g = synt_err;
      fatal_error("static exptected in inst def\n");
   }

   if (lex_token_get_current()->type_val.keyword != tkw_static) {
      itrerrno_g = synt_err;
      fatal_error("static exptected in inst def\n");      
   }

   if (lex_token_get_next()->t_type != t_keyword) {
      itrerrno_g = synt_err;
      fatal_error("type is expected\n");
   }

   switch (lex_token_get_current()->type_val.keyword) {
   case tkw_int:
   case tkw_double:
   case tkw_string:
   case tkw_void:
      parser_sg->saved_tok.t_type = lex_token_get_current()->t_type;
      parser_sg->saved_tok.type_val = lex_token_get_current()->type_val;
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("expected type in instance def\n");
      break;
   }

   if (lex_token_get_next()->t_type != t_id_simple) {
      itrerrno_g = synt_err;
      fatal_error("expected id in class instance def\n");
   }

   parser_sg->saved_id = lex_token_get_current()->type_val.id_p;

   switch (lex_token_get_next()->t_type) {
   case t_semicolon:
      if (!var_global_def()) {
         itrerrno_g = synt_err;
         fatal_error("var global def syntax err\n");
      }
      break;
   case t_left_parenth:
      if (!func_def()) {
         itrerrno_g = synt_err;
         fatal_error("function def syntax err\n");
      }
      break;
   case t_assignment:
      if (!var_global_init()) {
         itrerrno_g = synt_err;
         fatal_error("var global init syntax err\n");
      }
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("syntax error in class instance def\n");
   }

   if (!class_instance_list()) {
      itrerrno_g = synt_err;
      fatal_error("parser debug: missing func definition "
                  "or variables def in class %s\n", parser_sg->curr_class_id);
   }

   return true;
}

static bool var_global_def() {
   var_info_t var_info;

   enum type_variable type_new_var;
   switch (parser_sg->saved_tok.type_val.keyword) {
   case tkw_int:
      type_new_var = t_var_int;
      break;
   case tkw_double:
      type_new_var = t_var_float;
      break;
   case tkw_string:
      type_new_var = t_var_string;
      break;
   default:
      /* nie dolzhno po logike */
      itrerrno_g = synt_err;
      fatal_error("smth goes wrong :( \n");
      break;
   }

   var_info.t_var = type_new_var;
   var_info.init = false;
   var_info.var_ptr = NULL;
   char *id = parser_sg->saved_id;
   char *new_key = symtab_keygen(parser_sg->curr_class_id, NULL, id);
   symtab_insert_item(t_item_var, new_key, &var_info);

   return true;
}

static bool var_global_init() {
   var_info_t var_info;

   enum type_variable type_new_var;
   switch (parser_sg->saved_tok.type_val.keyword) {
   case tkw_int:
      type_new_var = t_var_int;
      break;
   case tkw_double:
      type_new_var = t_var_float;
      break;
   case tkw_string:
      type_new_var = t_var_string;
      break;
   default:
      /* nie dolzhno po logike */
      itrerrno_g = synt_err;
      fatal_error("smth goes wrong :( \n");
      break;
   }

   var_info.t_var = type_new_var;
   var_info.init = false;
   var_info.var_ptr = NULL;

   char *id = parser_sg->saved_id;
   char *new_key = symtab_keygen(parser_sg->curr_class_id, NULL, id);
   var_info_t *added_var_info;
   added_var_info = symtab_insert_item(t_item_var, new_key, &var_info);

   bool expected = false;
   etree_t *expr_tree = expr(NULL, &expected);

   if (lex_token_get_current()->t_type != t_semicolon) {
      itrerrno_g = synt_err;
      fatal_error("expected semicolon at the end of expr\n");
   }

   instr_t instr;
   instr.t_instr = instr_assignment_expr;
   instr.left_operand = added_var_info;
   instr.right_etree = expr_tree;
   instr.next = NULL;


   instr_t *new_instr = ilist_init_instr(&instr);
   if (expected) {
      soeitems_add_item(new_key, NULL, expected, new_instr);
   }
   else {
      compatibility_check_var_etree(added_var_info->t_var, expr_tree);
   }

   ilist_insert_first(new_instr);

   added_var_info->init = true;
   return true;
}

static bool func_def() {
   func_info_t func_info;
   func_info.ifj16_func = ifj16_not_ifj16;
   func_info.arrargs = NULL;
   func_info.num_args = 0;
   func_info.first_instruction = NULL;
   func_info.num_defvars = 0;
   func_info.arrvar_ptrs = NULL;
   switch (parser_sg->saved_tok.type_val.keyword) {
   case tkw_void:
      func_info.ret_type = t_var_void;
      break;
   case tkw_int:
      func_info.ret_type = t_var_int;
      break;
   case tkw_double:
      func_info.ret_type = t_var_float;
      break;
   case tkw_string:
      func_info.ret_type = t_var_string;
      break;
   default:
      /* do nothing nie dolzhno */
      break;
   }

   char *id = parser_sg->saved_id;
   parser_sg->curr_func_id = id;
   char *class_id = parser_sg->curr_class_id;
   char *new_key = symtab_keygen(class_id, NULL, id);
   func_info_t *added_func_info;
   added_func_info = symtab_insert_item(t_item_func, new_key, &func_info);
   parser_sg->curr_func = added_func_info;

   arg_list(added_func_info);

   if (lex_token_get_next()->t_type != t_left_brace) {
      itrerrno_g = synt_err;
      fatal_error("left brace expected in func def\n");
   }

   added_func_info->first_instruction = func_body();

   if (lex_token_get_current()->t_type != t_right_brace) {
      itrerrno_g = synt_err;
      fatal_error("right brace expected in func def\n");
   }

   if (added_func_info->num_args) {
      unsigned new_cap = added_func_info->num_args;
      size_t new_size = sizeof(*added_func_info->arrargs)*new_cap;
      added_func_info->arrargs = 
         rtab_realloc(added_func_info->arrargs, new_size);
   }

   if (added_func_info->num_defvars) {
      unsigned new_cap = added_func_info->num_defvars;
      size_t new_size = sizeof(*added_func_info->arrvar_ptrs)*new_cap;
      void *old_array = added_func_info->arrvar_ptrs;
      added_func_info->arrvar_ptrs = rtab_realloc(old_array, new_size);
   }

   parser_sg->curr_func_id = NULL;
   return true;
}

static void arg_list(func_info_t *func_info) {
   if (lex_token_get_next()->t_type == t_right_parenth) {
      return ;
   }

   arg(func_info);

   arg_next(func_info);
}

enum { arr_args_realloc_size = 8 };

static void arg(func_info_t *func_info) {
   if (lex_token_get_current()->t_type != t_keyword) {
      itrerrno_g = synt_err;
      fatal_error("type of argument missing\n");
   }

   var_info_t arg_info;
   arg_info.init = true;
   arg_info.var_ptr = NULL;
   switch (lex_token_get_current()->type_val.keyword) {
   case tkw_int:
      arg_info.t_var = t_var_int;
      break;
   case tkw_double:
      arg_info.t_var = t_var_float;
      break;
   case tkw_string:
      arg_info.t_var = t_var_string;
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("type in arglist expected\n");
      break;
   }

   if (lex_token_get_next()->t_type != t_id_simple) {
      itrerrno_g = synt_err;
      fatal_error("id in arg list expected\n");
   }

   char *id = lex_token_get_current()->type_val.id_p;
   char *class_id = parser_sg->curr_class_id;
   char *func_id = parser_sg->curr_func_id;
   char *new_key = symtab_keygen(class_id, func_id, id);

   var_info_t *added_var_info;
   added_var_info = symtab_insert_item(t_item_var, new_key, &arg_info);
   
   if (func_info->num_args % arr_args_realloc_size == 0) {
      unsigned new_cap = func_info->num_args + arr_args_realloc_size;
      size_t new_arr_size = sizeof(*func_info->arrargs)*new_cap;
      func_info->arrargs = rtab_realloc(func_info->arrargs, new_arr_size);
   }

   func_info->arrargs[func_info->num_args] = added_var_info;
   ++func_info->num_args;
}

static void arg_next(func_info_t *func_info) {
   if (lex_token_get_next()->t_type == t_right_parenth) {
      return ;
   }

   if (lex_token_get_current()->t_type != t_comma) {
      itrerrno_g = synt_err;
      fatal_error("comma expected in arglist\n");
   }

   lex_token_get_next();
   arg(func_info);

   arg_next(func_info);
}

static instr_t *func_body() {
   if (lex_token_get_next()->t_type == t_right_brace) {
      return NULL;
   }

   instr_t *new_instr_ptr;
   if (lex_token_get_current()->t_type != t_keyword) {
      if (!token_is_id( lex_token_get_current() )) {
         itrerrno_g = synt_err;
         fatal_error("syntax error in function body\n");
      }

      bool id_is_complex;
      if (lex_token_get_current()->t_type == t_id_complex) {
         id_is_complex = true;
      }
      else {
         id_is_complex = false;
      }
      parser_sg->saved_id = lex_token_get_current()->type_val.id_p;

      if (lex_token_get_next()->t_type != t_assignment) {
         if (lex_token_get_current()->t_type != t_left_parenth) {
            itrerrno_g = synt_err;
            fatal_error("syntax error in fb '=' or '(' missing?\n");
         }
         else {
            new_instr_ptr = func_call(id_is_complex);
            new_instr_ptr->next = func_body();
            return new_instr_ptr;
         }
      }

      parser_sg->saved_tok.t_type = lex_token_get_next()->t_type;
      parser_sg->saved_tok.type_val = lex_token_get_current()->type_val;
      parser_sg->saved_tok.val = lex_token_get_current()->val;

      if (lex_token_get_next()->t_type == t_left_parenth) {
         new_instr_ptr = assignment_func_call(id_is_complex);
      }
      else {
         lex_token_unget();
         new_instr_ptr = assignment_expr(id_is_complex);
      }

      new_instr_ptr->next = func_body();

      return new_instr_ptr;
   } /* if keyword */


   switch (lex_token_get_current()->type_val.keyword) {
   case tkw_int:
   case tkw_double:
   case tkw_string:
      parser_sg->saved_tok.t_type = t_keyword;
      parser_sg->saved_tok.type_val = lex_token_get_current()->type_val;
      break;
   case tkw_if:
      new_instr_ptr = condition();

      instr_t *next = func_body();
      if (next) {
         save_last_instrs(&new_instr_ptr->next_true);
         save_last_instrs(&new_instr_ptr->next_false);
         instr_t **saved_addr;
         while (NULL != (saved_addr = ilist_get_saved_instr_addr())) {
            *saved_addr = next;
         }
      }
      return new_instr_ptr;
   case tkw_while:
      new_instr_ptr = loop();

      new_instr_ptr->next_false = func_body();
      return new_instr_ptr;
   case tkw_return:
      new_instr_ptr = statement_return();

      new_instr_ptr->next = func_body();
      return new_instr_ptr;
   default:
      itrerrno_g = synt_err;
      fatal_error("expected type in local var def\n");
   }

   if (lex_token_get_next()->t_type != t_id_simple) {
      itrerrno_g = synt_err;
      fatal_error("expected id in local var def\n");
   }

   parser_sg->saved_id = lex_token_get_current()->type_val.id_p;

   switch (lex_token_get_next()->t_type) {
   case t_semicolon:
      var_local_def();
      new_instr_ptr = func_body();
      return new_instr_ptr;
   case t_assignment:
      new_instr_ptr = var_local_init();
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("expected assignment or semicolon in var local def\n");
   }

   new_instr_ptr->next = func_body();
   return new_instr_ptr;
}

static instr_t *func_call(bool id_is_complex) {
   instr_t instr;
   instr.num_params = 0;
   instr.t_instr = instr_call;
   instr.arretrees = NULL;
   instr.num_params = 0;

   bool expected = false;
   param_list(&instr, &expected);

   char *search_func_key;
   if (id_is_complex) {
      search_func_key = parser_sg->saved_id;
   }
   else {
      char *class_id = parser_sg->curr_class_id;
      char *id = parser_sg->saved_id;
      search_func_key = symtab_keygen(class_id, NULL, id);
      /* free id ? */
   }

   func_info_t *func_info;
   enum ifj16_funcs ifj_func = ifj16funcs_recognizer(search_func_key);
   if (ifj_func != ifj16_not_ifj16) {
      search_func_key = ifj16_get_func_id(ifj_func);
   }

   func_info = symtab_get_item_info(t_item_func, search_func_key);

   instr.func_info = func_info;
   instr.next = NULL;
   instr_t *new_instr = ilist_init_instr(&instr);
   if (lex_token_get_next()->t_type != t_semicolon) {
      itrerrno_g = synt_err;
      fatal_error("expected semicolon in func call\n");
   }

   if (ifj_func == ifj16_not_ifj16) {
      if (!func_info || expected) {
         soeitems_add_item(NULL, search_func_key, false, new_instr);
      }
      else {
         if (func_info->num_args != new_instr->num_params) {
            itrerrno_g = synt_err;
            fatal_error("to few arguments");
         }

         for (unsigned i = 0; i < func_info->num_args; ++i) {
            etree_t *etree = new_instr->arretrees[i];
            var_info_t *arg_info = func_info->arrargs[i];
            compatibility_check_var_etree(arg_info->t_var, etree);
         }
      }
   }
   else {
      if (expected) {
         soeitems_add_item(NULL, search_func_key, false, new_instr);
      }
   }

   return new_instr;
}

static instr_t *assignment_expr(bool id_is_complex) {
   bool expected = false;
   etree_t *expr_tree = expr(&parser_sg->saved_tok, &expected);

   char *search_key;
   if (id_is_complex) {
      search_key = parser_sg->saved_id;
   }
   else {
      char *class_id = parser_sg->curr_class_id;
      char *func_id = parser_sg->curr_func_id;
      search_key = symtab_keygen(class_id, func_id, parser_sg->saved_id);
      /* to free id ? */
   }

   var_info_t *var_info = symtab_get_item_info(t_item_var, search_key);
   if (!var_info) {
      char *class_id = parser_sg->curr_class_id;
      char *id = parser_sg->saved_id;
      rtab_free(search_key);
      search_key = symtab_keygen(class_id, NULL, id);
      var_info = symtab_get_item_info(t_item_var, search_key);
   }

   if (var_info) {
      var_info->init = true;
   }

   instr_t instr;
   instr.t_instr = instr_assignment_expr;
   instr.left_operand = var_info;
   instr.right_etree = expr_tree;
   instr.next = NULL;
   instr_t *new_instr = ilist_init_instr(&instr);

   if (lex_token_get_current()->t_type != t_semicolon) {
      itrerrno_g = synt_err;
      fatal_error("expected semicolon at the end of expr");
   }

   if (!var_info || expected) {
      soeitems_add_item(search_key, NULL, expected, new_instr);
   }
   else {
      compatibility_check_var_etree(var_info->t_var, expr_tree);
   }
   return new_instr;
}

enum { arr_var_ptrs_realloc_size = 16 };

static void var_local_def() {
   var_info_t var_info;
   var_info.init = false;
   var_info.var_ptr = NULL;
   switch (parser_sg->saved_tok.type_val.keyword) {
   case tkw_int:
      var_info.t_var = t_var_int;
      break;
   case tkw_double:
      var_info.t_var = t_var_float;
      break;
   case tkw_string:
      var_info.t_var = t_var_string;
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("smth goes wrong in var local def :( \n");
      break;
   }

   char *class_id = parser_sg->curr_class_id;
   char *func_id = parser_sg->curr_func_id;
   char *id = parser_sg->saved_id;
   char *new_key = symtab_keygen(class_id, func_id, id);
   /* free id ? */
   var_info_t *added_var;
   added_var = symtab_insert_item(t_item_var, new_key, &var_info);

   func_info_t *func_info = parser_sg->curr_func;
   if (func_info->num_defvars % arr_var_ptrs_realloc_size == 0) {
      unsigned new_cap = func_info->num_defvars + arr_var_ptrs_realloc_size;
      size_t new_arr_size = sizeof(*func_info->arrvar_ptrs)*new_cap;
      void *old_array = func_info->arrvar_ptrs;
      func_info->arrvar_ptrs = rtab_realloc(old_array, new_arr_size);
   }

   func_info->arrvar_ptrs[func_info->num_defvars] = added_var;
   ++func_info->num_defvars;
}

static instr_t *var_local_init() {
   var_info_t var_info;
   var_info.init = false;
   var_info.var_ptr = NULL;
   switch (parser_sg->saved_tok.type_val.keyword) {
   case tkw_int:
      var_info.t_var = t_var_int;
      break;
   case tkw_double:
      var_info.t_var = t_var_float;
      break;
   case tkw_string:
      var_info.t_var = t_var_string;
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("smth goes wrong in var init def :( \n");
      break;
   }

   char *class_id = parser_sg->curr_class_id;
   char *func_id = parser_sg->curr_func_id;
   char *id = parser_sg->saved_id;
   char *new_key = symtab_keygen(class_id, func_id, id);
   /* free id ? */

   var_info_t *added_var_info;
   added_var_info = symtab_insert_item(t_item_var, new_key, &var_info);

   bool expected = false;
   etree_t *expr_tree = expr(NULL, &expected);

   instr_t new_instr;
   new_instr.t_instr = instr_assignment_expr;
   new_instr.left_operand = added_var_info;
   new_instr.right_etree = expr_tree;
   new_instr.next = NULL;
   instr_t *new_instr_ptr = ilist_init_instr(&new_instr);

   if (expected) {
      soeitems_add_item(new_key, NULL, expected, new_instr_ptr);
   }
   else {
      compatibility_check_var_etree(added_var_info->t_var, expr_tree);
   }

   added_var_info->init = true;

   func_info_t *func_info = parser_sg->curr_func;
   if (func_info->num_defvars % arr_var_ptrs_realloc_size == 0) {
      unsigned new_cap = func_info->num_defvars + arr_var_ptrs_realloc_size;
      size_t new_arr_size = sizeof(*func_info->arrvar_ptrs)*new_cap;
      void *old_array = func_info->arrvar_ptrs;
      func_info->arrvar_ptrs = rtab_realloc(old_array, new_arr_size);
   }

   func_info->arrvar_ptrs[func_info->num_defvars] = added_var_info;
   ++func_info->num_defvars;
   return new_instr_ptr;
}

static instr_t *assignment_func_call(bool id_is_complex) {
   instr_t instr;
   instr.num_params = 0;
   instr.t_instr = instr_assignment_call;
   instr.arretrees = NULL;
   instr.num_params = 0;

   bool expected = false;
   param_list(&instr, &expected);

   instr_t *new_instr = ilist_init_instr(&instr);
   char *search_var_key;
   if (id_is_complex) {
      search_var_key = parser_sg->saved_id;
   }
   else {
      char *class_id = parser_sg->curr_class_id;
      char *func_id = parser_sg->curr_func_id;
      char *id = parser_sg->saved_id;
      search_var_key = symtab_keygen(class_id, func_id, id);
      /* free id ? */
   }

   var_info_t *left_operand = symtab_get_item_info(t_item_var, search_var_key);
   if (!left_operand) {
      char *class_id = parser_sg->curr_class_id;
      char *id = parser_sg->saved_id;
      rtab_free(search_var_key);
      search_var_key = symtab_keygen(class_id, NULL, id);
      left_operand = symtab_get_item_info(t_item_var, search_var_key);
   }

   if (left_operand) {
      left_operand->init = true;
   }

   char *search_func_key;
   if (parser_sg->saved_tok.t_type == t_id_complex) {
      search_func_key = parser_sg->saved_tok.type_val.id_p;
   }
   else {
      char *class_id = parser_sg->curr_class_id;
      char *id = parser_sg->saved_tok.type_val.id_p;
      search_func_key = symtab_keygen(class_id, NULL, id);
      /* free id ? */
   }

   func_info_t *func_info;
   enum ifj16_funcs ifj_func = ifj16funcs_recognizer(search_func_key);
   if (ifj_func != ifj16_not_ifj16) {
      search_func_key = ifj16_get_func_id(ifj_func);
   }

   func_info = symtab_get_item_info(t_item_func, search_func_key);
   if (func_info && func_info->ifj16_func == ifj16_not_ifj16) {
      if (!func_info || !left_operand || expected) {
         soeitems_add_item(search_var_key, search_func_key, false, new_instr);
      }
      else if (func_info->ret_type == t_var_void) {
         itrerrno_g = semant_match_err;
         fatal_error("function \"%s\" with void ret cannot be assign to smth");
      }
      else {
         compatibility_check_var_var(func_info->ret_type, left_operand->t_var);
         if (func_info->num_args != new_instr->num_params) {
            itrerrno_g = semant_match_err;
            fatal_error("to few arguments");
         }

         for (unsigned i = 0; i < func_info->num_args; ++i) {
            etree_t *etree = new_instr->arretrees[i];
            var_info_t *arg_info = func_info->arrargs[i];
            compatibility_check_var_etree(arg_info->t_var, etree);
         }
      }
   }
   else {
      if (!left_operand || expected || !func_info) {
         soeitems_add_item(search_var_key, search_func_key, false, new_instr);
      }
      else {
         compatibility_check_var_var(func_info->ret_type, left_operand->t_var);
      }
   }

   new_instr->left_operand = left_operand;
   new_instr->func_info = func_info;
   new_instr->next = NULL;

   if (lex_token_get_next()->t_type != t_semicolon) {
      itrerrno_g = synt_err;
      fatal_error("expected semicolon in func call\n");
   }

   return new_instr;
}

static void param_list(instr_t *func_call, bool *expected) {
   if (lex_token_get_next()->t_type == t_right_parenth) {
      return ;
   }

   lex_token_unget();
   param(func_call, expected);

   param_next(func_call, expected);
}

static void param(instr_t *func_call, bool *expected) {
   etree_t *expr_tree = expr( lex_token_get_next() , expected);

   if (func_call->num_params % arr_args_realloc_size == 0) {
      unsigned new_cap = func_call->num_params + arr_args_realloc_size;
      size_t new_arr_size = sizeof(*func_call->arretrees)*new_cap;
      func_call->arretrees = rtab_realloc(func_call->arretrees, new_arr_size);      
   }

   func_call->arretrees[func_call->num_params] = expr_tree;
   ++func_call->num_params;
}

static void param_next(instr_t *func_call, bool *expected) {
   if (lex_token_get_current()->t_type == t_semicolon) {
      lex_token_unget();
      return ;
   }

   if (lex_token_get_current()->t_type != t_comma) {
      itrerrno_g = synt_err;
      fatal_error("expected comma in call func\n");
   }

   param(func_call, expected);

   param_next(func_call, expected);
}

static instr_t *condition() {
   if (lex_token_get_next()->t_type != t_left_parenth) {
      itrerrno_g = synt_err;
      fatal_error("expected '(' in condition");
   }

   bool expected = false;
   etree_t *left_expr = expr(NULL, &expected);

   if (lex_token_get_current()->t_type != t_relop) {
      itrerrno_g = synt_err;
      fatal_error("expected relation operator in condition");
   }

   instr_t condition;
   switch (lex_token_get_current()->type_val.relop) {
   case gt_ro:
      condition.t_instr = instr_condition_gt;
      break;
   case geq_ro:
      condition.t_instr = instr_condition_geq;
      break;
   case lt_ro:
      condition.t_instr = instr_condition_lt;
      break;
   case leq_ro:
      condition.t_instr = instr_condition_leq;
      break;
   case eq_ro:
      condition.t_instr = instr_condition_eq;
      break;
   case noeq_ro:
      condition.t_instr = instr_condition_noeq;
      break;
   }

   etree_t *right_expr = expr(NULL, &expected);

   instr_t *new_instr = ilist_init_instr(&condition);

   lex_token_unget();
   instr_t *first_true_seq = statement_seq();

   if (lex_token_get_next()->t_type != t_keyword) {
      if (lex_token_get_current()->type_val.keyword != tkw_else) {
         itrerrno_g = synt_err;
         fatal_error("expected else keyword");
      }
   }

   instr_t *first_false_seq = statement_seq();

   new_instr->left_etree = left_expr;
   new_instr->right_etree = right_expr;
   new_instr->next_true = first_true_seq;
   new_instr->next_false = first_false_seq;

   if (expected) {
      soeitems_add_item(NULL, NULL, false, new_instr);
   }
   else {
      enum type_variable left = etree_get_result_type(left_expr);
      enum type_variable right = etree_get_result_type(right_expr);
      if (left == t_var_string || right == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot use strings in conditions");
      }
   }
   return new_instr;
}

static instr_t *statement_seq() {
   if (lex_token_get_next()->t_type != t_left_brace) {
      itrerrno_g = synt_err;
      fatal_error("expected left brace in statement seq\n");
   }

   instr_t *first = statement_list();
   return first;
}

static instr_t *loop() {
   if (lex_token_get_next()->t_type != t_left_parenth) {
      itrerrno_g = synt_err;
      fatal_error("expected '(' in condition");
   }

   bool expected;
   etree_t *left_expr = expr(NULL, &expected);

   if (lex_token_get_current()->t_type != t_relop) {
      itrerrno_g = synt_err;
      fatal_error("expected relation operator in condition");
   }

   instr_t condition;
   switch (lex_token_get_current()->type_val.relop) {
   case gt_ro:
      condition.t_instr = instr_condition_gt;
      break;
   case geq_ro:
      condition.t_instr = instr_condition_geq;
      break;
   case lt_ro:
      condition.t_instr = instr_condition_lt;
      break;
   case leq_ro:
      condition.t_instr = instr_condition_leq;
      break;
   case eq_ro:
      condition.t_instr = instr_condition_eq;
      break;
   case noeq_ro:
      condition.t_instr = instr_condition_noeq;
      break;
   }

   etree_t *right_expr = expr(NULL, &expected);

   instr_t *new_instr = ilist_init_instr(&condition);

   lex_token_unget();
   instr_t *first_true_seq = statement_seq();

   save_last_instrs(&first_true_seq);

   instr_t **last_instr_addr;
   while (NULL != (last_instr_addr = ilist_get_saved_instr_addr())) {
      *last_instr_addr = new_instr;
   }

   new_instr->left_etree = left_expr;
   new_instr->right_etree = right_expr;
   new_instr->next_true = first_true_seq;

   if (expected) {
      soeitems_add_item(NULL, NULL, false, new_instr);
   }
   else {
      enum type_variable left = etree_get_result_type(left_expr);
      enum type_variable right = etree_get_result_type(right_expr);
      if (left == t_var_string || right == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot use strings in conditions");
      }
   }
   return new_instr;   
}

static instr_t *statement_list() {
   if (lex_token_get_next()->t_type == t_right_brace) {
      return NULL;
   }

   instr_t *new_instr_ptr;
   if (lex_token_get_current()->t_type != t_keyword) {
      if (!token_is_id( lex_token_get_current() )) {
         itrerrno_g = synt_err;
         fatal_error("syntax error in statement_seq\n");
      }

      bool id_is_complex;
      if (lex_token_get_current()->t_type == t_id_complex) {
         id_is_complex = true;
      }
      else {
         id_is_complex = false;
      }
      parser_sg->saved_id = lex_token_get_current()->type_val.id_p;

      if (lex_token_get_next()->t_type != t_assignment) {
         if (lex_token_get_current()->t_type != t_left_parenth) {
            itrerrno_g = synt_err;
            fatal_error("syntax error in fb '=' or '(' missing?\n");
         }
         else {
            new_instr_ptr = func_call(id_is_complex);
            new_instr_ptr->next = func_body();
            return new_instr_ptr;
         }
      }

      parser_sg->saved_tok.t_type = lex_token_get_next()->t_type;
      parser_sg->saved_tok.type_val = lex_token_get_current()->type_val;
      parser_sg->saved_tok.val = lex_token_get_current()->val;

      if (lex_token_get_next()->t_type == t_left_parenth) {
         new_instr_ptr = assignment_func_call(id_is_complex);
      }
      else {
         lex_token_unget();
         new_instr_ptr = assignment_expr(id_is_complex);
      }

      new_instr_ptr->next = statement_list();
      return new_instr_ptr;
   } /* if keyword */

   switch (lex_token_get_current()->type_val.keyword) {
   case tkw_if:
      new_instr_ptr = condition();

      instr_t *next_instr = statement_list();
      if (next_instr) {
         save_last_instrs(&new_instr_ptr->next_true);
         save_last_instrs(&new_instr_ptr->next_false);
         instr_t **saved_addr;
         while (NULL != (saved_addr = ilist_get_saved_instr_addr())) {
            *saved_addr = next_instr;
         }
      }
      break;
   case tkw_while:
      new_instr_ptr = loop();

      new_instr_ptr->next_false = func_body();
      return new_instr_ptr;
   case tkw_return:
      new_instr_ptr = statement_return();

      new_instr_ptr->next = func_body();
      return new_instr_ptr;   
   default:
      itrerrno_g = synt_err;
      fatal_error("bad statement in statement list\n");
   }


   return new_instr_ptr; /* nie dolzhno */
}


static void save_last_instrs(instr_t **instr_addr) {
   static instr_t *saved_condtition;
   if (*instr_addr == NULL) {
      ilist_save_instr_addr(instr_addr);
      return ;
   }

   if (*instr_addr == (*instr_addr)->next_true) {
      return;
   }

   switch ((*instr_addr)->t_instr) {
   case instr_condition_gt:
   case instr_condition_geq:
   case instr_condition_lt:
   case instr_condition_leq:
   case instr_condition_eq:
   case instr_condition_noeq:
      saved_condtition = *instr_addr;
      save_last_instrs(&(*instr_addr)->next_true);
      save_last_instrs(&(*instr_addr)->next_false);
      saved_condtition = NULL;
      break;
   default:
      if (saved_condtition && (*instr_addr)->next == saved_condtition) {
         saved_condtition = NULL;
         return;
      }
      save_last_instrs(&(*instr_addr)->next);
      break;
   }
}

static instr_t *statement_return() {
   instr_t instr;
   instr.t_instr = instr_ret;
   instr_t *new_instr = ilist_init_instr(&instr);
   new_instr->func_info = parser_sg->curr_func;
   func_info_t *func_info = parser_sg->curr_func;
   if (parser_sg->curr_func->ret_type != t_var_void) {
      if (lex_token_get_next()->t_type == t_semicolon) {
         itrerrno_g = synt_err;
         fatal_error("expected expr in non-void func ret statement");
      }
      lex_token_unget();
      bool expected = false;
      etree_t *expr_tree = expr(NULL, &expected);
      new_instr->left_etree = expr_tree;
      if (expected) {
         soeitems_add_item(NULL, NULL, expected, new_instr);
      }
      else {
         compatibility_check_var_etree(func_info->ret_type, expr_tree);
      }


      if (lex_token_get_current()->t_type != t_semicolon) {
         itrerrno_g = synt_err;
         fatal_error("expected semicolon after expr in ret statement");
      }
   }
   else {
      new_instr->left_etree = NULL;
      if (lex_token_get_next()->t_type != t_semicolon) {
         itrerrno_g = synt_err;
         fatal_error("expected semicolon in void func ret statement");
      }
   }

   return new_instr;
}