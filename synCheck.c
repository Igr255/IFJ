/**
 * Igor Hanus <xhanus19>
 */

#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "stdlib.h"
#include "expressionParse.h"
#include "tokenizer.h"
#include "symtable.h"
#include "semantic.h"
#include "codegen-interface.h"
#include "synCheck.h"

// global variables used for realloc purposes
int operationRealloc = 10;
int tmpArgRealloc = 10;
int argRealloc = 10;
int globalArgRealloc = 10;
int retArgRealloc = 10;

void *reallocArgMemory(int *globalReallocVar, f_foo_var** varArray) {
    *globalReallocVar = (*globalReallocVar)+10;
    void *temp = realloc(varArray, sizeof(f_foo_var*) * (*globalReallocVar));
    if (temp != NULL) {
        return temp;
    }
    else {
        exit(2);
    }
}

void *reallocOpScopeMemory(int *globalReallocVar, f_op_scope_str** opArray) {
    *globalReallocVar = (*globalReallocVar)+10;
    void *temp = realloc(opArray, sizeof(f_op_scope_str*) * (*globalReallocVar));
    if (temp != NULL) {
        return temp;
    }
    else {
        exit(2);
    }
}

f_type getType(token_t *token)
{
    if (strcmp(token->strValue, "integer")==0)
        return t_integer;
    else if (strcmp(token->strValue, "number")==0)
        return t_number;
    else if (strcmp(token->strValue, "string")==0)
        return t_string;
    else if (strcmp(token->strValue, "nil")==0)
        return t_null;
    else
        exit(2);
}

f_foo_var *allocateNewVariable (bool isDefined, bool isHandle, bool isImmediate, f_type type, char *id, char *originalId, long int intval, double numVal, char* stringVal){
    f_foo_var * newVar = malloc(sizeof(f_foo_var));
    newVar->id = id;
    newVar->originalId = originalId;
    newVar->stringVal = stringVal;
    newVar->isHandle = isHandle;
    newVar->isImmediate = isImmediate;
    newVar->isDefined = isDefined;
    newVar->type = type;
    newVar->intVal = intval;
    newVar->numVal = numVal;
    newVar->stringVal = stringVal;

    return newVar;
}

// freeing builtin functiosn after analysis passed successfully
void disposeBuiltinFooArgs(bst_node_t **globalTree) {
    char* functions[8] = {"reads", "readi", "readn", "tointeger", "substr", "ord", "chr", "write"};

    bst_node_t  *res = NULL;
    for (int i = 0; i < 8; i++) {
        bst_search(*globalTree, functions[i], &res);

        if (res == NULL) {
            exit(2);
        } else {
            if (((f_foo_str *)res->content)->argsCount > 0) {
                for (int j = 0; j < ((f_foo_str *) res->content)->argsCount; j++) {
                    free(((f_foo_str *) res->content)->args[j]);
                    ((f_foo_str *) res->content)->args[j] = NULL;
                }

                free(((f_foo_str *) res->content)->args);
                ((f_foo_str *) res->content)->args = NULL;
            }

            if (((f_foo_str *)res->content)->ret_args > 0) {
                free(((f_foo_str *)res->content)->ret_args);
                ((f_foo_str *) res->content)->ret_args = NULL;
            }
        }
    }
}

//function reads() : string
//function readi() : integer
//function readn() : number

//function tointeger(f : number) : integer
//function substr(s : string, i : number, j : number) : string
//function ord(s : string, i : integer) : integer
//function chr(i : integer) : string

void initBuildInFunctions(bst_node_t **globalTree)
{
    for (int i = 0; i < 8; i++) {
        f_foo_str *newFunction = malloc(sizeof(f_foo_str));
        newFunction->isDefined = true;
        newFunction->localVars = NULL;
        newFunction->opArray = NULL;
        newFunction->opCount = 0;
        newFunction->argsCount = 0;
        newFunction->args = NULL;
        newFunction->ret_args = NULL;

        if (i == 0) {
            newFunction->id = strdup("reads");
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_string;
            newFunction->retArgsCount = 1;
            newFunction->argsCount = 0;
        }
        else if (i == 1) {
            newFunction->id = strdup("readi");
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_integer;
            newFunction->retArgsCount = 1;
            newFunction->argsCount = 0;
        }
        else if (i == 2) {
            newFunction->id = strdup("readn");
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_number;
            newFunction->retArgsCount = 1;
            newFunction->argsCount = 0;
        }
        else if (i == 3) {
            newFunction->id = strdup("tointeger");
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_integer;
            newFunction->retArgsCount = 1;

            newFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * 1);
            newFunction->args[0] = allocateNewVariable(true, false, false, t_number, NULL, NULL, 0, 0, NULL );

            newFunction->args[0]->id = strdup("f");

            newFunction->args[0]->VarDesc = NULL;

            newFunction->argsCount = 1;
        }
        else if (i == 4) {
            newFunction->id = strdup("substr"); //function substr(s : string, i : number, j : number) : string
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_string;
            newFunction->retArgsCount = 1;

            newFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * 3);
            newFunction->args[0] = allocateNewVariable(true, false, false, t_string, NULL, NULL, 0, 0, NULL );
            newFunction->args[1] = allocateNewVariable(true, false, false, t_number, NULL, NULL, 0, 0, NULL );
            newFunction->args[2] = allocateNewVariable(true, false, false, t_number, NULL, NULL, 0, 0, NULL );

            newFunction->args[0]->id = strdup("s");
            newFunction->args[1]->id = strdup("i");
            newFunction->args[2]->id = strdup("j");

            newFunction->args[0]->VarDesc = NULL;
            newFunction->args[1]->VarDesc = NULL;
            newFunction->args[2]->VarDesc = NULL;

            newFunction->argsCount = 3;
        } else if (i == 5) {
            newFunction->id = strdup("ord"); //function ord(s : string, i : integer) : integer
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_integer;
            newFunction->retArgsCount = 1;

            newFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * 2);
            newFunction->args[0] = allocateNewVariable(true, false, false, t_string, NULL, NULL, 0, 0, NULL );
            newFunction->args[1] = allocateNewVariable(true, false, false, t_integer, NULL, NULL, 0, 0, NULL );

            newFunction->args[0]->VarDesc = NULL;
            newFunction->args[1]->VarDesc = NULL;

            newFunction->argsCount = 2;
        } else if (i == 6) {
            newFunction->id = strdup("chr"); //function chr(i : integer) : string
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->ret_args[0] = t_string;
            newFunction->retArgsCount = 1;

            newFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * 10);
            newFunction->args[0] = allocateNewVariable(true, false, false, t_integer, NULL, NULL, 0, 0, NULL );

            newFunction->args[0]->id = strdup("i");

            newFunction->args[0]->VarDesc = NULL;

            newFunction->argsCount = 1;
        } else if (i == 7) {
            newFunction->id = strdup("write"); //function write() -- defined as void but does not act as one
            newFunction->ret_args = malloc(sizeof(f_type) * 1);
            newFunction->argsCount = 0;
            newFunction->retArgsCount = 0;
            newFunction->args = NULL;
        }
        bst_insert(globalTree, newFunction->id, newFunction);
    }
}

