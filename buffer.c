/*
 * IFJ Project Java SE 8 Interpret
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
 * Description:
 *   Buffer for temporary storage and processing of the incoming character stream        
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "buffer.h"
#include "soui.h"
#include "qoui.h"
#include "itrerror.h"
#include "restab.h"
#include "boolean.h"

enum { 
       arrbuff_realloc_c = 4,
       buffer_size_c     = 4096, 
       arrch_size_c      = 1024,
       eobuff_c          = -1,  
       eostream_c        = -2, 
};

struct buffer {
   FILE *stream;
   qoui_t *q_load_buf_i; 
   soui_t *s_store_buf_i;

   char **arrbuff;
   unsigned arrbuff_cap;

   unsigned curr_buff_i;
   bool set_lex_flag;
   char *lexeme_begin;
   char *forward;
};

static void load_in_buff(buffer_t *buff, unsigned free_buffer_index);

static unsigned init_last_nbuff(char **arrb, unsigned arrb_cap, unsigned n);

static unsigned arrbuff_expand(buffer_t *buff);

static unsigned get_next_inbuff(buffer_t *buff);

static void append_lex(char **lex, char *begin_lex_part, 
                       size_t curr_size, size_t add_size);

buffer_t *buf_init(FILE *new_stream) {
   buffer_t *new_buffer = rtab_malloc(sizeof(*new_buffer));

   size_t arrbuff_size = sizeof(*new_buffer->arrbuff)*arrbuff_realloc_c;
   new_buffer->arrbuff = rtab_malloc(arrbuff_size);

   new_buffer->arrbuff_cap = arrbuff_realloc_c;
   init_last_nbuff(new_buffer->arrbuff,
                   new_buffer->arrbuff_cap, arrbuff_realloc_c);

   new_buffer->stream = new_stream;
   new_buffer->lexeme_begin = new_buffer->arrbuff[0];
   new_buffer->forward = new_buffer->lexeme_begin;
   new_buffer->q_load_buf_i = qoui_init();
   new_buffer->s_store_buf_i = soui_init();
   new_buffer->set_lex_flag = false;
   new_buffer->curr_buff_i = 0;
   qoui_put(new_buffer->q_load_buf_i, 0);
   load_in_buff(new_buffer, 0);
   return new_buffer;
}

int buf_getch(buffer_t *buff) {
   unsigned next_buff_i;
   switch (*buff->forward) {
   case eobuff_c:
      next_buff_i = get_next_inbuff(buff);
      buff->forward = &buff->arrbuff[next_buff_i][0];
      if (*buff->forward == eostream_c)
         return EOF;

      if (buff->set_lex_flag == false) {
         buff->lexeme_begin = buff->forward;
         qoui_get(buff->q_load_buf_i);
         qoui_put(buff->q_load_buf_i, buff->curr_buff_i);
      }
      else {
         qoui_put(buff->q_load_buf_i, next_buff_i);
      }
      break;
   case eostream_c:
      return EOF;
      break;
   }

   if (buff->set_lex_flag == false) {
      ++buff->lexeme_begin;
   }
   
   return *buff->forward++;
}

int buf_ungetch(buffer_t *buff) {
   if (buff->forward == &buff->arrbuff[buff->curr_buff_i][0]) {
      soui_push(buff->s_store_buf_i, buff->curr_buff_i);
      unsigned prev_buff_i = qoui_get_back(buff->q_load_buf_i); /* 0? */
      buff->forward = &buff->arrbuff[prev_buff_i][arrch_size_c - 1];
      buff->lexeme_begin = buff->forward;
      return *buff->forward;
   }

   if (buff->set_lex_flag == false)
      --buff->lexeme_begin;

   return *--buff->forward;
}

