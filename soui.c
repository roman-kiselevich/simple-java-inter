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


#include <stdio.h>
#include <stdlib.h>
#include "soui.h"
#include "boolean.h"
#include "restab.h"

struct stack_of_uint {
   unsigned *arruint;
   unsigned *first;
   unsigned *last;

   unsigned curr_cap;
   unsigned count;
   unsigned *eostack;
};

enum { soui_realloc_c = 8 };

soui_t *soui_init() {
   soui_t *new_soui = rtab_malloc(sizeof(*new_soui));
   new_soui->arruint = rtab_malloc(soui_realloc_c*sizeof(unsigned));

   new_soui->first = new_soui->arruint;
   new_soui->last = new_soui->first;
   new_soui->curr_cap = soui_realloc_c;
   new_soui->count = 0;
   new_soui->eostack = &new_soui->arruint[soui_realloc_c - 1];
   return new_soui;
}

void soui_push(soui_t *soui, unsigned value) {
   if (soui->last > soui->eostack) {
      size_t curr_size = soui->curr_cap*sizeof(unsigned);
      size_t add_size = soui_realloc_c*sizeof(unsigned); 
      soui->arruint = rtab_realloc(soui->arruint, curr_size + add_size);

      soui->first = soui->arruint;
      soui->last = soui->first + soui->count;
      soui->eostack = soui->arruint + soui->curr_cap - 1;
   }

   *soui->last = value;
   ++soui->last;
   ++soui->count;
}

unsigned soui_pop(soui_t *soui) {
   if (0 == soui->count)
      return 0;

   --soui->count;
   /* ret & dec */
   return *soui->last--;
}

bool soui_empty(soui_t *soui) {
   if (0 == soui->count)
      return true;

   return false;
}