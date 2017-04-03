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

#include "fcallstack.h"
#include "restab.h"
#include "symtab.h"
#include "itrerror.h"

static struct function_call_stack {
   size_t stack_size;
   struct selem *arrel;
   unsigned count;
} *fcalls_sg;

struct selem {
   instr_t *func_call;
   union var_info_val *arrvals;
};

enum { fcstack_max_size = 8388608 }; /* 8MiB standart system stack size */
enum { fcstack_max_cap  = 100000  };

void init_function_call_stack() {
   fcalls_sg = rtab_malloc(sizeof(*fcalls_sg));
   fcalls_sg->stack_size = 0;
   fcalls_sg->arrel = rtab_malloc(sizeof(struct selem)*fcstack_max_cap);
   fcalls_sg->count = 0;
}

void fcalls_push_func(instr_t *func_call) {
   if (fcalls_sg->count == fcstack_max_cap) {
      itrerrno_g = runtime_other_err;
      fatal_error("stack overflow: max_stack_size=%u byte", fcstack_max_size);
   }

   unsigned index = fcalls_sg->count;
   fcalls_sg->arrel[index].func_call = func_call;
   unsigned num_defvars = func_call->func_info->num_defvars;
   size_t arr_cap = func_call->func_info->num_args + num_defvars;
   size_t arrsize = sizeof(union var_info_val)*arr_cap;
   fcalls_sg->stack_size += arrsize;
   if (fcalls_sg->stack_size > fcstack_max_size) {
      itrerrno_g = runtime_other_err;
      fatal_error("stack overflow: max_stack_size=%u byte", fcstack_max_size);
   }

   if (arrsize) {
      fcalls_sg->arrel[index].arrvals = rtab_malloc(arrsize);
   }
   else {
      fcalls_sg->arrel[index].arrvals = NULL;
   }
   
   union var_info_val *arrvals = fcalls_sg->arrel[index].arrvals;
   for (unsigned i = 0; i < func_call->func_info->num_args; ++i) {
      enum type_variable tval;
      etree_node_t *root = func_call->arretrees[i]->root;
      union var_info_val val = etree_get_expr_result(root, &tval);
      arrvals[i] = val;
   }

   unsigned i = 0;
   for (unsigned i = 0; i < func_call->func_info->num_args; ++i) {
      func_call->func_info->arrargs[i]->var_ptr = &arrvals[i];
   }

   for (unsigned j = 0; j < num_defvars; ++j, ++i) {
      func_call->func_info->arrvar_ptrs[j]->var_ptr = &arrvals[i];
   }

   ++fcalls_sg->count;
}

instr_t *fcalls_pop_func() {
   if (fcalls_sg->count == 0) {
      return NULL;
   }

   --fcalls_sg->count;
   size_t deleted_cap = 
      fcalls_sg->arrel[fcalls_sg->count].func_call->num_params;
   size_t deleted_size = sizeof(union var_info_val)*deleted_cap;
   fcalls_sg->stack_size -= deleted_size;
   rtab_free(fcalls_sg->arrel[fcalls_sg->count].arrvals); /* TODO free strings */

   unsigned index = fcalls_sg->count - 1;
   instr_t *func_call;
   func_call = fcalls_sg->arrel[fcalls_sg->count].func_call;
   unsigned num_defvars = func_call->func_info->num_defvars;
   union var_info_val *arrvals = fcalls_sg->arrel[index].arrvals;
   unsigned i;
   for (i = 0; i < func_call->func_info->num_args; ++i) {
      func_call->func_info->arrargs[i]->var_ptr = &arrvals[i];
   }

   for (unsigned j = 0; j < num_defvars; ++j, ++i) {
      func_call->func_info->arrvar_ptrs[j]->var_ptr = &arrvals[i];
   }
   return func_call;
}
