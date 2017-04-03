#include <stdio.h>
#include <ctype.h>
#include "../buffer.h"
#include "../restab.h"

int main(int argc, char *argv[]) {
   resourse_table_init();
   FILE *source = fopen(argv[1], "r");

   buffer_t *my_buf = buf_init(source);
   int ch;
   enum { start, string, number, com_begin, com, com_end } state = start;
   char *str;
   while (EOF != (ch = buf_getch(my_buf))) {
      switch (state) {
      case start:
         if (ch == '\"') {
            state = string;
            buf_setlex(my_buf);
         }

         if (ch == '/') {
            state = com_begin;
         }

         if (isdigit(ch)) {
            buf_ungetch(my_buf);
            buf_setlex(my_buf);
            state = number;
         }
         break;
      case string:
         if (ch == '\"') {
            buf_ungetch(my_buf);
            str = buf_getlex(my_buf);
            printf("string: %s\n", str);
            state = start;
            buf_getch(my_buf);
         }
         break;
      case number:
         if (!isdigit(ch)) {
            buf_ungetch(my_buf);
            str = buf_getlex(my_buf);
            printf("number: %s\n", str);
            state = start;
         }
         break;
      case com_begin:
         if (ch != '*')
            printf("comment error\n");
         else
            state = com;
         break;
      case com:
         if (ch == '*')
            state = com_end;
         break;
      case com_end:
         if (ch != '/')
            state = com;
         else
            state = start;
         break;
      }
   }

   fclose(source);
   delete_resource_table_and_free_all_resources();
   return 0;
}