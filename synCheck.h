/**
 * Igor Hanus <xhanus19>
 */

#ifndef UNTITLED_SYNCHECK_H
#define UNTITLED_SYNCHECK_H
#include "tokenizer.h"
#include "symtable.h"
#include "codegen-interface.h"

#define MAX_ARGS 100
#define ALLOC_CAP 10

typedef struct op_scope_str f_op_scope_str;

typedef enum access_modifier {
    local,
    global
} access_modifier;

typedef enum scope_switch {
    function_case,
    if_case,
    else_case,
    while_case,
    global_case
} scope_switch;

typedef enum arg_type {
    t_integer,
    t_number,
    t_string,
    t_bool,
    t_null
} f_type;

typedef struct foo_arg{
    f_type type;
    char *id;
} f_foo_arg;

typedef struct foo_var{
    bool isDefined;
    bool isHandle;
    bool isImmediate; // t pre const else f
    f_type type;
    char *id;
    char *originalId;

    long int intVal;
    double numVal;
    char * stringVal;
    var_desc * VarDesc;
} f_foo_var;

struct op_scope_str {
    int nestLevel;
    int count;

    char *id;
    bst_node_t  *localScopeVars;
    int varCount;

    bool isIf;
    bool isElse;
    bool isWhile;
};

typedef struct foo_str{
    bool isDefined; // global declaration / local definition
    char *id;       // name
    int argsCount;  // n.o. args
    int varCount;   // n.o. vars inside the scope of the foo
    int opCount;
    int currentOp;
    int retArgsCount; // n.o. ret values
    bst_node_t *localVars;

    int ifCount;
    int elseCount;
    int whileCount;

    f_op_scope_str **opArray;
    f_foo_var **args;
    f_type *ret_args;
} f_foo_str;

token_t *callFunction(token_t *token, bst_node_t **globalTree, access_modifier fooCallSrc, scope_switch usedScope, int *nestLevel, f_foo_str *function, f_foo_str *calledFunction, var_desc ***retVarArrayHandle);
f_foo_var *getExistingVariable(f_foo_str *currentFunction, token_t *token, int *nestLevel, scope_switch usedScope, bool useDefined);
token_t * stListRead(f_foo_str *currentFunction, token_t *token, int *nestLevel, scope_switch usedScope, bst_node_t **globalTree);
void startReading();

#endif //UNTITLED_SYNCHECK_H
