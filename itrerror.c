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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "itrerror.h"
#include "restab.h"

enum err_codes itrerrno_g;

FILE *source_file_g;

static char *error_msgs[] = {
   "exit_sucess",
   "lexical error",
   "syntax error",
   "undefined function, variable or class, or redefinition",
   "compatibility error type or number of params don't match to each other",
   "some semantic error",
   "wrong numeric values on the input",
   "uninitialized variables",
   "an attempt to divide by zero",
   "some runtime error",
   "error uninfluenced by the entry program"
};

void fatal_error(const char *fmt, ...) {
   va_list arguments;

   va_start(arguments, fmt);

   fprintf(stderr, "interpret fatal error: *** \n");
   vfprintf(stderr, fmt, arguments);
   fprintf(stderr, "\nprogram terminated with code %d\n", itrerrno_g);
   
   int error_index = itrerrno_g;
   if (itrerrno_g == iternal_compile_err) {
      error_index = 10;
   }
   fprintf(stderr, "error type: %s\n", error_msgs[error_index]);

   if (source_file_g) {
      if (fclose(source_file_g)) {
         fprintf(stderr, "cannot close source file: %s", strerror(errno));
      }
      source_file_g = NULL;
   }

   delete_resource_table_and_free_all_resources();
   va_end(arguments);
   fflush(stderr);
   exit(itrerrno_g);
}