token_t * readFooRetTypes(f_foo_str *newFunction,token_t *token) {
    destroyToken(token);

    while(true) {
        token = getNextToken();

        if (token->type == KEYW_TOKEN) {

            if (newFunction->retArgsCount >= retArgRealloc - 1)
            {
                retArgRealloc = retArgRealloc + 10;
                void *temp = realloc(newFunction->ret_args, sizeof(f_type) * retArgRealloc);
                if (temp != NULL) {
                    newFunction->ret_args = temp;
                }
                else {
                    exit(2);
                }
            }

            newFunction->ret_args[newFunction->retArgsCount] = getType(token);
            newFunction->retArgsCount++;
        } else
            exit(2); // nebol prideleny expected token

        destroyToken(token);

        token = getNextToken();
        if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ',') {
            destroyToken(token);
            continue;
        } else
            break;
    }

    newFunction->isDefined = true;
    return token;
}

void readSingleArg(f_foo_str * newFunction, f_foo_var *arg, token_t *token)
{
    int varNameLen = (int)strlen(token->strValue);
    arg->originalId = strdup(token->strValue); // copy old name before free
    arg->VarDesc = NULL;

    arg->id = malloc(varNameLen * sizeof(char) + 100 * sizeof(char));
    sprintf(arg->id, "%s",arg->originalId); //  sprintf(arg->id, "%d~arg~%s~%s", newFunction->argsCount, newFunction->id, arg->originalId);

    destroyToken(token);

    token = getNextToken(newFunction);

    if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ':') {
        destroyToken(token);

        token = getNextToken();

        if (token->type == KEYW_TOKEN) {
            arg->type = getType(token);
        }
        else
            exit(2);

        if (newFunction->argsCount >= argRealloc - 1) // reallocc created var array if needed
        {
            newFunction->args = reallocArgMemory(&argRealloc, newFunction->args);
        }

        newFunction->args[newFunction->argsCount] = arg;
        newFunction->argsCount++;

        destroyToken(token);
        return;
    }
    else
        exit(2);
}

// <foo_arg> -> ID : <type> <foo_arg_next>
void readFooArgs(f_foo_str *newFunction) // pome si precitat argumenty :)
{
    bool argRead = false;

    while(true)
    {
        token_t *token =  getNextToken();
        if (token->type == ID_TOKEN) {  // get arg id so there are no duplicates
            f_foo_var *arg = malloc(sizeof(f_foo_var)); // create new arg
            arg->id = NULL;
            arg->originalId = NULL;
            arg->stringVal = NULL;
            arg->isImmediate = arg->isHandle = false;
            arg->isDefined = true;

            argRead = false;
            readSingleArg(newFunction, arg, token);
        }
        else if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ')' && argRead == false) {
            destroyToken(token);
            break;
        }
        else if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ',') {
            argRead = true;
            destroyToken(token);
        }
        else
            exit(2);
    }
}

void killHandle(f_foo_var *retHandle) {
    if (retHandle != NULL) {
        if (retHandle->isImmediate) {
            if (retHandle->stringVal != NULL)
                free(retHandle->stringVal);
            if (retHandle->id != NULL) {
                free(retHandle->id);
            }
            free(retHandle);
        }
        else if (retHandle->isHandle) {
            freeVarDesc(&retHandle->VarDesc);
            free(retHandle);
        }
    }
}

void checkVarCollision(f_foo_var *var, f_foo_str *function, scope_switch usedScope) // checkne ci sa nekolidujuu args alebo vars v scope fcie
{
    bst_node_t *resultPtr = NULL;

    if (usedScope == function_case) {
        // args can't be the same as vars

        if (bst_search(function->localVars, var->id, &resultPtr))
            exit(2);
    }
    else if (usedScope == if_case || usedScope == else_case || usedScope == while_case) {
        for (int i = 0; i < function->opArray[function->opCount-1]->varCount; i++) {
            if (strcmp(function->opArray[function->opCount-1]->id, var->id) == 0)
                exit(2);
        }

        if (bst_search(function->opArray[function->opCount-1]->localScopeVars, var->id, &resultPtr))
            exit(2);
    }
    else {
        exit(2);
    }
}

f_foo_var *allocFooCallArg(token_t *token, scope_switch usedScope, int *nestLevel, f_foo_str *function, bst_node_t **globalTree) {
    f_foo_var *newVar = malloc(sizeof(f_foo_var));
    newVar->isImmediate = true;
    newVar->isHandle = false;
    newVar->isDefined = false;
    newVar->stringVal = NULL;
    newVar->stringVal = NULL;
    newVar->id = NULL;
    newVar->originalId = NULL;
    newVar->VarDesc = NULL;

    if (token->type == STRING_TOKEN) {
        newVar->type = t_string;
        newVar->stringVal = strdup(token->strValue);
    } else if (token->type == FLOATNUM_TOKEN) {
        newVar->type = t_number;
        newVar->numVal = token->dbValue;
    } else if (token->type == INTNUM_TOKEN) {
        newVar->type = t_integer;
        newVar->intVal = token->intValue;
    } else { // search for variables
        if (usedScope != global_case) {
            f_foo_var *returnPtr = NULL;
            returnPtr = getExistingVariable(function, token, nestLevel, usedScope, true);

            if (returnPtr != NULL) {
                free(newVar);
                return ((f_foo_var*)returnPtr);
            } else {
                fprintf(stderr, "Arg variable does not exist or is not defined");
                exit(2);
            }
        }
        else {
            bst_node_t *returnPtr = NULL;
            bst_search(*globalTree, token->strValue, &returnPtr);

            if (returnPtr != NULL) {
                free(newVar);
                return ((f_foo_var*)returnPtr->content);
            }
        }
    }

    return newVar;
}

