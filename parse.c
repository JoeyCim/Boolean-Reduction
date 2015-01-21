//Parses the user-entered expression into a tree

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "collection.h"

/*Numerical representations for each operand's precendence -- a higher value 
 * indicates a higher precedence. */
#define ADD_PREC 1
#define MULT_PREC 2
#define NOT_PREC 3
#define PAREN_PREC 4

#define MAX_LINE_LENGTH 150

int can_trim(char *text);
int eval(tree_node *node);
char* sub_str(char *text, int n);
link_node *add_node(char var_name);
void trim(char *text);
void parse_line(char *text, tree_node *node);
void print_table();
void set_values(int m);
void print_list_vals();
void remove_spaces(char *text);
void check_paren();

void add_min_node(link_node *node);
void print_min_list();
void factor();

tree_node *op_root = NULL;
link_node *var_head = NULL;
int var_count = 0;
int output[1 << MAX_VAR];

//Used to check for tautologies and contradictions
int sum = 0;
int prod = 1;

//Parse a line of input (representing the equation) into a tree representation
void parse_line(char *text, tree_node *node) {
    char min_prec_op = 0; //operator with lowest precedence
    int min_prec_loc = 0; //index of lowest-precedence operator
    int min_prec = INT_MAX;
    int i;
    int len = strlen(text);
    int prec = 0; //current "precedence" based on parentheses

    while(can_trim(text))
        trim(text);
    if (len == 1) {
        node->var = add_node(*text);
        node->op_val = 0; //Not an operator, so make this 0
        node->left_node = NULL;
        node->right_node = NULL;
        return;
    }
    else if (len == 0) {
        printf("Error: incorrect use of operator or missing operand\n");
        exit(1);
    }
    for (i = 0; i < len; i++) {
        
        //Establish "precedence" of operators
        switch(*(text + i)) {
            case '(':
                prec += PAREN_PREC;
                break;
            case ')':
                prec -= PAREN_PREC;
                break;
            case '+':
                if (prec + ADD_PREC < min_prec) {
                    min_prec = prec + ADD_PREC;
                    min_prec_op = *(text + i);
                    min_prec_loc = i;
                }
                break;
            case '*':
                if (prec + MULT_PREC < min_prec) {
                    min_prec = prec + MULT_PREC;
                    min_prec_op = *(text + i);
                    min_prec_loc = i;
                }
                break;
            case '\'':
                if (prec + NOT_PREC < min_prec) {
                    min_prec = prec + NOT_PREC;
                    min_prec_op = *(text + i);
                    min_prec_loc = i;
                }
                break;
        }
    }
    node->op_val = min_prec_op;
    node->var = NULL;
    node->left_node = malloc(sizeof(tree_node));
    node->right_node = malloc(sizeof(tree_node));
    char *left_str = sub_str(text, min_prec_loc);
    char *right_str = sub_str(text + min_prec_loc + 1, len - (min_prec_loc + 1));
    if (min_prec_op != '\'') {
        parse_line(left_str, node->left_node);
        parse_line(right_str, node->right_node);
    }
    else { //"NOT" operator only takes one operand to its left
        parse_line(left_str, node->left_node);
        node->right_node = NULL;
    }
}

/*Returns a non-zero value if the expression text has unnecessary 
  parentheses surrounding it (e.g., (a + b)) and 0 otherwise (e.g., 
  (a + b)*(c + d) */
int can_trim(char *text) {
    int balance = 0;
    int total = 0;
    int i;
    
    if (*text != '(') //Must start with open parenthesis
        return 0;
    for (i = 0; i < strlen(text) - 1; ++i) {
        if (*(text + i) == '(') {
            balance++;
            total++;
        } 
        else if (*(text + i) == ')') {
            balance--;
            total++;
            if (balance == 0) //Parentheses match up before end -- don't trim
                return 0;
        }
    }
    return balance;
}

/*Removes the outermost pair of unnecessary parentheses surrounding text:
 for example, (a + b) would become a + b.*/
void trim(char *text) {
    char *lastch = strrchr(text, ')');
    char *firstch = strchr(text, '(');
    char *trimmed = text;
    
    while (*text) {
        if (text != firstch && text != lastch) {
            *trimmed++ = *text++;
        }
        else {
            text++;
        }
    }
    *trimmed = '\0';
}

/*Add a variable to the linked list in alphabetical order and return a pointer 
  to it. If a variable with the same name has already been added to the linked
  list, simply return a pointer to it and don't add anything. */
