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


#include "kwrecognizer.h"
#include "restab.h"
#include <string.h>

enum kwrec_fsm_states {
   st_kwrec_start,
   st_kwrec_while,
   st_kwrec_class,
   st_kwrec_return,
   st_kwrec_double,

   st_kwrec_if_int_i,
   st_kwrec_if,
   st_kwrec_int,

   st_kwrec_static,
   st_kwrec_String,
   st_kwrec_void,
   st_kwrec_else,
   st_kwrec_exit,
};

static void set_first_keyword_state(int ch, enum kwrec_fsm_states *state_ptr) {
   switch (ch) {
   case 'w':
      *state_ptr = st_kwrec_while;
      break;
   case 'c':
      *state_ptr = st_kwrec_class;
      break;
   case 'r':
      *state_ptr = st_kwrec_return;
      break;
   case 'd':
      *state_ptr = st_kwrec_double;
      break;
   case 'i':
      *state_ptr = st_kwrec_if_int_i;
      break;
   case 'S':
      *state_ptr = st_kwrec_String;
      break;
   case 's':
      *state_ptr = st_kwrec_static;
      break;
   case 'v':
      *state_ptr = st_kwrec_void;
      break;
   case 'e':
      *state_ptr = st_kwrec_else;
      break;
   default:
      *state_ptr = st_kwrec_exit;
      break;
   }
}

enum {
   kwrec_while_maxcmp    = 6,
   kwrec_class_maxcmp    = 6,
   kwrec_return_maxcmp   = 7,
   kwrec_double_maxcmp   = 7,
   kwrec_if_maxcmp       = 3,
   kwrec_int_maxcmp      = 4,
   kwrec_static_maxcmp   = 7,
   kwrec_string_maxcmp   = 7,
   kwrec_void_maxcmp     = 5,
   kwrec_else_maxcmp     = 5,
};

/* FSM */
bool keyword(token_t *tok) {
   char *probable_keyword = tok->type_val.id_p;
   enum kwrec_fsm_states kwrec_state = st_kwrec_start;
   while (*probable_keyword) {
      switch (kwrec_state) {
      case st_kwrec_start:
         set_first_keyword_state(*probable_keyword, &kwrec_state);
         break;
      case st_kwrec_while:
         if (!strncmp("while", probable_keyword, kwrec_while_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_while;
            return true;
         }
         else return false;
         break;
      case st_kwrec_class:
         if (!strncmp("class", probable_keyword, kwrec_class_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_class;
            return true;
         }
         else return false;
         break;
      case st_kwrec_return:
         if (!strncmp("return", probable_keyword, kwrec_return_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_return;
            return true;
         }
         else return false;
         break;
      case st_kwrec_double:
         if (!strncmp("double", probable_keyword, kwrec_double_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_double;
            return true;
         }
         else return false;
         break;
      case st_kwrec_if_int_i:
         ++probable_keyword;
         if (*probable_keyword == 'f') kwrec_state = st_kwrec_if;
         else if (*probable_keyword == 'n') kwrec_state = st_kwrec_int;
         else return false;
         break;
      case st_kwrec_if:
         --probable_keyword;
         if (!strncmp("if", probable_keyword, kwrec_if_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_if;
            return true;
         }
         else return false;
         break;
      case st_kwrec_int:
         --probable_keyword;
         if (!strncmp("int", probable_keyword, kwrec_int_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_int;
            return true;
         }
         else return false;
         break;
      case st_kwrec_static:
         if (!strncmp("static", probable_keyword, kwrec_static_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_static;
            return true;
         }
         else return false;
         break;
      case st_kwrec_String:
         if (!strncmp("String", probable_keyword, kwrec_string_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_string;
            return true;
         }
         else return false;
         break;
      case st_kwrec_void:
         if (!strncmp("void", probable_keyword, kwrec_void_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_void;
            return true;
         }
         else return false;
         break;
      case st_kwrec_else:
         if (!strncmp("else", probable_keyword, kwrec_else_maxcmp)) {
            tok->t_type = t_keyword;
            rtab_free(probable_keyword);
            tok->type_val.keyword = tkw_else;
            return true;
         }
         else return false;
         break;
      case st_kwrec_exit:
         return false;
         break;
      } /* switch */
   } /* while */

   return false;
}
