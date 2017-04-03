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
 *          Kiselevich Roman 
 * Description:
 *   Module for work with errors
 */

#ifndef ITR_ERROR_H
#define ITR_ERROR_H

#include <stdio.h>

enum err_codes {
                              /* err codes description */
   exit_success         = 0,
   lex_err              = 1,  /* lexical error */
   synt_err             = 2,  /* syntax error */
   
   semant_undef_err     = 3,  /* in case when we have undefined function,
                               * variable or class, 
                               * or just trying to redefine them
                               */
   
   semant_match_err     = 4,  /* compatibility error; 
                               * type or number of parametres 
                               * don't match to each other
                               */
                                 
   semant_other_err     = 6,  /* other semantic errors */
   runtime_invalue_err  = 7,  /* wrong numeric values on the input */
   runtime_uninit_err   = 8,  /* uninitialized variables */
   runtime_zerodiv_err  = 9,  /* an attempt to divide by zero */
   runtime_other_err    = 10, /* other runtime errors */
   
   iternal_compile_err  = 99, /* error uninfluenced by the entry program; 
                               * including memory allocation,
                               * file opening errors etc.
                               */
};

extern enum err_codes itrerrno_g;

extern FILE *source_file_g;

/**
 * Write out the error to stderr as an format string 
 * and completes the program in accordance with the 
 * established variable itrerrno_g
  * @param formatting string works similarly to printf ()
 */
void fatal_error(const char *fmt, ...);

#endif /* ITR_ERROR_H */