link_node *add_node(char var_name) {
    link_node *new_node = malloc(sizeof(link_node));
    new_node->name = var_name;
    new_node->val = 0;
    
    if (var_head == NULL) {
        new_node->next = var_head;
        var_head = new_node;
        var_count++;
        return new_node;
    }
    else {
        printf("%d\n", var_count);
         if (var_count >= MAX_VAR) {
            printf("Sorry, no more than %d variables can be used\n", MAX_VAR);
            exit(1);
        }
         
        link_node *iter1 = var_head;
        link_node *iter2 = var_head;
        while (iter2) {
            if (var_name < iter2->name) { //Found appropriate place to add node
                new_node->next = iter2;
                if (iter2 != var_head)
                    iter1->next = new_node;
                else
                    var_head = new_node;
                var_count++;
                return new_node;
            }
            else if (var_name == iter2->name) //Node found with same name
                return iter2;
            else {
                iter1 = iter2;
                iter2 = iter2->next;
            }
        }
        
        //Add it to the end
        iter1->next = new_node;
        new_node->next = iter2;
        var_count++;
        return new_node;
    }
}

//Return a substring consisting of the first n characters in "text"
char *sub_str(char* text, int n) {
    char *sub = malloc((n+1)*sizeof(*text));
    int i;
    
    for (i = 0; *text && i < n; i++) {
        *(sub + i) = *(text + i);
    }
    *(sub + i) = '\0';
    return sub;
}

/*Set the value of the input bits for the m'th truth table value (i.e., so the 
  bits form the binary representation of m)*/
void set_values(int m) {
    int i;
    link_node *pnode = var_head;
    
    for(i = var_count; i >= 1; --i) {
        pnode->val = (m/(int)pow(2, i - 1)) % 2;
        pnode = pnode->next;
    }
}

//Print the values currently stored in the input variables
void print_list_vals() {
    link_node *p = var_head;
    
    while (p) {
        printf("%d  ", p->val);
        p = p->next;
    }
    printf("%n");
}

//Returns the function output based on the current input values
int eval(tree_node *node) {
    switch(node->op_val) {
        case '+':
           return eval(node->left_node) || eval(node->right_node);
           break;
        case '*':
            return eval(node->left_node) && eval(node->right_node);
            break;
        case '\'':
            return 1 - eval(node->left_node);
            break;
        case 0:
            return node->var->val;
    }
}

//Print the "truth table" for the equations
void print_table() {
    int i;
    link_node *node = var_head;
    
    while (node) {
        printf("%c  ", node->name);
        node = node->next;
    }
    printf("Output\n");
    for (i = 0; i < (int)pow(2, var_count); i++) {
        node = var_head;
        set_values(i);
        print_list_vals();
        int res = eval(op_root); //Truth value for current input values
        prod *= res;
        sum += res;
        printf("%d\n", res);
        output[i] = res;
        if (res != 0)
            add_min_node(var_head);
    }
}

//Removes spaces from the input
void remove_spaces(char *text) {
    char *trimmed_text = text;
    
    while (*text) {
        if (*text != ' ') {
            *trimmed_text++ = *text++;
        }
        else {
            text++;
        }
    }
    *trimmed_text = '\0';
}

//Checks whether there is a mismatch of parentheses
void check_paren(char *line) {
    int balance = 0; //Tracks "balance" of open/closed parentheses
   
    while (*line) {
        if (*line == '(')
            balance++;
        if (*line == ')') {
            balance--;
            if (balance < 0) {
                printf("Error: too many closed parentheses");
                exit(1);
            }
        }
        line++;
    }
    if (balance) {
        printf("Error: too many open parentheses");
        exit(1);
    }
}

main() {
    char text[MAX_LINE_LENGTH];
    
    printf("Enter an equation to be evaluated below. Use single quotes (\') to"
            "\nindicate NOT. Use + for OR and * for AND. Parentheses are"
            "\nallowed (ex. (a+b)*(a+c))\n\n");
    fgets(text, MAX_LINE_LENGTH, stdin);
    size_t len = strlen(text) - 1;
    text[len] = '\0'; //Replace new-line character with null terminator
    remove_spaces(text);
    check_paren(text); //Ensure parentheses pair up properly
    
    //Create an "operation tree" from the input
    op_root = malloc(sizeof(tree_node));
    parse_line(text, op_root);
   
    printf("\n");
    print_table();
    printf("\n\n");
    
    if (sum == 0) //Contradiction
        printf("Simplified expression: 0\n");
    else if (prod == 1) //Tautology
        printf("Simplified expression: 1\n");
    else {
        mark_reducable();
        printf("Simplified expression: \n");
        factor();
    }
}