token_t *callFunction(token_t *token, bst_node_t **globalTree, access_modifier fooCallSrc, scope_switch usedScope, int *nestLevel, f_foo_str *function, f_foo_str* calledFunction, var_desc ***retVarArrayHandle) {

    fprintf(stderr, "CALLED EXISTING FUNCTION : %s\n", calledFunction->id);

    f_foo_var **tmpArgArray = malloc(sizeof(f_foo_var*) * tmpArgRealloc);
    int argCount = 0;
    int symbolCount = 0;

    // read args that the foo is being called with
    while (true) {

        if (argCount >= tmpArgRealloc - 1) // reallocc created var array if needed
        {
            tmpArgArray = reallocArgMemory(&tmpArgRealloc, tmpArgArray);
        }

        if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ')')
        {
            break;
        }

        if (symbolCount % 2 == 0) {
            tmpArgArray[argCount] = allocFooCallArg(token, usedScope, nestLevel, function, globalTree);
            argCount++;
        }
        symbolCount++;
        if (token->type == EOF_TOKEN)
            exit(2);

        destroyToken(token);
        token = getNextToken();
    }
    destroyToken(token);

    if (strcmp(calledFunction->id, "write") == 0) {

        bst_node_t * retPtr = NULL;
        bst_search(*globalTree, calledFunction->id, &retPtr);

        if (retPtr == NULL) {
            exit(2);
        } else {
            f_foo_str *debug = ((f_foo_str*) retPtr->content);

            ((f_foo_str*) retPtr->content)->args = tmpArgArray;

            ((f_foo_str*) retPtr->content)->argsCount = argCount;
        }
    }


    // calledFunction->id, tmpArgArray[i], argCout
    //SEM-IMPLEMENT -> call for a fucntion check
    // no handle needed sp pls do not give me ani :prayge:
    // TODO SEND IT TO SEMANTAK :cry:


    *retVarArrayHandle =  fun_call(calledFunction, tmpArgArray, argCount);

    for (int i = 0; i < argCount; i++) {
        if (tmpArgArray[i]->isImmediate && !tmpArgArray[i]->isDefined) {

            if (tmpArgArray[i]->stringVal != NULL)
                free(tmpArgArray[i]->stringVal);

            free(tmpArgArray[i]);
        }
    }
    free(tmpArgArray);
    if (strcmp(calledFunction->id, "write") == 0) {
        calledFunction->argsCount = 0;
        calledFunction->args = NULL;
    }

    // checkign if function was called with the right amount of arguments
    if (calledFunction->argsCount != argCount) {
        if (strcmp(calledFunction->id, "write") != 0) { // different behaviour for write as it can eat unlimited args
            fprintf(stderr, "incorrect num of args to call function");
            exit(2);
        }
    }

    // checks if the function was called locally or globally
    if(fooCallSrc == local) {
        token = getNextToken();
        return token;
    }
    else
        return NULL;
}

// <st_list> -> local ID : <type> <st-list>
token_t *readLocalVar(f_foo_str *function, token_t *token, scope_switch usedScope, int *nestLevel, bst_node_t **globalTree)
{
    f_foo_var * newVar = malloc(sizeof(f_foo_var));
    newVar->id = NULL;
    newVar->originalId = NULL;
    newVar->stringVal = NULL;
    newVar->isHandle = false;
    newVar->isImmediate = false;
    newVar->isDefined = false;
    newVar->VarDesc = NULL;

    token = getNextToken();
    if (token->type == ID_TOKEN) {
        newVar->id = strdup(token->strValue);
        destroyToken(token);

        // load a variable (local ID <type>)
        token = getNextToken();
        if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ':') {
            destroyToken(token);

            token = getNextToken();
            if (token->type == KEYW_TOKEN) {
                if (strcmp(token->strValue, "integer")==0 || strcmp(token->strValue, "number")==0 || strcmp(token->strValue, "string")==0) {
                    newVar->type = getType(token);
                    destroyToken(token);
                } else {
                    exit(2);
                }
            } else {
                exit(2);
            }

            bst_node_t * resultPtr = NULL;
            bst_search(*globalTree, newVar->id, &resultPtr);

            if (resultPtr != NULL) {
                fprintf(stderr, "Varibale name ist he same as function name\n");
                exit(3);
            }

            if (usedScope == function_case) {
                checkVarCollision(newVar, function, usedScope);

                bst_insert(&(function->localVars), newVar->id, newVar);
                function->varCount++;
            }
            else if (usedScope == if_case) {
                checkVarCollision(newVar, function, usedScope);

                int varNameLen = (int)strlen(newVar->id);
                newVar->originalId = strdup(newVar->id); // copy old name before free
                free(newVar->id);

                newVar->id = malloc(varNameLen * sizeof(char) + 30 * sizeof(char));
                sprintf(newVar->id, "if-%s-%i-%i", newVar->originalId, function->opCount, function->opArray[function->opCount - 1]->nestLevel);


                f_op_scope_str *debugScopw = function->opArray[function->opCount - 1];

                bst_insert(&(function->opArray[function->opCount - 1]->localScopeVars), newVar->originalId, newVar);
                function->opArray[function->opCount - 1]->varCount++;
            }
            else if (usedScope == else_case) {
                checkVarCollision(newVar, function, usedScope);

                int varNameLen = (int)strlen(newVar->id);
                newVar->originalId = strdup(newVar->id); // copy old name before free
                free(newVar->id);

                newVar->id = malloc(varNameLen * sizeof(char) + 30 * sizeof(char));
                sprintf(newVar->id, "else-%s-%i-%i", newVar->originalId, function->opCount, function->opArray[function->opCount - 1]->nestLevel);
                bst_insert(&(function->opArray[function->opCount - 1]->localScopeVars), newVar->originalId, newVar);
                function->opArray[function->opCount - 1]->varCount++;
            }
            else if (usedScope == while_case) {
                checkVarCollision(newVar, function, usedScope);

                int varNameLen = (int)strlen(newVar->id);
                newVar->originalId = strdup(newVar->id); // copy old name before free
                free(newVar->id);

                f_op_scope_str *debugScopw = function->opArray[function->opCount - 1];

                newVar->id = malloc(varNameLen * sizeof(char) + 50 * sizeof(char));
                sprintf(newVar->id, "while-%s-%i-%i", newVar->originalId, function->opCount, function->opArray[function->opCount - 1]->nestLevel);
                bst_insert(&(function->opArray[function->opCount - 1]->localScopeVars), newVar->originalId, newVar);
                function->opArray[function->opCount - 1]->varCount++;
            }
            else
                exit(2);
        }
        else {
            exit(2);
        }


        newVar->VarDesc = decVar(newVar);

        // load a variable (local ID <type>) with assigned value local ID : <type> = <value>
        token = getNextToken();
        if (token->type == OP_TOKEN && *(token->strValue) == '=') {
            destroyToken(token);
            bool isFoo = false;
            f_foo_var  *retHandle = NULL;
            f_foo_str  *retFunction = NULL;

            token = readExpression(function, &retHandle, nestLevel, usedScope, globalTree, &isFoo, &retFunction);

            if (retHandle == NULL && !isFoo) {
                fprintf(stderr, "variable does not exist\n");
                exit(3);
            }

            if (*(token->strValue) == '(' && !isFoo) {
                fprintf(stderr, "Function does not exist");
                exit(3);
            }

            if (retHandle != NULL) {
                // this only passes if expression was processed (not a single variable/ constant)
                if (retHandle->VarDesc != NULL) {
                    if (true) {
                        fprintf(stderr, "ASSIGNING VAR DESCRIPTOR AT DECLARATION\n");
                        move_eq(newVar, retHandle);
                        killHandle(retHandle);
                        newVar->isDefined = true;
                    } else {
                        fprintf(stderr, "cannot assign, incorrect types\n");
                        exit(3);
                    }
                } else {
                    //retHandle->VarDesc = convertConstToReg(retHandle);
                    if (true) {
                        fprintf(stderr, "ASSIGNING VAR DESCRIPTOR AT DECLARATION\n");
                        move_eq(newVar, retHandle);
                        killHandle(retHandle);
                        newVar->isDefined = true;
                    } else {
                        exit(3);
                    }
                }
            }

            if (isFoo) {
                destroyToken(token);
                token = getNextToken();
                newVar->isDefined = true;

                var_desc **retVarArrayHandle = NULL;

                token = callFunction(token, globalTree, local, usedScope, nestLevel, function, retFunction, &retVarArrayHandle);

                hard_move(newVar, retVarArrayHandle[0]);

            }
            else {
                newVar->isDefined = true;
                return token;
            }

        }
    }
    else {
        exit(2); // cele zle neni ID rip program skaredyz
    }
    return token;
}

