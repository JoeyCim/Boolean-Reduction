#ifndef COLLECTION_H
#define	COLLECTION_H

#define MAX_VAR 8


/* Node for linked list of variables: each node represents a unique variable
   that appears in the equation and its associated value (1 or 0).*/
typedef struct lnode {
    char name;
    int val;
    struct lnode *next;
} link_node;

/* Node for tree representation of the inputted equation: if the node 
   represents a variable, then "var" points to the appropriate node in the
   above linked list (since the same variable may appear multiple times in an
   equation, this allows its value to be changed more easily) and op_val is set
   to 0. If it represents an operator, then var points to NULL and op_val
   is set to the char representation of the operator ('+', '*', etc.).*/
typedef struct tnode {
    link_node *var;
    char op_val;
    struct tnode *left_node;
    struct tnode *right_node;
} tree_node;

/* Node for linked list that stores the minterms (the values of the input
   variables that make the output true) of the expression */
typedef struct mnode {
    int var_vals[MAX_VAR];
    int one_count;
    int diff_by_one;
    int diff_loc;
    struct mnode *next;
} min_node;
#endif


