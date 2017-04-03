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
#include <string.h>
#include <errno.h>
#include "restab.h"
#include "itrerror.h"

struct lop_item {
   void *ptr;
   struct lop_item *next;
};

struct list_of_pointers {
   struct lop_item *first;
};

struct resourse_table {
   unsigned count;
   unsigned curr_cap;
   struct list_of_pointers *arrlist;
};

static unsigned curr_hash_size_sg = 0;
static unsigned max_size_i_sg     = 17;
static unsigned hash_sizes_sg[]   = {
   769,     1543,    3079,       6151,       12289,      24593,
   49157,   98317,   96613,      393241,     786433,     1572869,
   3145739, 6291469, 12582917,   25165843,   50331653,   100663319
};

static struct resourse_table *resourse_table_g;

static void lop_insert_item(struct list_of_pointers *lop, 
                            struct lop_item *item) 
{
   item->next = lop->first;
   lop->first = item;
}

static void lop_insert_ptr(struct list_of_pointers *lop, void *new_ptr) {
   struct lop_item *new_item = malloc(sizeof(*new_item));
   if (NULL == new_item) {
      itrerrno_g = iternal_compile_err;
      delete_resource_table_and_free_all_resources();
      fatal_error("couldn't allocate memory\n");
   }

   new_item->ptr = new_ptr;
   lop_insert_item(lop, new_item);
}

static struct lop_item *lop_uninsert_ptr(struct list_of_pointers *lop, 
                                         void *ptr) 
{
   struct lop_item **prev_item_next_addr = &lop->first;
   for (struct lop_item *tmp = lop->first; tmp; tmp = tmp->next) {
      if (tmp->ptr == ptr) {
         *prev_item_next_addr = tmp->next;
         return tmp;
      }
      prev_item_next_addr = &tmp->next;
   }

   return NULL;
}

/*
 * Knuth's multiplicative method: 
 */
static unsigned hash_func(void *ptr) {
   unsigned long long val = (unsigned long long) ptr;
   unsigned index = val*2654435761ull % resourse_table_g->curr_cap;
   return index;
}

void resourse_table_init() {
   resourse_table_g = malloc(sizeof(*resourse_table_g));  
   if (NULL == resourse_table_g) {
      itrerrno_g = iternal_compile_err;
      fatal_error("couldn't allocate memory\n");
   }

   unsigned new_cap = hash_sizes_sg[curr_hash_size_sg];
   ++curr_hash_size_sg;
   resourse_table_g->arrlist = malloc(sizeof(struct list_of_pointers)*new_cap);
   if (NULL == resourse_table_g->arrlist) {
      itrerrno_g = iternal_compile_err;
      fatal_error("couldn't allocate memory\n");
      free(resourse_table_g);
   }

   for (unsigned i = 0; i < new_cap; ++i)
      resourse_table_g->arrlist[i].first = NULL;

   resourse_table_g->count = 0;
   resourse_table_g->curr_cap = new_cap;
   return;
}

static void resize_table() {
   struct resourse_table *tab = resourse_table_g;
   unsigned new_cap = hash_sizes_sg[curr_hash_size_sg];
   ++curr_hash_size_sg;
   struct list_of_pointers *new_arrlist = malloc(sizeof(*new_arrlist)*new_cap);
   /* TODO NULL ptr handling */

   for (unsigned i = 0; i < new_cap; ++i)
      new_arrlist[i].first = NULL;

   for (unsigned i = 0; i < tab->curr_cap; ++i) {
      struct lop_item *next_item;
      for (struct lop_item *tmp = tab->arrlist[i].first; 
           tmp != NULL; tmp = next_item) 
      {
         next_item = tmp->next;
         unsigned index = hash_func(tmp->ptr);
         lop_insert_item(&new_arrlist[index], tmp);
      }
   }

   free(tab->arrlist);
   tab->arrlist = new_arrlist;
   tab->curr_cap = new_cap;
}

static void rtab_add_res(void *ptr) {
   if (resourse_table_g->count == resourse_table_g->curr_cap) {
      if (curr_hash_size_sg != max_size_i_sg)
         resize_table(resourse_table_g);
   }

   unsigned index = hash_func(ptr);
   lop_insert_ptr(&resourse_table_g->arrlist[index], ptr);
   ++resourse_table_g->count;
}

void delete_resource_table_and_free_all_resources() {
   struct resourse_table *tab = resourse_table_g;
   if (tab == NULL)
      return;

   for (unsigned i = 0; i < tab->curr_cap; ++i) {
      struct lop_item *next_item;
      for (struct lop_item *tmp = tab->arrlist[i].first; 
           tmp != NULL; tmp = next_item) 
      {
         next_item = tmp->next;
         free(tmp->ptr);
         free(tmp);
      }
   }
   free(tab->arrlist);
   free(tab);

   if (source_file_g) {
      if (fclose(source_file_g)) {
         itrerrno_g = iternal_compile_err;
         fatal_error("cannot close source file: %s", strerror(errno));
      }
   }
}

static struct lop_item *rtab_get_res(void *ptr) {
   unsigned i = hash_func(ptr);
   struct list_of_pointers *found_list = &resourse_table_g->arrlist[i];
   struct lop_item *res = lop_uninsert_ptr(found_list, ptr);
   if (res == NULL) {
      delete_resource_table_and_free_all_resources();
      itrerrno_g = iternal_compile_err;
      fatal_error("cannot find resource\n");
   }

   return res;
}

void *rtab_malloc(size_t size) {
   void *ret_ptr = malloc(size);
   if (NULL == ret_ptr) {
      itrerrno_g = iternal_compile_err;
      delete_resource_table_and_free_all_resources();
      fatal_error("couldn't allocate memory\n");
   }

   rtab_add_res(ret_ptr);
   return ret_ptr;
}

void *rtab_realloc(void *ptr, size_t size) {
   void *ret_ptr;
   if (NULL == ptr) {
      ret_ptr = rtab_malloc(size);
      return ret_ptr;
   }

   ret_ptr = realloc(ptr, size);
   if (NULL == ret_ptr) {
      itrerrno_g = iternal_compile_err;
      fatal_error("couldn't allocate memory\n");
   }

   if (ret_ptr != ptr) {
      struct lop_item *res = rtab_get_res(ptr);
      res->ptr = ret_ptr;
      unsigned i = hash_func(ret_ptr);
      struct list_of_pointers *found_list = &resourse_table_g->arrlist[i];
      lop_insert_item(found_list, res);
   }
   return ret_ptr;
}

void rtab_free(void *ptr) {
   if (ptr == NULL) {
      return;
   }
   
   struct lop_item *res = rtab_get_res(ptr);
   --resourse_table_g->count;
   free(res->ptr);
   free(res);
}