f_op_scope_str * allocateOpScope(scope_switch scopeType, f_foo_str *function, int *nestLevel ) {

    // alloc struct for if scope
    bst_node_t *localScope;
    bst_init(&localScope);

    // allocate new Op and allocate space for the id
    f_op_scope_str *newScope = malloc(sizeof(f_op_scope_str));
    int fooNameLen = (int)strlen(function->id);
    newScope->id = (char*)malloc(4 * sizeof(char) + fooNameLen * sizeof(char) + 30 * sizeof(char));

    // preset all bools to false and increment op count
    newScope->isElse = newScope->isWhile = newScope->isIf = false;
    function->currentOp++;

    // set local scope and nest level
    newScope->localScopeVars = localScope;
    newScope->nestLevel = (*nestLevel);
    newScope->varCount = 0;

    if (scopeType == if_case) {
        sprintf(newScope->id, "if~%s~%i~%i", function->id, function->ifCount+1, (*nestLevel));

        newScope->count = function->ifCount;
        function->ifCount++;

        newScope->isIf = true;
    }
    else if (scopeType == else_case) {
        sprintf(newScope->id, "else~%s~%i~%i", function->id, function->elseCount+1, (*nestLevel));

        newScope->count = function->elseCount;
        function->elseCount++;

        newScope->isElse = true;
    }
    else if (scopeType == while_case) {
        sprintf(newScope->id, "while~%s~%i~%i", function->id, function->whileCount+1, (*nestLevel));

        newScope->count = function->whileCount;
        function->whileCount++;

        newScope->isWhile = true;
    }
    else {
        exit(2);
    }

    if (function->opCount >= operationRealloc - 1) // reallocc created var array if needed
    {
        function->opArray = reallocOpScopeMemory(&operationRealloc, function->opArray);
    }

    // add it to function scope
    function->opArray[function->opCount] = newScope;
    function->opCount++;

    return newScope;
}

// <st_list> -> while <expression> do <st_list> end <st_list>
token_t *readLoop(f_foo_str *function, token_t *token, int *nestLevel, bst_node_t **globalTree)
{

    allocateOpScope(while_case, function, nestLevel);
    fprintf(stderr, "WHILE STARTED: %s\n", function->opArray[function->opCount-1]->id);
    while_announce();

    // check if condition si even valid or false
    f_foo_var *retHandle = NULL;
    f_foo_str *retFunction = NULL;
    bool isFoo = false;
    token = readExpression(function, &retHandle, nestLevel, if_case, globalTree, &isFoo, &retFunction);

    if (retHandle != NULL)
        while_start(retHandle);
    else {
        fprintf(stderr, "expected expression\n");
        exit(2);
    }

    // while 2 + 2 < 3 - 21

    //SEM-IMPLEMENT
    // po prejdeni expressionu dostanete naspat handle v -> retHandle <-
    // to si poslite kde chcete :pepeLa: ale naspat mi ju uz vraciat nemusite nakolko ja tuto podmienku uz nikdy citat nebudem
    // cize by bolo zbytocne niekde tu podmienku ukladat (ak bude velmi nutne tak tam tu podmienku kdesi ulozim i guess)
    // mazem ju v podmienke pod touto


    // expression is missing a keyword token after (expected to be `then`)
    if (token == NULL) {
        fprintf(stderr, "ERROR : 'do' token expected");
        exit(2);
    }

    if (token->type != KEYW_TOKEN || strcmp(token->strValue, "do") != 0) {
        exit(2);
    }
    destroyToken(token);
    token = getNextToken();

    if (token->type == EOF_TOKEN || token->type == ERR_TOKEN) {
        fprintf(stderr, "EOF token after 'do' ");
        exit(2);
    }

    token = stListRead(function, token, nestLevel, while_case, globalTree);

    if (token->type == KEYW_TOKEN && strcmp(token->strValue, "end") != 0) {
        exit(2);
    } else {
        fprintf(stderr, "ENDING WHILE\n");
        while_end();
        //SEM-IMPLEMENT -> tu si mozte dat call ze skoncil current scope/blok s podmienkou

        (*nestLevel)--;
        destroyToken(token);
        token = getNextToken();

        function->currentOp--;
    }
    return token;
}

