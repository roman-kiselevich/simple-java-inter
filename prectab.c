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

#include "prectab.h"
#include "restab.h"
#include "exprtree.h"
#include "symtab.h"
#include "boolean.h"
#include "parser.h"
#include "token.h"
#include "boolean.h"
#include "lexer.h"
#include "itrerror.h"

enum prectab_rules {
   Eis_id_literal, /* E -> id | literal          */
   Eis_EplusE,     /* E -> E + E                 */
   Eis_EminusE,    /* E -> E - E                 */
   Eis_EmultE,     /* E -> E * E                 */
   Eis_EdivE,      /* E -> E / E                 */
   Eis_brEbr,      /* E -> ( E )                 */

   Eis_nothing     /* to check for syntax errors */   
};

static char prectab_sg[][8] = {
        /*  +    -    *    /    (    )    i    $  */
/* + */  { '>', '>', '<', '<', '<', '>', '<', '>' },
/* - */  { '>', '>', '<', '<', '<', '>', '<', '>' },
/* * */  { '>', '>', '>', '>', '<', '>', '<', '>' },
/* / */  { '>', '>', '>', '>', '<', '>', '<', '>' },
/* ( */  { '<', '<', '<', '<', '<', '=', '<',  0  },
/* ) */  { '>', '>', '>', '>',  0 , '>',  0 , '>' },
/* i */  { '>', '>', '>', '>',  0 , '>',  0 , '>' },
/* $ */  { '<', '<', '<', '<', '<',  0 , '<',  0  },
};

static unsigned get_operator_index(token_t *tok) {
   switch (tok->type_val.op) {
   case plus_o:  return 0;
   case minus_o: return 1;
   case mult_o:  return 2;
   case div_o:   return 3;
   }

   return 0;
}

static bool next_token_push_flag_sg;

static unsigned prectab_getindex(token_t *tok) {
   static int parenth_counter = 0;
   if (NULL == tok) {
      parenth_counter = 0;
      return 7;
   }

   switch (tok->t_type) {
      case t_op:            return get_operator_index(tok);
      case t_left_parenth:
         if (next_token_push_flag_sg) ++parenth_counter;  
         return 4;
      case t_right_parenth:
         if (parenth_counter <= 0) {
            parenth_counter = 0;
            lex_token_get_next();
            return 7;
         }
         else {
            if (next_token_push_flag_sg) --parenth_counter;
            return 5;
         }
      case t_id_simple:     return 6;
      case t_id_complex:    return 6;
      case t_literal:       return 6;
      default:
         parenth_counter = 0;              
         return 7;
   }

   return 0;
}

enum { pdowns_initial_capacity = 128 };
enum { sred_initial_capacity   = 32  };
enum { stack_realloc_cap       = 64  };

static void sred_resize();
static void pdowns_resize();
static struct stack_of_reduce_ptrs *sred_init();

static struct pushdown_stack {
   unsigned capacity;
   struct pdowns_element *arrel;
   struct pdowns_element *top;
   struct pdowns_element *top_term;
   struct pdowns_element *eostack;
   struct stack_of_reduce_ptrs *sred;
} *pushdown_stack_sg;

struct stack_of_reduce_ptrs {
   unsigned capacity;
   struct pdowns_element **arrptrs_el;
   struct pdowns_element **eostack;
   struct pdowns_element **top;
};

struct pdowns_element {
   token_t tok;
   etree_node_t *expr_node;
};

void init_precedence_tab_parser() {
   pushdown_stack_sg = rtab_malloc(sizeof(*pushdown_stack_sg));
   size_t arrel_size = sizeof(struct pdowns_element)*pdowns_initial_capacity;
   pushdown_stack_sg->arrel = rtab_malloc(arrel_size);

   pushdown_stack_sg->capacity = pdowns_initial_capacity;
   unsigned last_el_index = pdowns_initial_capacity - 1;
   pushdown_stack_sg->eostack = &pushdown_stack_sg->arrel[last_el_index];
   pushdown_stack_sg->top = &pushdown_stack_sg->arrel[0];

   pushdown_stack_sg->top_term = NULL;

   pushdown_stack_sg->sred = sred_init();
}

