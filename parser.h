/*
 * IFJ project
 * Team leader: 
 *         Kiselevich Roman   xkisel00@stud.fit.vutbr.czs
 * Team:
 *         But Andrii         xbutan00@stud.fit.vutbr.cz
 *         Inhliziian Bohdan  xinhli00@stud.fit.vutbr.cz
 *         Niahodkin Pavel    xniaho00@stud.fit.vutbr.cz
 *         Kharytonov Danylo  xkhary00@stud.fit.vutbr.cz
 * 
 * The module was created by:
 *        Kiselevich Roman
 * Description:
 * 		Module for work with parser
 */


#ifndef PARSER_H
#define PARSER_H

/**
 * launches parser
 */
void run_parsing();


/**
 * @return returns identifier of the current class that parser is processing 
 */
char *parser_get_curr_class_id();

/**
 * @return returns identifier of the current class that parser is processing
 */
char *parser_get_curr_func_id();

#endif /* PARSER_H */
