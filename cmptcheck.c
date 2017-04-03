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

#include "cmptcheck.h"
#include "itrerror.h"

void compatibility_check_var_var(enum type_variable left,
                                 enum type_variable right)
{
   switch (left) {
   case t_var_int:
      if (right == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot convert string to int");
      }
      break;
   case t_var_float:
      if (right == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot convert string to double");
      }
      break;
   case t_var_string:
      if (right != t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot use strings with other types\n\t"
                            "or other operators except \'+\'");
      }
      break;
   case t_var_void:
      itrerrno_g = semant_match_err;
      fatal_error("cannot use strings with other types\n\t"
                         "or other operators except \'+\'");
   }
}

void compatibility_check_var_etree(enum type_variable res_t, etree_t* etree) {
   enum type_variable etree_res_type = etree_get_result_type(etree);
   switch (etree_res_type) {
   case t_var_int:
      if (res_t == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot convert string to int");
      }
      break;
   case t_var_float:
      if (res_t == t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot convert string to double");
      }
      break;
   case t_var_string:
      if (res_t != t_var_string) {
         itrerrno_g = semant_match_err;
         fatal_error("cannot use strings with other types\n\t"
                            "or other operators except \'+\'");
      }
      break;
   case t_var_void:
      itrerrno_g = semant_match_err;
      fatal_error("cannot use strings with other types\n\t"
                         "or other operators except \'+\'");
   }
}