static struct stack_of_reduce_ptrs *sred_init() {
   struct stack_of_reduce_ptrs *new_stack = rtab_malloc(sizeof(*new_stack));
   size_t new_arrsize = sizeof(struct pdowns_element *)*sred_initial_capacity;
   new_stack->arrptrs_el = rtab_malloc(new_arrsize);
   new_stack->capacity = sred_initial_capacity;

   unsigned last_ptr_index = sred_initial_capacity - 1;
   new_stack->eostack = &new_stack->arrptrs_el[last_ptr_index];
   new_stack->top = &new_stack->arrptrs_el[0];

   return new_stack; 
}

static void pdowns_push(token_t *tok_to_push, bool *expected) {
   if (pushdown_stack_sg->top > pushdown_stack_sg->eostack) {
      pdowns_resize();
   }

   var_info_t *var_info = NULL;
   char *key;
   if (tok_to_push->t_type == t_id_simple) {
      char *class_id = parser_get_curr_class_id();
      char *func_id = parser_get_curr_func_id();
      char *id = tok_to_push->type_val.id_p;
      key = symtab_keygen(class_id, func_id, id);
      var_info = symtab_get_item_info(t_item_var, key);
      if (NULL == var_info) {
         rtab_free(key);
         key = symtab_keygen(class_id, NULL, id);
         tok_to_push->t_type = t_id_complex;
         var_info = symtab_get_item_info(t_item_var, key);
         if (NULL == var_info) {
            *expected = true;
         }
      }
      rtab_free(tok_to_push->type_val.id_p);
      tok_to_push->type_val.id_p = key;
   }
   else if (tok_to_push->t_type == t_id_complex) {
      key = tok_to_push->type_val.id_p;
      var_info = symtab_get_item_info(t_item_var, key);
      if (NULL == var_info) {
         *expected = true;
      }         
   }

   if (var_info) {
      if (var_info->init == false) {
         itrerrno_g = runtime_uninit_err;
         fatal_error("variable \'%s\' must be initialized before using", key);
      }
   }

   pushdown_stack_sg->top->expr_node = NULL;
   pushdown_stack_sg->top->tok.t_type = tok_to_push->t_type;
   pushdown_stack_sg->top->tok.type_val = tok_to_push->type_val;
   pushdown_stack_sg->top->tok.val = tok_to_push->val;
   ++pushdown_stack_sg->top;
}

static void pdowns_resize() {
   size_t old_size = sizeof(struct pdowns_element)*pushdown_stack_sg->capacity;
   size_t new_size = old_size + sizeof(struct pdowns_element)*stack_realloc_cap;
   
   struct pdowns_element *newarr;
   newarr = rtab_realloc(pushdown_stack_sg->arrel, new_size);
   pushdown_stack_sg->arrel = newarr;
   unsigned old_capacity = pushdown_stack_sg->capacity;
   pushdown_stack_sg->top = newarr + old_capacity;
   pushdown_stack_sg->capacity += stack_realloc_cap;

   unsigned last_el_index = pushdown_stack_sg->capacity;
   pushdown_stack_sg->eostack = &newarr[last_el_index];
}

static void sred_push() {
   if (pushdown_stack_sg->sred->top > pushdown_stack_sg->sred->eostack) {
      sred_resize();
   }

   if (pushdown_stack_sg->top_term) {
      *pushdown_stack_sg->sred->top = pushdown_stack_sg->top_term + 1;
   }
   else {
      *pushdown_stack_sg->sred->top = pushdown_stack_sg->arrel;
   }
   ++pushdown_stack_sg->sred->top;
}

static struct pdowns_element *sred_pop() {
   if (pushdown_stack_sg->sred->top == pushdown_stack_sg->sred->arrptrs_el) {
      return NULL;
   }