// <st_list> -> if <expression> then <st_list> else <st_list> end <st_list>
token_t *readCondition(f_foo_str *function, token_t *token, int *nestLevel, bst_node_t **globalTree)
{
    // check if condition si even valid or false
    f_foo_var *retHandle = NULL;
    f_foo_str *retFunction = NULL;
    bool isFoo = false;

    allocateOpScope(if_case, function, nestLevel);
    fprintf(stderr, "CONDITION STARTED: %s\n", function->opArray[function->opCount-1]->id);

    token = readExpression(function, &retHandle, nestLevel, if_case, globalTree, &isFoo, &retFunction);

    if (retHandle != NULL)
        if_head(retHandle);
    else {
        fprintf(stderr, "expected condition\n");
        exit(2);
    }

    // po prejdeni expressionu dostanete naspat handle v -> retHandle <-
    // to si poslite kde chcete :pepeLa: ale naspat mi ju uz vraciat nemusite nakolko ja tuto podmienku uz nikdy citat nebudem
    // cize by bolo zbytocne niekde tu podmienku ukladat (ak bude velmi nutne tak tam tu podmienku kdesi ulozim i guess)


    // expression is missing a keyword token after (expected to be `then`)
    if (token == NULL) {
        fprintf(stderr, "'then' token expected");
        exit(2);
    }

    // dalej si precitame then :)
    if (token->type != KEYW_TOKEN || strcmp(token->strValue, "then") != 0) { // TODO checknut aj ostatne predtym bolo == && !=0
        exit(2);
    }
    destroyToken(token);
    token = getNextToken();
    if (token->type == EOF_TOKEN || token->type == ERR_TOKEN) {
        fprintf(stderr, "ERROR : EOF token after 'then' ");
        exit(2);
    }

    token = stListRead(function, token, nestLevel, if_case, globalTree);

    if (token->type == KEYW_TOKEN && strcmp(token->strValue, "else") == 0) {
        allocateOpScope(else_case, function, nestLevel);
        else_head();
        fprintf(stderr, "ELSE STARTED: %s\n", function->opArray[function->opCount-1]->id);
    }
    else {
        exit(2);
    }

    destroyToken(token);
    token = getNextToken();
    token = stListRead(function, token, nestLevel, else_case, globalTree);

    if (token->type == KEYW_TOKEN && strcmp(token->strValue, "end") != 0) {
        exit(2);
    } else {
        fprintf(stderr, "ENDING IF\n");
        //SEM-IMPLEMENT -> tu si mozte dat call ze skoncil current scope/blok s podmienkou
        end_if();

        (*nestLevel)--;
        destroyToken(token);
        token = getNextToken();

        function->currentOp--;
    }
    return token;
}

f_foo_var *getExistingVariable(f_foo_str *currentFunction, token_t *token, int *nestLevel, scope_switch usedScope, bool useDefined) {
    if (usedScope == function_case)
    {
        bst_node_t *resultPtr = NULL;
        bst_search(currentFunction->localVars, token->strValue, &resultPtr);
        if (useDefined) {
            if (resultPtr != NULL && ((f_foo_var *) resultPtr->content)->isDefined) // only use if the val is defined
                return resultPtr->content;
        }
        else {
            if (resultPtr != NULL)
                return resultPtr->content;
        }

        for (int i = 0; i < currentFunction->argsCount; i++) {
            if (strcmp(token->strValue, currentFunction->args[i]->originalId) == 0) {
                return currentFunction->args[i];
            }
        }

    }
    else if (usedScope == if_case || usedScope == else_case || usedScope == while_case)
    {
        // check if it's declared in current scope
        bst_node_t *resultPtr = NULL;

        if (currentFunction->opCount != 0) {

            bst_search((currentFunction->opArray[currentFunction->opCount-1]->localScopeVars), token->strValue, &resultPtr);
            if (useDefined) {
                if (resultPtr != NULL && ((f_foo_var *) resultPtr->content)->isDefined) // only use if the val is defined
                    return resultPtr->content;
            }
            else {
                if (resultPtr != NULL)
                    return resultPtr->content;
            }
        }

        // checking is it is no the root condition/while
        if (currentFunction->opCount - 1 != 0)
        {
            int iter = currentFunction->opCount;
            int tmpNestLevel = (*nestLevel);
            while (iter != 0) {
                // getting the nearest scope with lower nest level

                if (currentFunction->opArray[iter-1]->nestLevel < tmpNestLevel) {

                    f_op_scope_str * debugOp = currentFunction->opArray[iter-1];

                    tmpNestLevel--;
                    bst_search(currentFunction->opArray[iter-1]->localScopeVars, token->strValue, &resultPtr);
                    if (useDefined) {
                        if (resultPtr != NULL && ((f_foo_var *) resultPtr->content)->isDefined) // only use if the val is defined
                            return resultPtr->content;
                    }
                    else {
                        if (resultPtr != NULL)
                            return resultPtr->content;
                    }
                }

                iter--;
            }
            if (useDefined) {
                if (resultPtr != NULL && ((f_foo_var *) resultPtr->content)->isDefined) // only use if the val is defined
                    return resultPtr->content;
            }
            else {
                if (resultPtr != NULL)
                    return resultPtr->content;
            }
        }

        // lastly check function scope
        bst_search((currentFunction->localVars), token->strValue, &resultPtr);
        if (useDefined) {
            if (resultPtr != NULL && ((f_foo_var *) resultPtr->content)->isDefined) // only use if the val is defined
                return resultPtr->content;
        }
        else {
            if (resultPtr != NULL)
                return resultPtr->content;
        }

        for (int i = 0; i < currentFunction->argsCount; i++) {
            if (strcmp(token->strValue, currentFunction->args[i]->originalId) == 0) {
                return currentFunction->args[i];
            }
        }
        // err happened
        return NULL;
    }
    else
        return NULL;

    return NULL;
}

