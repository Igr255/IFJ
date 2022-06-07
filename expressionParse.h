/**
 * Igor Hanus <xhanus19>
 */

#ifndef _EXPRESSIONPARSE_H
#define _EXPRESSIONPARSE_H

#include "tokenizer.h"
#include "stdbool.h"
#include "synCheck.h"
#define TAB_SIZE 10

typedef enum operators{
    P,  // =
    L,  // < SHIFT
    R,  // > REDAKSN
    X,  // err
    N,  // skip
} Table_operators;

typedef enum rules{
    E_ADD_E,    // E -> E + E
    E_SUB_E,    // E -> E - E
    E_MUL_E,    // E -> E * E
    E_DIV_E,    // E -> E / E
    E_IDIV_E,   // E -> E // E
    LEN_E,      // E -> #E
    L_E_R,      // E -> (E)
    E_APP_E,    // E -> E..E
    E_i,        // E -> i
    E_nil,      // E -> nil
    E_err,      // big bad
} Table_rules;

// indexes in the table
typedef enum tableOperations{
    t_plus_minus, //0
    t_div_mul,    //1
    t_compare,    //2
    t_get_len,    //3
    t_concat,     //4
    t_left_br,    //5
    t_right_br,   //6
    t_i,          //7
    t_nil,        //8
    t_end,        //9
    t_error       //10
} Table_operations;

typedef enum operations{
    plus,
    minus,
    fdiv,
    idiv,
    mul,
    int_mul, // ?? was too drunk dont remember
    less_than,
    less_eq_than,
    more_than,
    more_eq_than,
    eq,
    not_eq,
    len,
    concat,


    left_br,
    right_br,
    i,
    nil,
    end,
    integer,
    number,
    string,
    E
} Operations;

token_t * readExpression(f_foo_str *function, f_foo_var** handle, int *nestLevel, scope_switch usedScope, bst_node_t **globalTree, bool *isFoo, f_foo_str** calledFunction);
int getOperator(int bufferIndex, int stackIndex);

#endif //_EXPRESSIONPARSE_H
