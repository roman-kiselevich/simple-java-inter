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
 *     Module for working with expression tree
 */

#ifndef EXPR_TREE_H
#define EXPR_TREE_H

#include "token.h"
#include "symtab.h"

typedef struct expression_tree {
   struct etree_node *root;
} etree_t;

typedef struct etree_node {
   token_t tok;
   struct etree_node *left;
   struct etree_node *right;
} etree_node_t;

/**
 * Initializes tree
 * @param A pointer to the node that will be root of the tree after initialization
 * @return A pointer to a tree that has been initialized
 */
etree_t *etree_init(etree_node_t *init_root);

/**
 * Initializes the tree node
 * @param a pointer to the token that will be stored in the node
 * @return pointer to the initialized node
 */
etree_node_t *etree_create_node(token_t *tok_to_node);

/**
 * Connects the left and right subtree with new node
 * @param tok_to_node token that will be stored in the new node
 * @param left a pointer to the root of the left subtree
 * @param right pointer to the root of the right subtree
 * @return pointer to a new node
 */
etree_node_t *etree_connect_nodes(token_t *tok_to_node,
                                  etree_node_t *left, 
                                  etree_node_t *right);

/**
 * Returns a type of result of the expression by the rule of typecasting
 * @param a pointer to a tree
 * @return type of result
 */
enum type_variable etree_get_result_type(etree_t *etree);

/**
 * Check if all the variables (which are part of the expression) 
 * are in the symbol table
 * @param a pointer to the root of the tree
 */
void etree_var_control(etree_node_t *root);

/**
 * Returns the result of the expression tree and type through
 *  the pointer * tvar_p
 * @param Root pointer to the root of the tree
 * @param Tvar_p pointer to the type of result

 * @return result of expression
 */
union var_info_val etree_get_expr_result(etree_node_t *root, 
                                         enum type_variable *tvar_p);

/**
 * Returns a token value that is a literal or variable
 * @param Tok pointer to the token
 * @param Tvar_p pointer to the type of result
 * @return the value of the literal or variable represented by token
 */
union var_info_val etree_get_var_tok(token_t *tok, enum type_variable *tvar_p);

#endif /* EXPR_TREE_H */