// <st_list>
token_t * stListRead(f_foo_str *currentFunction, token_t *token, int *nestLevel, scope_switch usedScope, bst_node_t **globalTree) {
    token_t *tmpToken = token;
    bool isFunction = false;
    bool multipleVars = false;

    while (true) {
        isFunction = false;
        multipleVars = false;

        if (token->type == KEYW_TOKEN && strcmp(token->strValue, "end") == 0 || token->type == KEYW_TOKEN && strcmp(token->strValue, "else") == 0)
            return token;

        if (token->type == INTNUM_TOKEN || token->type == FLOATNUM_TOKEN || token->type == STRING_TOKEN) {
            exit(2);
        }

        if (token->type == EOF_TOKEN) {
            exit(2);
        }

        if (token->type == ERR_TOKEN)
            exit(2); // TODO PROBABLY NOT EXIT 2?

        if (token->type == KEYW_TOKEN && strcmp(token->strValue, "local") == 0) {
            tmpToken = readLocalVar(currentFunction, token, usedScope, nestLevel, globalTree);
            destroyToken(token);
            token = tmpToken;
        }

        else if (token->type == ID_TOKEN) {
            f_foo_var *resultPtr = NULL;
            bst_node_t *fooResultPtr = NULL;

            f_foo_var* mpVarArr[100]; // TODO DYNAMICC
            int varCounter = 0;


            resultPtr = getExistingVariable(currentFunction, token, nestLevel, usedScope, false);
            bst_search(*globalTree, token->strValue, &fooResultPtr);

            bool expectID = false;

            while (token->type != OP_TOKEN) {
                if (resultPtr != NULL) {
                    mpVarArr[varCounter] = resultPtr; // get variable base on search above
                    varCounter++;
                    expectID = false;
                }

                destroyToken(token);
                token = getNextToken();

                if (token->type == EOF_TOKEN) {
                    exit(2); // RECENTLY-ADDED
                }

                if (*token->strValue == ',') {
                    expectID = true;
                }

                if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == '(')
                    break;

                resultPtr = getExistingVariable(currentFunction, token, nestLevel, usedScope, false);

                if (token->type == ID_TOKEN && resultPtr == NULL) {
                    fprintf(stderr, "undefined variable %s", token->strValue);
                    exit(3);
                }
            }

            if (expectID) {
                fprintf(stderr, "expecting identifier\n");
                exit(2);
            }

            // checking if ID = <value> is valid
            if (token->type == OP_TOKEN && *(token->strValue) == '=') {
                destroyToken(token);

                if (varCounter == 0) {
                    fprintf(stderr,"Undefined variuable ");
                    exit(3);
                }
            }
            else if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == '(') {
                if (fooResultPtr != NULL) {
                    destroyToken(token);
                    token = getNextToken();

                    var_desc **retVarArrayHandle = NULL;

                    token = callFunction(token, globalTree, local, usedScope, nestLevel, currentFunction,
                                         fooResultPtr->content, &retVarArrayHandle);
                    isFunction = true;
                }
                else {
                    exit(2);
                }
            }

            int exprCount = 0;
            f_foo_var* handleArray[100]; // TODO DYNAMICC
            bool isFoo = false;
            bool expectExpression = false;

            if(!isFunction) {
                // next check what is the value after `=`
                f_foo_var *retHandle = NULL;
                f_foo_str *retFunction = NULL;
                bool isSingleVar = true;

                while(true) {
                    if (!isSingleVar) {
                        destroyToken(token);
                        retHandle = NULL;
                    }
                    // read assigned expression
                    token = readExpression(currentFunction, &retHandle, nestLevel, usedScope, globalTree, &isFoo,
                                           &retFunction);

                    if (retHandle != NULL) {
                        expectExpression = false;
                        handleArray[exprCount] = retHandle;
                        exprCount++;

                        if (*(token->strValue) == ',') {
                            expectExpression = true;
                        }
                    }

                    if (token->type != CTRLCHAR_TOKEN && *(token->strValue) != ',')
                        break;

                    // Check if function was called
                    if (isFoo) {
                        if (!isSingleVar) {
                            exit(2); // function cannot be called if multiple args are present
                        }

                        destroyToken(token);
                        token = getNextToken();

                        var_desc **retVarArrayHandle = NULL;

                        token = callFunction(token, globalTree, local, usedScope, nestLevel, currentFunction,
                                             retFunction, &retVarArrayHandle);

                        int j = retFunction->retArgsCount-1;

                        for (int i = varCounter-1; i > -1 ; i--) {
                            mpVarArr[i]->isDefined = true;
                            hard_move(mpVarArr[i], retVarArrayHandle[j]);
                            j--;
                        }

                        // TODO ASSIGN RETURN HANDLES
                        break;
                    }

                    isSingleVar = false;
                }

                if (expectExpression) {
                    fprintf(stderr, "expected epxression\n");
                    exit(2);
                }

            }

            if (varCounter > exprCount && !isFoo) {
                fprintf(stderr, "More variables than expressions");
                exit(2);
            }

            int j = exprCount;
            if (!isFoo) {
                for (int i = varCounter - 1; i > -1; i--) {

                    f_foo_var *debug = handleArray[i];

                    j--;
                    if (j < 0) {
                        exit(2);
                    }

                    if (handleArray[j] != NULL) {
                        // this only passes if expression was processed (not a single variable/ constant)
                        if (handleArray[i]->VarDesc != NULL) {
                            if (true) {
                                fprintf(stderr, "ASSIGNING VAR DESCRIPTOR AT DECLARATION\n");
                                move_eq(mpVarArr[i], handleArray[j]);
                                killHandle(handleArray[j]);
                                mpVarArr[i]->isDefined = true;
                            } else {
                                fprintf(stderr, "cannot assign, incorrect types\n");
                                exit(3);
                            }
                        } else {
                            if (true) {
                                fprintf(stderr, "ASSIGNING VAR DESCRIPTOR AT DECLARATION\n");
                                move_eq(mpVarArr[i], handleArray[j]);
                                killHandle(handleArray[j]);
                                mpVarArr[i]->isDefined = true;
                            } else {
                                exit(3);
                            }
                        }
                    }
                }
            }
        }

        else if (token->type == KEYW_TOKEN && strcmp(token->strValue, "if") == 0) {
            (*nestLevel)++;

            if (currentFunction->opCount >= operationRealloc - 1)
            {
                currentFunction->opArray = reallocOpScopeMemory(&operationRealloc, currentFunction->opArray);
            }

            tmpToken = readCondition(currentFunction, token, nestLevel, globalTree);
            destroyToken(token);
            token = tmpToken;
        }

        else if (token->type == KEYW_TOKEN && strcmp(token->strValue, "while") == 0) {
            // TODO POSli to tomu ugly kodeenu :)

            (*nestLevel)++;
            if (currentFunction->opCount >= operationRealloc - 1)
            {
                currentFunction->opArray = reallocOpScopeMemory(&operationRealloc, currentFunction->opArray);
            }

            tmpToken = readLoop(currentFunction, token, nestLevel, globalTree);
            destroyToken(token);
            token = tmpToken;
        }

        else if (token->type == KEYW_TOKEN && strcmp(token->strValue, "return") == 0) {

            f_foo_var  *retHandle = NULL;
            f_foo_str  *retFunction = NULL;
            bool isFoo = false;
            f_foo_var * tmpRetArray[100];
            int tmpRetCount = 0;

            tmpToken = readExpression(currentFunction, &retHandle, nestLevel, usedScope, globalTree, &isFoo, &retFunction);

            if (retHandle != NULL ) {

                tmpRetArray[0] = retHandle;
                tmpRetCount++;

                if (tmpToken->type == CTRLCHAR_TOKEN && *(tmpToken->strValue) == ',') {
                    destroyToken(tmpToken);
                    while (true) {
                        tmpToken = readExpression(currentFunction, &retHandle, nestLevel, usedScope, globalTree, &isFoo,
                                                  &retFunction);


                        tmpRetArray[tmpRetCount] = retHandle;
                        tmpRetCount++;

                        if (tmpToken->type != CTRLCHAR_TOKEN && *(tmpToken->strValue) != ',') {
                            break;
                        }

                        destroyToken(tmpToken);
                    }
                }

                to_return(currentFunction, tmpRetArray, tmpRetCount);
                funDec_ret();
                fprintf(stderr, "RETURN IS NOT EMPTY\n");

                for (int i = 0; i < tmpRetCount; i++) {
                    if (tmpRetArray[i]->isImmediate && !tmpRetArray[i]->isDefined) {

                        if (tmpRetArray[i]->stringVal != NULL)
                            free(tmpRetArray[i]->stringVal);

                        if (tmpRetArray[i]->VarDesc != NULL)
                            freeVarDesc(&tmpRetArray[i]->VarDesc);

                        free(tmpRetArray[i]);
                    }
                }

                // TODO CAN I FREE COSTANT?

            }
            else {

                to_return(currentFunction, tmpRetArray, tmpRetCount);
                funDec_ret();
                fprintf(stderr, "RETURN IS EMPTY\n");
            }
            destroyToken(token);
            token = tmpToken;
        } else {
            fprintf(stderr, "illegal character\n");
            exit(2);
        }

    }
}

