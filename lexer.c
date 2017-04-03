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

#include "lexer.h"
#include "restab.h"
#include "buffer.h"
#include "itrerror.h"
#include "boolean.h"
#include "kwrecognizer.h"
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static struct lexer {
   buffer_t *buf;
   FILE *source_file;
   token_t curr_tok;
   bool ungetch_flag;
} *lexer;

enum states {
   start_st,
   comment_begin_st,
   block_comment_st,
   block_comment_end_st,
   line_comment_st,
   comment_unterminated_error_st,

   id_simple_st,
   id_complex_begin_st,
   id_complex_st,

   string_lit_st,
   string_lit_escape_st,

   int_lit_st,

   float_lit_exp_begin_st,
   float_lit_exp_digits_st,
   float_lit_exp_st,

   relop_greater_eq_st,
   relop_less_eq_st,
   relop_eq_st,
   relop_no_eq_st,

};

void lexer_init(FILE *new_source_file) {
   lexer = rtab_malloc(sizeof(*lexer));
   lexer->buf = buf_init(new_source_file);
   lexer->source_file = new_source_file;
   lexer->ungetch_flag = false;
}

token_t *lex_token_get_current() {
   return &lexer->curr_tok;
}

static bool id_begin_char(int ch) {
   if (isalpha(ch) || ch == '$' || ch == '_')
      return true;
   else
      return false;
}

static bool id_char(int ch) {
   if (isalpha(ch) || isdigit(ch) || ch == '$' || ch == '_')
      return true;
   else
      return false;
}

static int str_to_int(char *str) {
   long int to_ret = strtol(str, NULL, 10);
   if (errno == ERANGE) {
      /* set itrerrno_g */
      itrerrno_g = lex_err;
      fatal_error("integer literal is too large for its type\n");
   }
   else if (to_ret > INT_MAX || to_ret < INT_MIN) {
      /* set itrerrno_g */
      itrerrno_g = lex_err;
      fatal_error("integer literal is too large for its type\n");
   }
   
   return to_ret;
}

static double str_to_double(char *str) {
   double to_ret = strtod(str, NULL);
   if (errno == ERANGE) {
      /* set itrerrno_g */
      itrerrno_g = lex_err;
      fatal_error("floating point literal is too large for its type\n");
   }
   return to_ret;
}

static void set_simple_token(token_t *tok, int ch) {
   switch (ch) {
   case '(':
      tok->t_type = t_left_parenth;
      break;
   case ')':
      tok->t_type = t_right_parenth;
      break;
   case '{':
      tok->t_type = t_left_brace;
      break;
   case '}':
      tok->t_type = t_right_brace;
      break;
   case ',':
      tok->t_type = t_comma;
      break;
   case '+':
      tok->t_type = t_op;
      tok->type_val.op = plus_o;
      break;
   case '*':
      tok->t_type = t_op;
      tok->type_val.op = mult_o;
      break;
   case '-':
      tok->t_type = t_op;
      tok->type_val.op = minus_o;
      break;
   case ';':
      tok->t_type = t_semicolon;
      break;
   default:
      itrerrno_g = lex_err;
      fatal_error("invalid character\n");
      break;
   }
}

token_t *lex_token_unget() {
   lexer->ungetch_flag = true;
   return lex_token_get_current();
}

void escape_seq_proc(char *s)
{
   int count = 0;
   for ( ; *s; ++s) {
      if (s[0] == '\\' && s[1] == '\"') {
         count = 1;
         s[0] = '\"';
      }
      else if (s[0] == '\\' && s[1] == 'n') {
         count = 1;
         s[0] = 0xa;
      }
      else if (s[0] == '\\' && s[1] == 't') {
         count = 1;
         s[0] = 0x9;
      }
      else if (s[0] == '\\' && s[1] == '\\') {
         count = 1;
      }
      else if (s[0] == '\\' && isdigit(s[1]) &&
               isdigit(s[2]) && isdigit(s[3]))
      {
         char escape[3];
         for (int i = 0; i < 3; i++) {
            escape[i] = s[1+i];
         }
         escape[3] = 0;
         unsigned oct;
         sscanf(escape, "%o", &oct);
         s[0] = oct;
         count = 3;
      }

      for (int j = 0; j < count; j++) {
         for (int i = 1; s[i]; i++) {
            s[i] = s[i + 1];
         }
      }
      count = 0;
   }
}

