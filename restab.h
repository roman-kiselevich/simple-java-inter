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
 * Descritption:
 * 		Module for work with table that stores dynamically allocated resources 
 */


#ifndef RESTAB_H
#define RESTAB_H

#include <stdio.h>

/**
 * Initialize resource table
 */
void resourse_table_init();

/**
 * is similar to the standard function malloc()
 * @param size of allocated memory
 * @return pointer to the memory that we allocate
 */
void *rtab_malloc(size_t size);

/**
 * is similar to the standard function realloc()
 * @param ptr pointer to resource in memory
 * @param size new size of resource
 * @return pointer to the resource
 */
void *rtab_realloc(void *ptr, size_t size);

/**
 * is similar to the standart function free()
 * @param pointer to the resource which we want to free
 */
void rtab_free(void *ptr);

/**
 * delete all resources which was alocated using functions
 * rtab_malloc(), rtab_realloc()
 */
void delete_resource_table_and_free_all_resources();

#endif /* RESTAB_H */