// <PROGRAM> -> require "ifj21"
void checkHeader(token_t *token, int pos) // using ifj blabla
{
    if (pos == 2 && strcmp(token->strValue, "require") == 0 || pos == 1 && strcmp(token->strValue, "ifj21") == 0)
        return;
    else
        exit(2);
}

// <PROGRAM> -> function ID(<foo_arg>) : <ret_type> <st-list> end
void checkFunction(token_t *token, bst_node_t **globalTree)
{
    bst_node_t *localScope;
    bst_init(&localScope);

    bst_node_t *localArgScope;
    bst_init(&localArgScope);

    f_foo_str *newFunction = malloc(sizeof(f_foo_str));
    newFunction->argsCount = newFunction->retArgsCount = newFunction->varCount = newFunction->opCount = newFunction->currentOp = 0;
    newFunction->isDefined = true;
    newFunction->localVars = localScope;
    newFunction->opArray = (f_op_scope_str **) malloc(sizeof(f_op_scope_str*) * operationRealloc);
    newFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * argRealloc);
    newFunction->ret_args = malloc(sizeof (f_type) * retArgRealloc);
    newFunction->opCount = 0;
    newFunction->ifCount = 0;
    newFunction->elseCount = 0;
    newFunction->whileCount = 0;


    // set scope type
    scope_switch currentScope = function_case;

    bool fooHeaderRead = false;

    // #######################################################################
    // ziskame meno funkcie
    token =  getNextToken();

    bst_node_t *returnPtr = NULL;
    bst_search(*(globalTree), token->strValue, &returnPtr);
    if (token->type == ID_TOKEN) {
        newFunction->id = strdup(token->strValue);
        destroyToken(token);
    }
    else {
        exit(2); // cele zle neni ID rip program skaredy
    }

    // #######################################################################
    // dalej ceme zatvorecku ofc a prejdeme do <foo_arg> :neboj:
    token =  getNextToken();

    if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == '(') {
        readFooArgs(newFunction);

        destroyToken(token);
    }
    else {
        exit(2); // cele zle neni ID rip program skaredy
    }

    // #######################################################################
    // ')' bola precitana, ocakavam : alebo E
    token =  getNextToken();

    if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ':') {
        token = readFooRetTypes(newFunction, token);
    }

    funDec_s(newFunction);

    // TODO SEMANTAK -> toto asi nemam robit :pepeLa: kedze checkujem typy globalnej fcie
    if (returnPtr != NULL) { // check if global declaration did not happen
        if (strcmp(((f_foo_str*)returnPtr->content)->id, newFunction->id) == 0) {
            if ( ((f_foo_str*)returnPtr->content)->argsCount == newFunction->argsCount ) {
                for (int i = 0; i < newFunction->argsCount; i++) {
                    if (((f_foo_str*)returnPtr->content)->args[i]->type != newFunction->args[i]->type) {
                        fprintf(stderr, "foo definition is not the same as declaration big bad\n");
                        exit(3);
                    }
                }

                if (newFunction->retArgsCount == ((f_foo_str*)returnPtr->content)->retArgsCount) {
                    for (int i = 0; i < newFunction->retArgsCount; i++) {
                        if (((f_foo_str *) returnPtr->content)->ret_args[i] != newFunction->ret_args[i]) {
                            fprintf(stderr, "foo definition is not the same as declaration big bad\n");
                            exit(3);
                        }
                    }
                } else {
                    exit(3);
                }
            }else {
                fprintf(stderr, "foo definition is not the same as declaration big bad\n");
                exit(3);
            }

        } else {
            fprintf(stderr, "foo definition is not the same as declaration big bad\n");
            exit(3);
        }

        f_foo_str *tmp = ((f_foo_str*)returnPtr->content);
    }

    bst_node_t *resPtr = NULL;
    bst_search(*globalTree, newFunction->id, &resPtr);
    if (returnPtr != NULL) {
        if (((f_foo_str*)returnPtr->content)->isDefined) {

            f_foo_str  * debug = ((f_foo_str*)returnPtr->content);

            fprintf(stderr, "fucntion with the same name already exists\n");
            exit(3);
        }
    }


    bst_insert(globalTree, newFunction->id, newFunction);

    // #######################################################################
    // READ THE SCOPE OF FUNCTION

    int nestLevel = 0; // for if conditions
    token = stListRead(newFunction, token, &nestLevel, currentScope, globalTree);

    if (token->type == KEYW_TOKEN && strcmp(token->strValue, "end") != 0)
        exit(2);


    fprintf(stderr, "ENDING FUNCTION\n");
    //SEM-IMPLEMENT -> tu si mozte dat call ze skoncil current scope/blok s podmienkou
    funDec_e();

    destroyToken(token);


    // DISPOSING OF ALLOCATED FUNCTION
}

