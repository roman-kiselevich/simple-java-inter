#include "../restab.h"
#include <stdlib.h>

int main() {
	resourse_table_init();

   int *num = rtab_malloc(10022323232323223323232323);
   

   num = rtab_malloc(2);
   delete_resource_table_and_free_all_resources();
}