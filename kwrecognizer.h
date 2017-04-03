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
 * 		Module contains instruments for recognizing keywords
 */

#ifndef KEYWORD_RECOGNIZER
#define KEYWORD_RECOGNIZER

#include "boolean.h"
#include "token.h"

/**
 * Recognize if token a keyword
 * @param pointer to the token
 * @return true if tok is a keyword
 *         false if opposite result
 */
bool keyword(token_t *tok);

#endif /* KEYWORD_RECOGNIZER */