char *buf_getlex(buffer_t *buff) {
   size_t new_lex_size;
   char *new_lex = NULL;
   unsigned lexb_buff_i;
   if (1 == qoui_get_count(buff->q_load_buf_i)) {
      new_lex_size = buff->forward - buff->lexeme_begin;
      append_lex(&new_lex, buff->lexeme_begin, new_lex_size, 0);

      buff->set_lex_flag = false;
      buff->lexeme_begin = buff->forward;
   }
   else if (2 == qoui_get_count(buff->q_load_buf_i)) {
      lexb_buff_i = qoui_get(buff->q_load_buf_i);
      new_lex_size =                                                       \
         &buff->arrbuff[lexb_buff_i][arrch_size_c - 1] - buff->lexeme_begin;
      append_lex(&new_lex, buff->lexeme_begin, new_lex_size, 0);
      unsigned add_size = buff->forward - &buff->arrbuff[buff->curr_buff_i][0];
      if (add_size != 0) {
         append_lex(&new_lex, &buff->arrbuff[buff->curr_buff_i][0], \
                    new_lex_size, add_size);
      }

      buff->set_lex_flag = false;
      buff->lexeme_begin = buff->forward;
   }
   else {
      lexb_buff_i = qoui_get(buff->q_load_buf_i);
      new_lex_size =    \
         &buff->arrbuff[lexb_buff_i][arrch_size_c - 1] - buff->lexeme_begin;
      append_lex(&new_lex, buff->lexeme_begin, new_lex_size, 0);

      unsigned add_size = arrch_size_c - 1;
      while (1 != qoui_get_count(buff->q_load_buf_i)) {
         lexb_buff_i = qoui_get(buff->q_load_buf_i);
         append_lex(&new_lex, &buff->arrbuff[lexb_buff_i][0], \
                    new_lex_size, add_size);
         new_lex_size += add_size;
      }

      add_size = buff->forward - &buff->arrbuff[buff->curr_buff_i][0];
      if (add_size != 0) {
         append_lex(&new_lex, &buff->arrbuff[buff->curr_buff_i][0], \
                    new_lex_size, add_size);
      }

      buff->set_lex_flag = false;
      buff->lexeme_begin = buff->forward;
   }
   return new_lex;
}

static unsigned init_last_nbuff(char **arrb, unsigned arrb_cap, unsigned n) {
   unsigned arr_alloc_index = arrb_cap - 1;
   for ( ; arr_alloc_index % n; --arr_alloc_index)
   {}
   
   arrb += arr_alloc_index;
   arrb[0] = rtab_malloc(buffer_size_c);

   for (int i = 1; i < arrbuff_realloc_c; ++i)
      arrb[i] = arrb[0] + i*arrch_size_c;

   for (int i = 0; i < arrbuff_realloc_c; ++i) {
      arrb[i][arrch_size_c - 1] = eobuff_c;
   }
   return arr_alloc_index;
}

static void load_in_buff(buffer_t *buff, unsigned free_buffer_index) {
   size_t read_size = fread(buff->arrbuff[free_buffer_index], 1,
                         arrch_size_c - 1, buff->stream);

   if (feof(buff->stream)) {
      buff->arrbuff[free_buffer_index][read_size] = eostream_c;
      return;
   }

   if (ferror(buff->stream)) {
      delete_resource_table_and_free_all_resources();
      itrerrno_g = iternal_compile_err;
      fatal_error("occurred some error connected with input stream\n");
      return;
   }
   return;
}

static unsigned get_next_inbuff(buffer_t *buff) {
   unsigned next_buff_i;
   if (!soui_empty(buff->s_store_buf_i)) {
      next_buff_i = soui_pop(buff->s_store_buf_i);
      buff->curr_buff_i = next_buff_i;
      qoui_put(buff->q_load_buf_i, next_buff_i);
      return next_buff_i;
   }

   unsigned *found_arrch_i = NULL;
   for (unsigned i = 0; i < buff->arrbuff_cap; ++i) {
      found_arrch_i = qoui_is_elem_in(buff->q_load_buf_i, i);
      if (NULL == found_arrch_i) {
         buff->curr_buff_i = i;
         load_in_buff(buff, i);
         return i;
      }
   }

   /* rtab_realloc arrbuff */
   next_buff_i = arrbuff_expand(buff);
   buff->curr_buff_i = next_buff_i;
   load_in_buff(buff, next_buff_i);
   return next_buff_i;
}

static unsigned arrbuff_expand(buffer_t *buff) {
   size_t curr_size = buff->arrbuff_cap*sizeof(char *);
   size_t add_size = arrbuff_realloc_c*sizeof(char *);
   buff->arrbuff = rtab_realloc(buff->arrbuff, curr_size + add_size);
   
   buff->arrbuff_cap += arrbuff_realloc_c;
   unsigned buff_index = 
      init_last_nbuff(buff->arrbuff, buff->arrbuff_cap, arrbuff_realloc_c);
   return buff_index;
}

void buf_setlex(buffer_t *buff) {
   buff->set_lex_flag = true;
}

static void append_lex(char **lex, 
                       char *begin_lex_part, 
                       size_t curr_size,
                       size_t add_size)
{
   *lex = rtab_realloc(*lex, curr_size + add_size + 1);
   if (add_size == 0) {
      strncpy(*lex, begin_lex_part, curr_size);
   }
   else {
      strncpy(*lex + curr_size, begin_lex_part, add_size);  
   } 
   (*lex)[curr_size + add_size] = '\0';
}