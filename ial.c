/**
 * symbol table 
 */

/**
 * types of elements which stored in symbol table
 */
enum type_item { t_item_func, t_item_var, t_item_class };

/**
 * types of variables in the language ifj16
 */
enum type_variable { t_var_void, t_var_int, t_var_float, t_var_string };

typedef struct function_information {
   enum ifj16_funcs ifj16_func;

   struct variable_information **arrargs;
   unsigned num_args;
   struct instruction *first_instruction;
   enum type_variable ret_type;

   unsigned num_defvars;
   struct variable_information **arrvar_ptrs;
} func_info_t;

typedef struct variable_information {
   enum type_variable t_var;
   bool init;
   union var_info_val *var_ptr; /* when var is local (ptr to stack) */
} var_info_t;

/**
 * The value of any variable is represented as a union
 */
union var_info_val {
   char *string;
   int integer;
   double flpoint;
};

static unsigned curr_hash_size_sg = 0;
static unsigned max_size_i_sg     = 17;
/**
 * good hash sizes for symbol table
 */
static unsigned hash_sizes_sg[]   = {
   769,     1543,    3079,       6151,       12289,      24593,
   49157,   98317,   96613,      393241,     786433,     1572869,
   3145739, 6291469, 12582917,   25165843,   50331653,   100663319
};

static struct symbol_table {
   unsigned count;                   /* item counter */
   unsigned capacity;
   struct symtab_item **arritem_ptr; /* array of ptrs to items */
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


/* sdbm algorithm */
static unsigned long hash(char *key, unsigned symtab_cap) {
   int c;
   unsigned long hash = 0;
   while ((c = *key++) != '\0')
      hash = c + (hash << 6) + (hash << 16) - hash;

   return hash % symtab_cap;
}

/**
 * ganerate key for identifier id
 * @param class_id id class to which the identifier id is referred
 * @param func_id if the identifier is a function
 * @param id the identifier for which we generate key
 * @return new key
 */
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

/**
 * Initialize symbol table
 */
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

/**
 * @param t_search_item type of element
 * @param key_search_item key of element
 * @return pointer to the structure which contains information about element
 */
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

/**
 * Put element into the table
 * @param t_new_item type of element
 * @param new_key key of element
 * @param item_info structure with information about element
 * @return pointer to the structure which contains information about element
 */
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
      /* init errno */
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

   /* init */
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

/**
 * end of symbol table realization
 */

/**
 * standart functions realization
 */

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