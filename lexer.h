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
 * 		Module for work with lexer
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdio.h>

/**
 * initialize lexer
 * @param pointer to the file with source code
 */
void lexer_init(FILE *new_source_file);

/**
 * @return returns next token from the stream
 */
token_t *lex_token_get_next();

/**
 * Returns token back to the stream. Can be used after
 * call of function lex_token_get_next() only once
 * @return previous token
 */
token_t *lex_token_unget();

/**
 * @return returns current token, that was read
 * by function lex_token_get_next()
 */
token_t *lex_token_get_current();

#endif /* LEXER_H */