   struct pdowns_element *el_to_pop;
   --pushdown_stack_sg->sred->top;
   el_to_pop = *pushdown_stack_sg->sred->top;
   return el_to_pop;
}

static void sred_resize() {
   unsigned old_cap = pushdown_stack_sg->sred->capacity;
   size_t old_size = sizeof(struct pdowns_element*)*old_cap;
   unsigned add_size = sizeof(struct pdowns_element*)*stack_realloc_cap;
   size_t new_size = old_size + add_size;

   struct pdowns_element **newarr;
   newarr = rtab_realloc(pushdown_stack_sg->sred->arrptrs_el, new_size);
   pushdown_stack_sg->sred->arrptrs_el = newarr;
   pushdown_stack_sg->sred->top = &newarr[old_cap];
   pushdown_stack_sg->sred->capacity += stack_realloc_cap;
   unsigned new_cap = pushdown_stack_sg->sred->capacity;
   pushdown_stack_sg->sred->eostack = &newarr[new_cap];
}

static bool expr_element(token_t *tok) {
   switch (tok->t_type) {
   case t_id_simple:
   case t_id_complex:
   case t_literal:
   case t_op:
   case t_left_parenth:
   case t_right_parenth:
      return true;
   default:
      return false;
   }
}

static bool end_of_expression(token_t *next_tok) {
   if (!expr_element(next_tok) && !pushdown_stack_sg->top_term) {
     return true;
   }
   else {
      return false;
   }
}

static bool el_is_id_or_literal(struct pdowns_element *el) {
   if (!el->expr_node) {
      switch (el->tok.t_type) {
         case t_id_simple:   return true;
         case t_id_complex:  return true;
         case t_literal:     return true;
         default:            return false;
      }
   }
   else {
      return false;
   }

}

enum rule_match_states {
   rule_match_st_start,
   rule_match_st_syntax_err,
   rule_match_st_operator,
   rule_match_st_plus,
   rule_match_st_minus,
   rule_match_st_mult,
   rule_match_st_div,
   rule_match_st_braces,
   rule_match_st_right_parenth
};

static enum prectab_rules rule_match() {
   enum rule_match_states fsm_state = rule_match_st_start;
   struct pdowns_element *next_el = *(pushdown_stack_sg->sred->top - 1);
   enum prectab_rules rule = Eis_nothing;
   while (next_el != pushdown_stack_sg->top) {
      switch (fsm_state) {
      case rule_match_st_start:
         if (el_is_id_or_literal(next_el)) {
            rule = Eis_id_literal;
            fsm_state = rule_match_st_syntax_err;
         }
         else if (next_el->expr_node) {
            fsm_state = rule_match_st_operator;
         }
         else if (next_el->tok.t_type == t_left_parenth) {
            fsm_state = rule_match_st_braces;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax error start\n");
         }
         break;
      case rule_match_st_operator:
         if (next_el->tok.type_val.op == plus_o && !next_el->expr_node) {
            fsm_state = rule_match_st_plus;
         }
         else if (next_el->tok.type_val.op == minus_o && !next_el->expr_node) {
            fsm_state = rule_match_st_minus;
         }
         else if (next_el->tok.type_val.op == mult_o && !next_el->expr_node) {
            fsm_state = rule_match_st_mult;
         }
         else if (next_el->tok.type_val.op == div_o && !next_el->expr_node) {
            fsm_state = rule_match_st_div;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("rule match error operator expected\n");
         }
         break;
      case rule_match_st_plus:
         if (next_el->expr_node) {
            rule = Eis_EplusE;
            fsm_state = rule_match_st_syntax_err;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax plus error etree expected\n");
         }
         break;
      case rule_match_st_minus:
         if (next_el->expr_node) {
            rule = Eis_EminusE;
            fsm_state = rule_match_st_syntax_err;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax minus error etree expected\n");
         }         
         break;
      case rule_match_st_mult:
         if (next_el->expr_node) {
            rule = Eis_EmultE;
            fsm_state = rule_match_st_syntax_err;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax mult error etree expected\n");
         }
         break;
      case rule_match_st_div:
         if (next_el->expr_node) {
            rule = Eis_EdivE;
            fsm_state = rule_match_st_syntax_err;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax div error etree expected\n");
         }
         break;
      case rule_match_st_braces:
         if (next_el->expr_node) {
            fsm_state = rule_match_st_right_parenth;
         }
         else {
            itrerrno_g = synt_err;
            fatal_error("syntax error E expected\n");
         }
         break;
      case rule_match_st_right_parenth:
         if (next_el->tok.t_type == t_right_parenth && !next_el->expr_node) {
            rule = Eis_brEbr;
            fsm_state = rule_match_st_syntax_err;
         }
         break;
      case rule_match_st_syntax_err:
         itrerrno_g = synt_err;
         fatal_error("syntax error in expression (match fsm)\n");
         break;
      }

      ++next_el;
   }

