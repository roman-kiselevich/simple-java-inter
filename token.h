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
 *    Module describing the type of token
 */

#ifndef TOKEN_H
#define TOKEN_H

typedef struct token {
   enum {
      t_keyword,
      t_literal,
      t_left_parenth,  /* ( */
      t_right_parenth, /* ) */
      t_left_brace,    /* { */
      t_right_brace,   /* } */
      t_comma,
      t_assignment,
      t_semicolon,
      t_op,
      t_relop,
      t_id_simple,
      t_id_complex
   } t_type;

   union {
      enum tok_keywords {
         tkw_int,
         tkw_double,
         tkw_string,
         tkw_void,
         tkw_return,
         tkw_static,
         tkw_if,
         tkw_else,
         tkw_class,
         tkw_while
      } keyword;

      enum {
         int_l,
         float_l,
         string_l
      } literal;

      enum {
         plus_o,
         mult_o,
         div_o,
         minus_o
      } op;

      enum {
         gt_ro,   /* >  */
         geq_ro,  /* >= */
         lt_ro,   /* <  */
         leq_ro,  /* <= */
         eq_ro,   /* == */
         noeq_ro  /* != */
      } relop;

      char *id_p;
   } type_val; /* union */

   union {
      char *str;
      int integer;
      double f_pnt;
   } val;
} token_t;

#endif /* TOKEN_H */
