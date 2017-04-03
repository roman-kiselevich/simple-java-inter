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
 *        Kiselevich Roman
 * Description:
 *   Buffer for temporary storage and processing of the incoming 
 *   character stream
 * 
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

typedef struct buffer buffer_t;

/**
 * Initialize buffer and associates it with the stream
 * @param new_stream stream for association with buffer
 * @return pointer to a new initialized buffers
 */
buffer_t *buf_init(FILE *new_stream);

/**
 * Reads the next character from the stream, similar to a standart gatchar()
 * @param buff pointer to the buffer from which we read character
 * @return the next character
 */
int buf_getch(buffer_t *buff);

/**
 * Similar to standard ungetc() just do not specify which character and can
 * be safty used 3 times in a row. If the call is greater than 3 times the behavior
 * is undefined
 * @param buff pointer to the buffer
 * @return the previous character
 */
int buf_ungetch(buffer_t *buff);

/**
 * Inform buffer that immediately after the privious charater begins a sequence
 * of characters that we want to return by function buf_getlex() 
 * @param pointer to the buffer 
 */
void buf_setlex(buffer_t *buff);

/**
 * Buffer returns dynamic allocated sequence of characters (lexeme)
 * the begining of which we specify by function buf_setlex()
 * @param a pointer to the buffer
 * @return dynamically selected lexeme
 */
char *buf_getlex(buffer_t *buff);

#endif /* BUFFER_H */
