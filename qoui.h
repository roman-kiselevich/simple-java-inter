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
 * Description:
 * 		Module for work with queue unsigned integers
 */


#ifndef QUEUE_OF_UINTS_H
#define QUEUE_OF_UINTS_H

#include "boolean.h"

typedef struct queue_of_uint qoui_t;

/**
 * Initialize queue
 * @return pointer to queue
 */
qoui_t *qoui_init();

/**
 * @param qoui pointer to queue to which we put value
 * @param value value which we put in queue 
 */
void qoui_put(qoui_t *qoui, unsigned value);

/**
 * @param pointer to queue from which we get an element
 * @return element
 */
unsigned qoui_get(qoui_t *qoui);

/**
 * @param pointer to queue from which we get an element
 * @return returns element which is located in the end of the queue
 */
unsigned qoui_get_back(qoui_t *qoui);

/**
 * @param pointer to queue
 * @return the number of the elements in queue
 */
unsigned qoui_get_count(qoui_t *qoui);

/**
 * @param qoui pointer to queue
 * @param elem element
 * @return if element elem is located in queue return pointer to it
 *            in the opposite case return NULL
 */
unsigned *qoui_is_elem_in(qoui_t *qoui, unsigned elem);

#endif /* QUEUE_OF_UINTS_H */
