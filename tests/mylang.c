#include <stdio.h>
#include <ctype.h>

enum {
   lex_error_const = 1
};

enum token_type {
   id, number, keyword, prog_begin, prog_end,
   block_begin, block_end, open_brack, 
   close_brack, equalop, semicolon
};

union token_attribute {
   int number;
   void *ptr;
};

struct token {
   enum token_type type;
   union token_attribute attr;
};

int get_token(struct token *token) {
   int ch;
   enum states { 
      START, COMMENT_BEGIN, COMMENT, COMMENT_END,
      ID, NUMBER, KEYWORD, KEYWORD_T
   } state = START;

   while (EOF != (ch = getchar())) {
      switch (state) {
      case START:
         if (isspace(ch)) state = START;
         else if (ch == '/') state = COMMENT_BEGIN;
         else if (ch == 'i') state = KEYWORD;
         else if (isalpha(ch)) state = ID;
         else if (isdigit(ch)) state = NUMBER;
         else if (ch == '<') {
            token->type = prog_begin;
            return 0;
         }
         else if (ch == '>') {
            token->type = prog_end;
            return 0;
         }
         else if (ch == '{') {
            token->type = block_begin;
            return 0;
         }
         else if (ch == '}') {
            token->type = block_end;
            return 0;
         }
         else if (ch == '(') {
            token->type = open_brack;
            return 0;
         }
         else if (ch == ')') {
            token->type = close_brack;
            return 0;
         }
         else if (ch == '=') {
            token->type = equalop;
            return 0;
         }
         else if (ch == ';') {
            token->type = semicolon;
            return 0;
         }
         else return lex_error_const;
         break;
      case KEYWORD:
         if (ch == 'n') state = KEYWORD_T;
         else if (isalpha(ch)) state = ID;
         else {
            ungetc(ch, stdin);
            state = START;
         }
         break;
      case KEYWORD_T:
         if (ch == 't') {
            token->type = keyword;
            return 0;
         }
         else if (isalpha(ch)) state = ID;
         else {
            ungetc(ch, stdin);
            state = START;
         }
         break;
      case COMMENT_BEGIN:
         if (ch == '*') state = COMMENT;
         else return lex_error_const;
         break;
      case COMMENT:
         if (ch == '*') {
            state = COMMENT_END;
            break;
         }

         if (EOF == (ch = getchar())) return lex_error_const;
         else if (EOF != ch) ungetc(ch, stdin);
         break;
      case COMMENT_END:
         if (ch == '/') state = START;
         else state = COMMENT;
         break;
      case ID:
         if (isalpha(ch) || isdigit(ch)) state = ID;
         else {
            ungetc(ch, stdin);
            token->type = id;
            token->attr.ptr = NULL; /* TODO */
            return 0;
         }
         break;
      case NUMBER:
         if (isdigit(ch)) state = NUMBER;
         else {
            ungetc(ch, stdin);
            token->type = number;
            token->attr.number = 9; /* TODO */
            return 0;
         }
      }
   }
   return EOF;
}

void print_token(struct token *token) {
   switch (token->type) {
   case id:
      printf("[id, some_ptr] ");
      break;
   case number:
      printf("[number, %d] ", token->attr.number);
      break;
   case prog_begin:
      printf("['<', -] ");
      break;
   case prog_end:
      printf("['>', -] ");
      break;
   case block_begin:
      printf("['{', -] ");
      break;
   case block_end:
      printf("['}', -] ");
      break;
   case open_brack:
      printf("['(', -] ");
      break;
   case close_brack:
      printf("[')' -] ");
      break;
   case equalop:
      printf("['=', -] ");
      break;
   case semicolon:
      printf("[';', -] ");
      break;
   case keyword:
      printf("['keyword', -] ");
   }
}

int main() {
   struct token next_token;
   int status;
   unsigned token_counter = 0;
   while (EOF != (status = get_token(&next_token))) {
      ++token_counter;
      if (status == lex_error_const)
         goto lexical_error;

      if (token_counter % 5 == 0)
         putchar('\n');

      print_token(&next_token);
   }

   return 0;
lexical_error:
   fprintf(stderr, "lexical error\n");
   return status;
}