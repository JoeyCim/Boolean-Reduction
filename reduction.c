//Simplifies expression

#include <stdio.h>

#include "collection.h";

void add_min_node(link_node *node);
void add_min_node_dir(min_node *new);
void print_min_list();
min_node *step(min_node *node);
int mark_reducable();
void print_node(min_node *node);
int arr_eq(int arr1[], int arr2[]);
void remove_duplicates();

extern tree_node *op_root;
extern link_node *var_head;
extern int var_count;
extern int output[1 << MAX_VAR];

min_node *min_head;
int min_count; //The number of minterms

/*Create a linked "min_node" based on the current values stored in the variable
  linked list, then add that node to the "min_node" linked list */
void add_min_node(link_node *node) {
    int i;
    min_node *new = malloc(sizeof(min_node));
    
    min_count++;
    new->one_count = 0;
    new->diff_by_one = 0;
    for (i = 0; i < var_count; ++i) {
        new->var_vals[i] = node->val;
        if (new->var_vals[i] == 1)
            new->one_count++;
        node = node->next;
    }
    add_min_node_dir(new);
}

// Directly add a new minterm node to the list
void add_min_node_dir(min_node *new) {
    min_node *it = min_head;
    
    if (!min_head || new->one_count <= min_head->one_count) {
        new->next = min_head;
        min_head = new;
        return;
    }
    while(it->next) {
        if (new->one_count <= it->next->one_count) {
            min_node *temp = it->next;
            it->next = new;
            new->next = temp;
            return;
        }
        it = it->next;
    }
    it->next = new;
    new->next = NULL;
}

void arr_copy(int arrTo[], int arrFrom[]) {
    int i;
    
    for (i = 0; i < var_count; ++i) {
        arrTo[i] = arrFrom[i];
    }
}

//Returns 1 if two arrays have equal elements and 0 otherwise
int arr_eq(int arr1[], int arr2[]) {
    int i;
    
    for (i = 0; i < var_count; i++) {
        if (arr1[i] != arr2[i])
            return 0;
    }
    return 1;
}

//Removes a minterm from the expression
void remove_min_node(min_node *node) {
    min_node *it = min_head;
    
    if (it == node) { 
        min_head = it->next;
        free(node);
        return;
    }
    while (it->next) {
        if (node == it->next) {
            min_node *temp = it->next;
            it->next = temp->next;
            free(temp);
            return;
        }
        it = it->next;
    }
}

//Prints the minterms currently stored in the linked list
void print_min_list() {
    min_node *m = min_head;
    int i;
    
    while (m) {
        for (i = 0; i < var_count; ++i) {
            printf("%d ", m->var_vals[i]);
        }
        printf("%d  %d\n", m->one_count, m->diff_by_one);
        m = m->next;
    }
}

//Removes (and frees the memory of) duplicate terms in the simplified expression
void remove_duplicates() {
    min_node *m = min_head;
    while (m) {
        min_node *n = m;
        while (n->next) {
            if (arr_eq(m->var_vals, n->next->var_vals)) { //Delete from list
                min_node *temp = n->next;
                n->next = temp->next;
                free(temp);
            }
            else {
                n = n->next;
            }
        }
        m = m->next;
    }
}

void print_node(min_node *node) {
    int i;
    
    for (i = 0; i < var_count; ++i) {
        printf("%d ", node->var_vals[i]);
    }
    printf("\n");
}

//Checks whether two minterms differ by only one bit
int diff_by_one(min_node *m, min_node *n) {
    int dif_loc = -1;
    int i;
    
    for (i = 0; i < var_count; ++i) {
        if (m->var_vals[i] != n->var_vals[i]) {
            if (dif_loc != -1)
                return -1;
            else
                dif_loc = i;
        }
    }
    return dif_loc;
}

/* Returns the first minterm that has exactly one more one in it than the
   parameter "node", or NULL if no such parameter exists */
min_node *step(min_node *node) {
    int cur_count = node->one_count;
    
    while (node && (node->one_count == cur_count)) {
        node = node->next;
    }
    return node;
}

