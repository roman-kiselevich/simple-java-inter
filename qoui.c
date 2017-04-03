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
 *        Kiselevich Roman
 */


#include <stdio.h>
#include <stdlib.h>
#include "qoui.h"
#include "boolean.h"
#include "restab.h"

struct queue_of_uint {
   unsigned *arrint;
   unsigned *first;
   unsigned *last;

   unsigned curr_cap;
   unsigned count;
   unsigned *eoqueue;
};

enum { qoui_realloc_c = 4 };

qoui_t *qoui_init() {
   qoui_t *new_qoui = rtab_malloc(sizeof(*new_qoui));

   new_qoui->arrint = rtab_malloc(qoui_realloc_c*sizeof(unsigned));
   new_qoui->first = new_qoui->arrint;
   new_qoui->last = new_qoui->first;
   new_qoui->curr_cap = qoui_realloc_c;
   new_qoui->count = 0;
   new_qoui->eoqueue = &new_qoui->arrint[new_qoui->curr_cap - 1];
   return new_qoui;
}

static void qoui_refresh(qoui_t *qoui) {
   if (qoui->curr_cap > qoui->count) {
      for (unsigned *tmp = qoui->arrint;
           qoui->first != qoui->last; ++tmp, ++qoui->first) 
      {
         *tmp = *qoui->first;
      }

      qoui->first = qoui->arrint;
      qoui->last = qoui->first + qoui->count;
   }
   else {
      qoui->curr_cap += qoui_realloc_c;
      qoui->arrint = rtab_realloc(qoui->arrint, 
                                  sizeof(unsigned)*(qoui->curr_cap + 1));
      qoui->first = qoui->arrint;
      qoui->last = qoui->first + qoui->count;
      qoui->eoqueue = &qoui->arrint[qoui->curr_cap];
   }
}

void qoui_put(qoui_t *qoui, unsigned value) {
   if (qoui->last > qoui->eoqueue)
      qoui_refresh(qoui);

   *qoui->last = value;
   ++qoui->last;
   ++qoui->count;
}

unsigned qoui_get(qoui_t *qoui) {
   if (0 == qoui->count)
      return 0;

   --qoui->count;

   /* return then inc */
   return *qoui->first++;
}

unsigned qoui_get_back(qoui_t *qoui) {
   if (0 == qoui->count)
      return 0;

   --qoui->count;

   /* dec then return */
   return *--qoui->last;
}

unsigned qoui_get_count(qoui_t *qoui) {
   return qoui->count;
}

unsigned *qoui_is_elem_in(qoui_t *qoui, unsigned elem) {
   for (unsigned *tmp = qoui->first; tmp != qoui->last; ++tmp) {
      if (*tmp == elem)
         return tmp;
   }
   return NULL;
}