void readGlobalFooArgs(f_foo_str *newFunction) // pome si precitat argumenty :)
{
    while(true)
    {
        token_t *token =  getNextToken();
        if (token->type == KEYW_TOKEN) {
            f_foo_var *arg = malloc(sizeof(f_foo_var)); // create new arg
            arg->id = NULL;
            arg->originalId = NULL;
            arg->stringVal = NULL;
            arg->isImmediate = arg->isHandle = false;
            arg->isDefined = true;
            arg->type = getType(token);

            if (newFunction->argsCount >= globalArgRealloc - 1) // reallocc created var array if needed
            {
                newFunction->args = reallocArgMemory(&globalArgRealloc, newFunction->args);
            }

            newFunction->args[newFunction->argsCount] = arg;
            newFunction->argsCount++;
        }
        else if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ')') {
            destroyToken(token);
            break;
        }
        else if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ',') {
        }
        else
            exit(2);

        destroyToken(token);
    }
}

// <PROGRAM> -> global ID : function(<types>) : <ret_type> <PROGRAM>
token_t *checkGlobalFunction(token_t *token, bst_node_t **globalTree) {
    destroyToken(token);
    token = getNextToken();

    bst_node_t *localArgScope;
    bst_init(&localArgScope);

    f_foo_str *newGlobalFunction = malloc(sizeof(f_foo_str));
    newGlobalFunction->argsCount = newGlobalFunction->retArgsCount = newGlobalFunction->varCount = newGlobalFunction->opCount = newGlobalFunction->currentOp = 0;
    newGlobalFunction->args = (f_foo_var **) malloc(sizeof(f_foo_var *) * argRealloc);
    newGlobalFunction->ret_args = malloc(sizeof(f_type) * retArgRealloc);
    newGlobalFunction->isDefined = false;
    newGlobalFunction->opCount = 0;
    newGlobalFunction->ifCount = 0;
    newGlobalFunction->elseCount = 0;
    newGlobalFunction->whileCount = 0;
    newGlobalFunction->opArray = NULL;
    newGlobalFunction->localVars = NULL;


    if (token->type != ID_TOKEN) {
        fprintf(stderr, "incorrect global foo declaration");
        exit(2);
    } else {
        newGlobalFunction->id = strdup(token->strValue);
    }

    destroyToken(token);
    token = getNextToken();

    if (token->type != CTRLCHAR_TOKEN && *(token->strValue) != ':') {
        exit(2);
    }

    destroyToken(token);
    token = getNextToken();

    if (token->type != KEYW_TOKEN && strcmp(token->strValue, "function") != 0) {
        exit(2);
    }

    destroyToken(token);
    token = getNextToken();

    if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == '(') {
        destroyToken(token);

        readGlobalFooArgs(newGlobalFunction);

    }
    else {
        exit(2); // cele zle neni ID rip program skaredy
    }

    token =  getNextToken();

    if (token->type == CTRLCHAR_TOKEN && *(token->strValue) == ':') {
        token = readFooRetTypes(newGlobalFunction, token);
    }

    newGlobalFunction->isDefined = false;
    bst_insert(globalTree, newGlobalFunction->id, newGlobalFunction);

    return token;
}

void startReading()
{
    beginDocument();
    bst_node_t *globalTree;
    bst_init(&globalTree);

    int headerCount = 2; // read the header first
    bool skipToken = true;
    token_t *token =  getNextToken();

    initBuildInFunctions(&globalTree);

    while(true) {
        if (!skipToken)
            token = getNextToken();

        skipToken = false;
        if(token->type == ERR_TOKEN)
            exit(2); // just die

        else if(token->type == EOF_TOKEN) { // TODO set when EOF is correct and when it is not
            destroyToken(token);
            break;
        }

        if (headerCount != 0)
        {
            checkHeader(token, headerCount);
            headerCount--;
        }
        else if (token->type == KEYW_TOKEN && strcmp(token->strValue, "function") == 0) // foo def
        {
            checkFunction(token, &globalTree);
        }
        else if (token->type == KEYW_TOKEN && strcmp(token->strValue, "global") == 0) // foo decl
        {
            token = checkGlobalFunction(token, &globalTree);
            skipToken = true;
        }
        else if (token->type == ID_TOKEN) // foo call
        {
            bst_node_t *resultPtr = NULL;
            bst_search(globalTree, token->strValue, &resultPtr);

            if (resultPtr != NULL) {
                destroyToken(token);
                token = getNextToken();

                if (*token->strValue != '(') {
                    exit(2);
                }

                destroyToken(token);
                token = getNextToken();


                var_desc **retVarArrayHandle = NULL;

                token = callFunction(token, &globalTree, global, global_case, NULL, NULL, ((f_foo_str *)resultPtr->content), &retVarArrayHandle);
            }
            else {
                exit(2);
            }
        }
        if (token != NULL && !skipToken)
            destroyToken(token);
    }

    endDocument();
    //disposeBuiltinFooArgs(&globalTree);
    bst_dispose(&globalTree, function);
}
