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

#include "symtab.h"
#include "restab.h"
#include "boolean.h"
#include "itrerror.h"
#include <string.h>

static unsigned curr_hash_size_sg = 0;
static unsigned max_size_i_sg     = 17;
static unsigned hash_sizes_sg[]   = {
   769,     1543,    3079,       6151,       12289,      24593,
   49157,   98317,   96613,      393241,     786433,     1572869,
   3145739, 6291469, 12582917,   25165843,   50331653,   100663319
};

static struct symbol_table {
   unsigned count;
   unsigned capacity;
   struct symtab_item **arritem_ptr;
} *symtab_sg;

struct symtab_item {
   enum type_item t_item;
   union {
      struct function_information *func_info;
      struct variable_information *var_info;
      void *info;
   };

   char *key;
   struct symtab_item *next;
};

static unsigned long hash(char *key, unsigned symtab_cap);

static struct symtab_item *append_item(struct symtab_item **fst_item_ptr_addr,
                                       char *new_key,
                                       enum type_item t_new_item);

static void attach_func_info(struct symtab_item *item_ptr, 
                             struct function_information *func_info);

static void attach_var_info(struct symtab_item *item_ptr, 
                            struct variable_information *var_info);

static void resize_symtab();

/* sdbm algorithm */
static unsigned long hash(char *key, unsigned symtab_cap) {
   int c;
   unsigned long hash = 0;
   while ((c = *key++) != '\0')
      hash = c + (hash << 6) + (hash << 16) - hash;

   return hash % symtab_cap;
}

char *symtab_keygen(char *class_id, char *func_id, char *id) {
   if (!class_id) {
      itrerrno_g = runtime_other_err;
      fatal_error("smth goes wrong in symtab_keygen\n");
   }

   char *new_key;
   size_t new_key_size;
   if (func_id) {
      /* +3 here is for two dots and null byte */
      new_key_size = strlen(id) + strlen(class_id) + strlen(func_id) + 3;
      new_key = rtab_malloc(new_key_size);
      snprintf(new_key, new_key_size, "%s.%s.%s", class_id, func_id, id);
   }
   else {
      /* +2 for one dot and null byte */
      new_key_size = strlen(id) + strlen(class_id) + 2;
      new_key = rtab_malloc(new_key_size);
      snprintf(new_key, new_key_size, "%s.%s", class_id, id);
   }

   return new_key;
}

void init_symbol_table() {
   symtab_sg = rtab_malloc(sizeof(*symtab_sg));
   unsigned new_cap = hash_sizes_sg[curr_hash_size_sg];
   symtab_sg->capacity = new_cap;
   ++curr_hash_size_sg;
   symtab_sg->count = 0;

   symtab_sg->arritem_ptr = rtab_malloc(new_cap*sizeof(struct symtab_item*));
   for (unsigned i = 0; i < new_cap; ++i) {
      symtab_sg->arritem_ptr[i] = NULL;
   }
}

void *symtab_get_item_info(enum type_item t_search_item,
                           char *key_search_item)
{
   unsigned index = hash(key_search_item, symtab_sg->capacity);
   struct symtab_item *first = symtab_sg->arritem_ptr[index];
   for (struct symtab_item *tmp = first; tmp; tmp = tmp->next) {
      if (!strncmp(tmp->key, key_search_item, strlen(key_search_item) + 1)) {
         if (tmp->t_item == t_search_item) {
            return tmp->info;
         }
      }
   }

   return NULL;
}

void *symtab_insert_item(enum type_item t_new_item,
                         char *new_key, 
                         void *item_info)
{
   if (symtab_sg->count == symtab_sg->capacity) {
      if (curr_hash_size_sg != max_size_i_sg)
         resize_symtab();
   }

   unsigned index = hash(new_key, symtab_sg->capacity);
   struct symtab_item *new_item;
   new_item = append_item(&symtab_sg->arritem_ptr[index], new_key, t_new_item);
   if (NULL == new_item) {
      itrerrno_g = semant_undef_err;
      fatal_error("redefinition of %s\n", new_key);
   }

   ++symtab_sg->count;
   switch (t_new_item) {
   case t_item_func:
      attach_func_info(new_item, item_info);
      break;
   case t_item_var:
      attach_var_info(new_item, item_info);
      break;
   case t_item_class:
      break;
   }

   return new_item->info;
}

static struct symtab_item *append_item(struct symtab_item **fst_item_ptr_addr,
                                       char *new_key,
                                       enum type_item t_new_item)
{
   struct symtab_item *new_item = rtab_malloc(sizeof(*new_item));
   new_item->next = NULL;
   new_item->key = new_key;
   new_item->t_item = t_new_item;

   if (*fst_item_ptr_addr) {
      struct symtab_item *last_item;
      struct symtab_item *tmp;
      for (tmp = *fst_item_ptr_addr; tmp; tmp = tmp->next) {
         if (!strncmp(tmp->key, new_key, strlen(new_key) + 1)) {
            if (tmp->t_item == t_new_item) {
               return NULL;
            }
         }
         last_item = tmp;
      }
      last_item->next = new_item;
   }
   else {
      *fst_item_ptr_addr = new_item;
   }
   
   return new_item;
}

static void attach_func_info(struct symtab_item *item_ptr, 
                             struct function_information *func_info)
{
   struct function_information *new_func_info;
   new_func_info = rtab_malloc(sizeof(*new_func_info));
   item_ptr->func_info = new_func_info;

   new_func_info->num_args = func_info->num_args;
   new_func_info->arrargs = func_info->arrargs;
   new_func_info->first_instruction = func_info->first_instruction;
   new_func_info->num_defvars = func_info->num_defvars;
   new_func_info->arrvar_ptrs = func_info->arrvar_ptrs;
   new_func_info->ret_type = func_info->ret_type;
   new_func_info->ifj16_func = func_info->ifj16_func;
}

static void attach_var_info(struct symtab_item *item_ptr, 
                            struct variable_information *var_info)
{
   struct variable_information *new_var_info;
   new_var_info = rtab_malloc(sizeof(*new_var_info));
   item_ptr->var_info = new_var_info;

   /* init */
   new_var_info->t_var = var_info->t_var;
   new_var_info->init = var_info->init;
   new_var_info->var_ptr = var_info->var_ptr;
}

static void resize_symtab() {
   unsigned new_capacity = hash_sizes_sg[curr_hash_size_sg];
   ++curr_hash_size_sg;

   struct symtab_item **new_arritem_ptr;
   new_arritem_ptr = rtab_malloc(sizeof(*new_arritem_ptr)*new_capacity);
   for (unsigned i = 0; i < new_capacity; ++i) {
      new_arritem_ptr[i] = NULL;
   }

   for (unsigned i = 0; i < symtab_sg->capacity; ++i) {
      struct symtab_item *item;
      struct symtab_item *next_item;
      for (item = symtab_sg->arritem_ptr[i]; item; item = next_item) {
         next_item = item->next;
         unsigned new_index = hash(item->key, new_capacity);
         item->next = new_arritem_ptr[new_index];
         new_arritem_ptr[new_index] = item;
      }
   }

   struct symtab_item **old_arritem_ptr = symtab_sg->arritem_ptr;
   symtab_sg->arritem_ptr = new_arritem_ptr;
   symtab_sg->capacity = new_capacity;
   rtab_free(old_arritem_ptr);
}
