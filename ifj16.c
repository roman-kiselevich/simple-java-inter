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

#include <string.h>
#include <errno.h>
#include "prectab.h"
#include "restab.h"
#include "lexer.h"
#include "itrerror.h"
#include "fcallstack.h"
#include "soexpitems.h"
#include "ilist.h"
#include "parser.h"
#include "inter.h"
#include "symtab.h"

int main(int argc, char **argv) {
   (void)argc; /* to avoid compiler warnings */
   source_file_g = fopen(argv[1], "r");
   if (NULL == source_file_g) {
      itrerrno_g = iternal_compile_err;
      fatal_error("smth goes wrong while opening file: %s\n", strerror(errno));
   }

   resourse_table_init();
   lexer_init(source_file_g);
   init_symbol_table();

   ifj16_add_funcs_to_symtab();

   init_stack_exp_items();
   init_list_instructions();
   init_precedence_tab_parser();
   run_parsing();
   soeitems_stack_items_control();
   init_function_call_stack();

   run_interpret();

   delete_resource_table_and_free_all_resources();
   return exit_success;
}