/*Remove all terms that differ from other terms by one bit (pairs of these have
  already produced a more simplified "-"-term) */
int filter() {
    min_node *it = min_head;
    int count = 0;
    
    while (it) {
        if (it->diff_by_one == 1) {
            min_node *temp = it;
            it = it->next;
            remove_min_node(temp);
            count++;
        }
        else {
            it = it->next;
        }
    }
    return count;
}

//Print out the simplified expression
void print_equation() {
    int i;
    link_node *l = var_head;
    min_node *m = min_head;
        
    while (m) {
        if (m != min_head)
            printf(" + ");
       
        //Print every non-"-" entry in the simplified minterm
        for (i = 0; i < var_count; ++i) { 
            if (m->var_vals[i] == 1) {
                printf("%c", l->name);
            }
            else if (m->var_vals[i] == 0) {
                printf("%c\'", l->name);
            }
            l = l->next;
        }
        l = var_head;
        m = m->next;
    }
    printf("\n");
}

/*Repeatedly filter the expression (and remove duplicate minterms along the
  way) to factor the expression into its prime implicants, then remove 
  redundant implicants.  */
void factor() {
    while (filter()) {
       remove_duplicates();
       mark_reducable();
    }
    simplify();
    print_equation();
}

//Compute the truth values for the expression, excluding the minterm "skip"
int eval_minterms(int m, min_node *skip) {
    int i;
    int cur_val, prod_val;
    min_node *it = min_head;
    
    while (it) {
        if (it == skip) {
            it = it->next;
            continue;
        }
        prod_val = 1;
        for (i = 0; i < var_count; i++) {
            
            /*Assign cur_val to a binary number based on the minterm number
             and its significance within the expression */
            cur_val = (m/(int)pow(2, var_count - i - 1)) % 2;
            if (it->var_vals[i] == 1) {
                prod_val *= cur_val;
            }
            else if (it->var_vals[i] == 0) { //Variable is negated (e.g., a')
                prod_val *= (1 - cur_val);
            }
        }
        
        /* Since the expression is already in sum-of-product form, only need
        one term to be one for the output to be 1 */
        if (prod_val == 1)
            return 1;
        it = it->next;
    }
    return 0;
}

/*Determines whether a minterm is needed by checking whether the truth-table
  output values stay the same when that minterm is removed. */
int is_redundant(min_node *min) {
    int i;
    
    for (i = 0; i < (int)pow(2, var_count); ++i) {
        if (eval_minterms(i, min) != output[i]) {
            return 0;
        }
    }
    return 1;
}

//Removes "redundant" prime implicants from an expression
void simplify() {
    int i;
    min_node *min = min_head;
    min_node *temp;
    
    while (min) {
        if (is_redundant(min)) {
            temp = min;
            min = min->next;
            remove_min_node(temp);
        }
        else
            min = min->next;
    }
}

/*For all minterms that differ from another minterm by one bit, assign
  "diff_by_one" to 1, then add a new entry to the list that shares the common
  bits between the two and has a '-' in the bit slot where they differ 
 (e.g., '0101' and '1101' would add '-101' to the list) */
int mark_reducable() {
    min_node *node = min_head;
    min_node *first_bigger = min_head;
    
    while (first_bigger = step(first_bigger)) { 
        min_node *end = first_bigger;
        while (node != first_bigger) {
            while (end && end->one_count == first_bigger->one_count) {
                int diff_loc;
                if ((diff_loc = diff_by_one(node, end)) != -1) {
                    node->diff_by_one = 1;
                    end->diff_by_one = 1;
                    node->diff_loc = diff_loc;
                    
                    //Add new "-"-entry to the list.
                    min_node *dash_entry = malloc(sizeof(min_node));
                    arr_copy(dash_entry->var_vals, node->var_vals);
                    dash_entry->var_vals[diff_loc] = '-';
                    dash_entry->one_count = node->one_count - (node->var_vals[diff_loc] == 1? 1: 0);
                    dash_entry->diff_by_one = 0;
                    add_min_node_dir(dash_entry);
                }
                end = end->next;
            }
            end = first_bigger;
            node = node->next;
        }
    }
}