   if (rule != Eis_nothing) {
      return rule;
   }
   else {
      itrerrno_g = synt_err;
      fatal_error("bad expression no rules to match\n");
      return -1;
   }
}

static void find_set_top_term() {
   struct pdowns_element *search_ptr = pushdown_stack_sg->top;
   --search_ptr;
   for (; search_ptr != pushdown_stack_sg->arrel - 1; --search_ptr) {
      if (!search_ptr->expr_node) {
         pushdown_stack_sg->top_term = search_ptr;
         return ;
      }
   }

   pushdown_stack_sg->top_term = NULL;
}

static void prectab_reduce(enum prectab_rules rule, struct pdowns_element *el) {
   token_t *node_token = &(el + 1)->tok;
   etree_node_t *left = el->expr_node;
   etree_node_t *right = (el + 2)->expr_node;
   switch (rule) {
   case Eis_id_literal:
      el->expr_node = etree_create_node(&el->tok);
      break;
   case Eis_brEbr:
      el->expr_node = (el + 1)->expr_node;
      pushdown_stack_sg->top = el + 1;
      break;
   default:
      el->expr_node = etree_connect_nodes(node_token, left, right);
      pushdown_stack_sg->top = el + 1;
      break;
   }
}

static void prectab_parse(char action, token_t *next_tok, bool *expected) {
   enum prectab_rules reduce_rule;
   switch (action) {
   case '<':
      pdowns_push(next_tok, expected);
      sred_push();
      break;
   case '>':
      reduce_rule = rule_match();
      prectab_reduce(reduce_rule, sred_pop());
      break;
   case '=':
      pdowns_push(next_tok, expected);
      break;
   default:
      itrerrno_g = synt_err;
      fatal_error("syntax error in expr (action)\n");
      break;
   }
}

etree_t *expr(token_t *first, bool *expected) {
   token_t *next_tok;
   if (first) {
      next_tok = first;
   }
   else {
      next_tok = lex_token_get_next();   
   }
   next_token_push_flag_sg = true;
   while (!end_of_expression(next_tok)) {
      token_t *top_term = &pushdown_stack_sg->top_term->tok;
      unsigned row_index = prectab_getindex(top_term);
      unsigned column_index = prectab_getindex(next_tok);
      char action = prectab_sg[row_index][column_index];
      prectab_parse(action, next_tok, expected);

      next_token_push_flag_sg = false;
      if (action == '<' || action == '=') {
         next_tok = lex_token_get_next();
         next_token_push_flag_sg = true;
      } /* if action require push element to stack */
      
      find_set_top_term();
   }

   if (NULL == next_tok) {
      fprintf(stderr, "eof in expr\n");
      return NULL;
   }

   etree_node_t *init_root = (pushdown_stack_sg->top - 1)->expr_node;
   etree_t *expr_tree = etree_init(init_root);
   pushdown_stack_sg->top = &pushdown_stack_sg->arrel[0];
   pushdown_stack_sg->top_term = NULL;   
   return expr_tree;
}