/*static enum states fsm_state = start_st;*/

/* FSM */
token_t *lex_token_get_next() {
   if (lexer->ungetch_flag == true) {
      lexer->ungetch_flag = false;
      return lex_token_get_current();
   }
   bool found_token = false;
   enum states fsm_state = start_st;
   while (!found_token) {
      int ch = buf_getch(lexer->buf);
      switch(fsm_state) {
      case start_st:
         if (isspace(ch)) fsm_state = start_st;
         else if (ch == '/') fsm_state = comment_begin_st;
         else if (id_begin_char(ch)) {
            fsm_state = id_simple_st;
            buf_ungetch(lexer->buf);
            buf_setlex(lexer->buf);
            buf_getch(lexer->buf);
         }
         else if (ch == '\"') {
            buf_setlex(lexer->buf);
            fsm_state = string_lit_st;
         }
         else if (isdigit(ch)) {
            fsm_state = int_lit_st;
            buf_ungetch(lexer->buf);
            buf_setlex(lexer->buf);
            buf_getch(lexer->buf);
         }
         else if (ch == '>') {
            fsm_state = relop_greater_eq_st;
         }
         else if (ch == '<') {
            fsm_state = relop_less_eq_st;
         }
         else if (ch == '=') {
            fsm_state = relop_eq_st;
         }
         else if (ch == '!') {
            fsm_state = relop_no_eq_st;
         }
         else if (ch == EOF) {
            return NULL;
         }
         else {
            set_simple_token(&lexer->curr_tok, ch);
            found_token = true;
         }
         break;
      case comment_begin_st:
         if (ch == '/') fsm_state = line_comment_st;
         else if (ch == '*') fsm_state = block_comment_st;
         else {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_op;
            lexer->curr_tok.type_val.op = div_o;
            found_token = true;
            fsm_state = start_st;
         }
         break;
      case block_comment_st:
         if (ch == '*') fsm_state = block_comment_end_st;
         else if (ch == EOF) fsm_state = comment_unterminated_error_st;
         else fsm_state = block_comment_st;
         break;
      case block_comment_end_st:
         if (ch == '/') fsm_state = start_st;
         else if (ch == EOF) fsm_state = comment_unterminated_error_st;
         else fsm_state = block_comment_st;
         break;
      case line_comment_st:
         if (ch != '\n' && ch != EOF) fsm_state = line_comment_st;
         else fsm_state = start_st;
         break;
      case comment_unterminated_error_st:
         /* set itrerrno_g */
         itrerrno_g = lex_err;
         fatal_error("unterminated comment\n");
         break;
      case id_simple_st:
         if (id_char(ch)) fsm_state = id_simple_st;
         else if (ch == '.') fsm_state = id_complex_begin_st;
         else {
            /* keyword control */
            buf_ungetch(lexer->buf);
            lexer->curr_tok.type_val.id_p = buf_getlex(lexer->buf);
            if (keyword(&lexer->curr_tok)) {
               found_token = true;
               fsm_state = start_st;
               break;
            }
            lexer->curr_tok.t_type = t_id_simple;
            found_token = true;
            fsm_state = start_st;
         }
         break;
      case id_complex_begin_st:
         if (id_begin_char(ch)) fsm_state = id_complex_st;
         else {
            /* set itrerrno_g syntax */
            itrerrno_g = lex_err;
            fatal_error("id expected after point\n");
         }
         break;
      case id_complex_st:
         if (id_char(ch)) fsm_state = id_complex_st;
         else {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_id_complex;
            lexer->curr_tok.type_val.id_p = buf_getlex(lexer->buf);
            found_token = true;
            fsm_state = start_st;
         }
         break;
      case string_lit_st:
         if (ch == '\"') {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_literal;
            lexer->curr_tok.type_val.literal = string_l;
            lexer->curr_tok.val.str = buf_getlex(lexer->buf);
            escape_seq_proc(lexer->curr_tok.val.str);
            buf_getch(lexer->buf);
            found_token = true;
            fsm_state = start_st;
         }
         else if (ch == '\\') {
            fsm_state = string_lit_escape_st;
         }
         else if (ch == EOF) {
            /* set itrerrno_g */
            itrerrno_g = lex_err;
            fatal_error("unterminated string literal\n");
         }
         break;
      case string_lit_escape_st:
         fsm_state = string_lit_st;
         break;
      case int_lit_st:
         if (ch == '.') fsm_state = float_lit_exp_begin_st;
         else if (ch == 'E' || ch == 'e') fsm_state = float_lit_exp_st;
         else if (!isdigit(ch)) {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_literal;
            lexer->curr_tok.type_val.literal = int_l;
            lexer->curr_tok.val.integer = str_to_int( buf_getlex(lexer->buf) );
            found_token = true;
            fsm_state = start_st;
         }
         break;
      case float_lit_exp_begin_st:
         if (isdigit(ch)) fsm_state = float_lit_exp_digits_st;
         else if (ch == 'e' || ch == 'E') fsm_state = float_lit_exp_st;
         else {
            /* set itrerrno_g */
            itrerrno_g = lex_err;
            fatal_error("digits expected after point\n");
         }
         break;
      case float_lit_exp_st:
         if (isdigit(ch)) fsm_state = float_lit_exp_digits_st;
         else if (ch == '+' || ch == '-') fsm_state = float_lit_exp_digits_st;
         else {
            /* set itrerrno_g */
            itrerrno_g = lex_err;
            fatal_error("digits expected after exponent\n");
         }
         break;
      case float_lit_exp_digits_st:
         if (ch == 'E' || ch == 'e') fsm_state = float_lit_exp_st;
         else if (!isdigit(ch)) {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_literal;
            lexer->curr_tok.type_val.literal = float_l;
            lexer->curr_tok.val.f_pnt = str_to_double( buf_getlex(lexer->buf) );
            found_token = true;
            fsm_state = start_st;
         }
         break;
      case relop_greater_eq_st:
         lexer->curr_tok.t_type = t_relop;
         if (ch == '=') {
            lexer->curr_tok.type_val.relop = geq_ro;
         }
         else {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.type_val.relop = gt_ro;
         }
         found_token = true;
         fsm_state = start_st;
         break;
      case relop_less_eq_st:
         lexer->curr_tok.t_type = t_relop;
         if (ch == '=') {
            lexer->curr_tok.type_val.relop = leq_ro;
         }
         else {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.type_val.relop = lt_ro;
         }
         found_token = true;
         fsm_state = start_st;
         break;
      case relop_eq_st:
         if (ch == '=') {
            lexer->curr_tok.t_type = t_relop;
            lexer->curr_tok.type_val.relop = eq_ro;
         }
         else {
            buf_ungetch(lexer->buf);
            lexer->curr_tok.t_type = t_assignment;
         }
         found_token = true;
         fsm_state = start_st;
         break;
      case relop_no_eq_st:
         if (ch == '=') {
            lexer->curr_tok.t_type = t_relop;
            lexer->curr_tok.type_val.relop = noeq_ro;
            found_token = true;
            fsm_state = start_st;
         }
         else {
            /* set itrerrno_g */
            itrerrno_g = lex_err;
            fatal_error("simple relation operator '!' " 
                        "is no valid in our interpret version\n");
         }
         break;
      }
   }

   return &lexer->curr_tok;
}
