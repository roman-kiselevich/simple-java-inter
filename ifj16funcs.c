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
 *         Inhliziian Bohdan
 */


#include "ifj16funcs.h"
#include "symtab.h"
#include "restab.h"
#include <string.h>

static char *funcs_ids[] = {
   "ifj16.readInt",
   "ifj16.readDouble",
   "ifj16.readString",
   "ifj16.print",
   "ifj16.length",
   "ifj16.substr",
   "ifj16.compare",
   "ifj16.find",
   "ifj16.sort",
};

enum ifj16_funcs ifj16funcs_recognizer(char *probable_ifj16) {
   enum ifj16_funcs func_index = ifj16_read_int;
   for ( ; func_index < sizeof(funcs_ids)/sizeof(funcs_ids[0]); ++func_index) {
      size_t len_id = strlen(funcs_ids[func_index]);
      if (!strncmp(funcs_ids[func_index], probable_ifj16, len_id + 1)) {
         return func_index;
      }
   }

   return ifj16_not_ifj16;
}

char *ifj16_get_func_id(enum ifj16_funcs func) {
   if (func == ifj16_not_ifj16) {
      return NULL;
   }
   else {
      return funcs_ids[func];
   }
}

void ifj16_add_funcs_to_symtab() {
   func_info_t func;
   enum ifj16_funcs func_index = ifj16_read_int;
   for ( ; func_index != ifj16_not_ifj16; ++func_index) {
      char *key = funcs_ids[func_index];
      func.ifj16_func = func_index;
      switch (func_index) {
      case ifj16_read_int:
         func.ret_type = t_var_int;
         func.num_args = 0;
         break;
      case ifj16_read_double:
         func.ret_type = t_var_float;
         func.num_args = 0;
         break;
      case ifj16_read_string:
         func.ret_type = t_var_string;
         func.num_args = 0;
         break;
      case ifj16_length:
         func.ret_type = t_var_int;
         func.num_args = 1;
         break;
      case ifj16_substr:
         func.ret_type = t_var_string;
         func.num_args = 3;
         break;
      case ifj16_compare:
         func.ret_type = t_var_int;
         func.num_args = 2;
         break;
      case ifj16_find:
         func.ret_type = t_var_int;
         func.num_args = 2;
         break;
      case ifj16_sort:
         func.ret_type = t_var_string;
         func.num_args = 1;
         break;
      default:
         func.ret_type = t_var_void;
         func.num_args = 1;
      }
      symtab_insert_item(t_item_func, key, &func);
   }
}

int ifj16_find_method(char *s, char *search) {
   int i, j;
   int string_len = strlen(s);
   int search_len = strlen(search);
   if (search_len > string_len) {
      return -1;
   } else if (search_len == 0) {
      return 0;
   }

   int *mem = rtab_malloc(search_len*sizeof(int));

   /* prefix function */
   mem[0] = 0;
   for (i = 1, j = 0; i < search_len; i++) {
      while (j > 0 && search[j] != search[i]) {
         j = mem[j - 1];
      }

      if (search[j] == search[i]) {
         j++;
      }
      mem[i] = j;
   }
   

   /* search substring index in start string */
   for (i = 0,j = 0; i < string_len; i++) {
      while (j > 0 && search[j] != s[i]) {
         j=mem[j - 1];
      }

      if (search[j]==s[i]) {
         j++;
      }
      
      if (j==search_len) {
         rtab_free(mem);
         return i - j + 1;
      }
   }

   rtab_free(mem);
   return -1;
}

static void merge(char *a, size_t n, size_t m) {
   size_t i, j, k;
   char *x = rtab_malloc(n*sizeof(char));
   for (i = 0, j = m, k = 0; k < n; k++) {
      x[k] = j == n      ? a[i++]
           : i == m      ? a[j++]
           : a[j] < a[i] ? a[j++]
           :               a[i++];
   }

   for (i = 0; i < n; i++) {
      a[i] = x[i];
   }

   rtab_free(x);
}
 
void ifj16_merge_sort_method(char *str, size_t n) {
   if (n < 2) {
      return;
   }

   size_t m = n / 2;
   ifj16_merge_sort_method(str, m);
   ifj16_merge_sort_method(str + m, n - m);
   merge(str, n, m);
}
