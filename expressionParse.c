/**
 * Igor Hanus <xhanus19>
 */


#include "expressionParse.h"
#include "stdbool.h"
#include "tokenizer.h"
#include "synCheck.h"
#include "stdlib.h"
#include "stack.h"
#include "string.h"
#include "stdio.h"
#include "semantic.h"
#include "codegen-interface.h"

int varCount = 0;
int maxCount = 0;

int precTable[TAB_SIZE][TAB_SIZE] = {
        //buffer       +      /      C      #      .      (      )      i      n      $  // stack
        {R, L, R, L, R, L, R, L, X, R}, // +-
        {R, R, R, L, R, L, R, L, L, R}, // /*//
        {L, L, X, L, L, L, R, L, L, R}, // CMP
        {R, R, R, X, R, L, R, L, X, R}, // # // edit on S 3 B 6 from X to R
        {L, L, R, L, L, L, R, L, X, R}, // ..
        {L, L, L, L, L, L, P, L, X, X}, // (
        {R, R, R, R, R, X, R, X, X, R}, // )
        {R, R, R, R, R, X, R, N, X, R}, // i
        {X, X, R, X, X, X, X, X, N, X}, // nil
        {L, L, L, L, L, L, X, L, L, X}  // $
};


// testing i*i+i

// returns operator enum in int value
int getOperator(int stackIndex, int bufferIndex) {
    return precTable[stackIndex][bufferIndex];
}

void resetStack(Stack *stack, int index) {
    popByIndex(stack, index);
    stack->array[stack->topIndex].id = E;
}

void sendExpression(f_foo_var **tmpVarArray, Operations operation, int numOfOp)
{
    if (varCount < 2 && operation != len) {
        fprintf(stderr, "u most likely used an undefined variable lol\n");
        exit(2);
    }

    f_foo_var * unaryOpUse = NULL;

    if (operation != len)
        unaryOpUse = tmpVarArray[varCount - 2];

    f_foo_var *retVar = variable_check(unaryOpUse, tmpVarArray[varCount - 1], operation);

    varCount -= 1; // ignore the lat two used variables

    // TODO free var desc
    // very ugly riesenie
    if(tmpVarArray[varCount]->isHandle){
        freeVarDesc(&(tmpVarArray[varCount]->VarDesc));
        free(tmpVarArray[varCount]);
        tmpVarArray[varCount] = NULL;
        maxCount--; // the point of this is that if this value gets overwritten I do not want it to be double freed
    }
    else if(tmpVarArray[varCount]->isImmediate){
        if (tmpVarArray[varCount]->type == t_string)
            free(tmpVarArray[varCount]->stringVal);

        free(tmpVarArray[varCount]);
        tmpVarArray[varCount] = NULL;
        maxCount--;
    }


    // free if value is a handle or immediate before replacing it
    if (operation != len) {
        varCount -= 1; // ignore the lat two used variables

        if (tmpVarArray[varCount]->isHandle) {
            freeVarDesc(&(tmpVarArray[varCount]->VarDesc));
            free(tmpVarArray[varCount]);
            tmpVarArray[varCount] = NULL;
            maxCount--;
        } else if (tmpVarArray[varCount]->isImmediate) {

            if (tmpVarArray[varCount]->type == t_string)
                free(tmpVarArray[varCount]->stringVal);

            free(tmpVarArray[varCount]);
            tmpVarArray[varCount] = NULL;
            maxCount--;
        }
    }
    tmpVarArray[varCount] = retVar;

    varCount++;

    if (varCount > maxCount)
        maxCount = varCount;
}

bool checkRules(Stack *stack, f_foo_var **tmpVarArray) {
    int index = stackSearchSymbol(stack, less_than, false, false); // find last <

    switch (stack->topIndex - index) {
        case 1:
            // E -> i
            if (stack->array[index + 1].tableOperation == t_i) {
                stackPop(stack);
                stack->array[stack->topIndex].id = E;
                return true;
            }
            // E -> nil
            if (stack->array[index + 1].id == nil) {
                stackPop(stack);
                stack->array[stack->topIndex].id = E;
                return true;
            }
            exit(2);
        case 2:
            // E -> #E
            if (stack->array[index + 1].id == len) {
                sendExpression(tmpVarArray, stack->array[index + 1].id, 1);
                resetStack(stack, index);
                return true;
            }
            exit(2);
        case 3:
            // TODO zatial ich nedavam do kopy (rovnaky scope ifov) keby nahodou nieco trebalo pre buducnost
            // E -> E + E
            if (stack->array[index + 1].id == E && stack->array[index + 2].id == plus && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E * E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == mul && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E - E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == minus && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray,stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E / E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == fdiv && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            } // E -> E // E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == idiv && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E .. E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == concat && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> (E)
            else if (stack->array[index + 1].id == left_br && stack->array[index + 2].id == E && stack->array[index + 3].id == right_br) {
                resetStack(stack, index);
                return true;
            }
                // E -> E == E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == eq && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E ~= E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == not_eq && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E >= E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == more_eq_than && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E <= E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == less_eq_than && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E < E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == less_than && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
                // E -> E > E
            else if (stack->array[index + 1].id == E && stack->array[index + 2].id == more_than && stack->array[index + 3].id == E) {
                sendExpression(tmpVarArray, stack->array[index + 2].id, 2);
                resetStack(stack, index);
                return true;
            }
            break;

        default:
            return false;
    }


    return true;
}

bool checkSyntax(Stack *stack, Table_item *tableItem, Table_operators operator, f_foo_var **tmpVarArray) {
    switch (operator) {
        case (L): // <
            stackPushBeforeOperation(stack, tableItem);
            break;
        case (R): // >
            return checkRules(stack, tmpVarArray);
            break;
        case (P): // =
            stackPush(stack, *tableItem);
            break;
        default:
            return false;
    }

    return true;
}

void getTableItem(token_t *token, Table_item *tableItem) {
    tableItem->isOperation = true;

    switch (token->type) {
        case (CTRLCHAR_TOKEN):
            if (*(token->strValue) == '(') {
                tableItem->tableOperation = t_left_br;
                tableItem->id = left_br;
                return;
            } else if (*(token->strValue) == ')') {
                tableItem->tableOperation = t_right_br;
                tableItem->id = right_br;
                return;
            }
            break;
        case (ID_TOKEN):
            tableItem->tableOperation = t_i;
            tableItem->id = i;
            return;
        case (OP_TOKEN):
            if (*(token->strValue) == '+') {
                tableItem->tableOperation = t_plus_minus;
                tableItem->id = plus;
                return;
            } else if (*(token->strValue) == '-') {
                tableItem->tableOperation = t_plus_minus;
                tableItem->id = minus;
                return;
            } else if (strcmp(token->strValue, "//") == 0) {
                tableItem->tableOperation = t_div_mul;
                tableItem->id = idiv;
                return;
            } else if (*(token->strValue) == '/') {
                tableItem->tableOperation = t_div_mul;
                tableItem->id = fdiv;
                return;
            } else if (strcmp(token->strValue, "==") == 0) {
                tableItem->tableOperation = t_compare;
                tableItem->id = eq;
                return;
            } else if (strcmp(token->strValue, "~=") == 0) {
                tableItem->tableOperation = t_compare;
                tableItem->id = not_eq;
                return;
            } else if (strcmp(token->strValue, ">=") == 0) {
                tableItem->tableOperation = t_compare;
                tableItem->id = more_eq_than;
                return;
            } else if (strcmp(token->strValue, "<=") == 0) {
                tableItem->tableOperation = t_compare;
                tableItem->id = less_eq_than;
                return;
            } else if (*(token->strValue) == '>') {
                tableItem->tableOperation = t_compare;
                tableItem->id = more_than;
                return;
            } else if (*(token->strValue) == '<') {
                tableItem->tableOperation = t_div_mul;
                tableItem->id = less_than;
                return;
            } else if (*(token->strValue) == '*') {
                tableItem->tableOperation = t_div_mul;
                tableItem->id = mul;
                return;
            } else if (strcmp(token->strValue, "..") == 0) {
                tableItem->tableOperation = t_concat;
                tableItem->id = concat;
                return;
            } else if (*(token->strValue) == '#') {
                tableItem->tableOperation = t_get_len;
                tableItem->id = len;
                return;
            }
        case (INTNUM_TOKEN):
            tableItem->tableOperation = t_i;
            tableItem->id = integer;
            return;
        case (FLOATNUM_TOKEN):
            tableItem->tableOperation = t_i;
            tableItem->id = number;
            return;
        case (STRING_TOKEN):
            tableItem->tableOperation = t_i;
            tableItem->id = string;
            return;
        case (EOF_TOKEN):
            tableItem->tableOperation = t_end;
            tableItem->id = end;
            return;
        case (KEYW_TOKEN):
            if (strcmp(token->strValue, "nil") == 0) {
                tableItem->tableOperation = t_nil;
                tableItem->id = nil;
            }
            return;
        default:
            exit(2);
    }
}

void freeAllMem(Stack *stack, Table_item *tableItem, token_t * previousToken, f_foo_var **tmpVarArray)
{
    free(stack);
    free(previousToken);
    free(tableItem);


    for (int i = 1; i < maxCount-1; i++){
        if (tmpVarArray[i]->isHandle) {
            free(tmpVarArray[i]->id);

            free(tmpVarArray[i]);
        }
        else if (tmpVarArray[i]->isImmediate) {

            free(tmpVarArray[i]);
        }
    }

    maxCount = 0;
    varCount = 0;
    if (tmpVarArray != NULL)
        free(tmpVarArray);
}

token_t * readExpression(f_foo_str *function, f_foo_var** handle, int *nestLevel, scope_switch usedScope, bst_node_t **globalTree, bool *isFoo, f_foo_str** calledFunction) {
    bool takeToken = true; // set to false so the first token si processed (the one few lines under)
    bool readRulesOnly = false;

    bst_node_t *fooResultPtr = NULL; // catch a function if correct

    f_foo_var **tmpVarArray = malloc(sizeof(f_foo_var*) * ALLOC_CAP);

    Stack *stack = malloc(sizeof(Stack));
    Table_item *tableItem = malloc(sizeof(Table_item));
    token_t *previousToken = malloc(sizeof(token_t));

    token_t *token = NULL; // get first token (too lazy to allocate, valgrind hates me)

    tableItem->tableOperation = t_end;
    tableItem->isOperation = true;
    tableItem->id = end;

    stackInit(stack);
    stackPush(stack, *tableItem); // stack now only contains $

    while (stack->array[stack->topIndex].id != E || stack->array[stack->topIndex - 1].id != end || previousToken->type != EOF_TOKEN) {
        // TODO DEBUG DELTE LATER
        stackPrint(stack);

        if (function->varCount >= ALLOC_CAP - 1) // reallocc created var array if needed
        {
            void *temp = realloc(tmpVarArray, sizeof(f_foo_var*) * ALLOC_CAP);
            if (temp != NULL) {
                tmpVarArray = temp;
            }
            else {
                exit(2);
            }
        }

        if (takeToken) {
            token = getNextToken();

            f_foo_var *resultPtr = NULL;
            resultPtr = getExistingVariable(function, token, nestLevel, usedScope, true);

            if (resultPtr == NULL && ((stack->array[stack->topIndex].tableOperation == t_i) && (strcmp(token->strValue, "(") == 0))) { // called a function ID

                if (fooResultPtr != NULL) {
                    *(isFoo) = true;
                    *calledFunction = ((f_foo_str *)fooResultPtr->content);
                    freeAllMem(stack, tableItem, previousToken, tmpVarArray);
                    return token;
                }
            }

            if (token->type == ID_TOKEN ) {

                bst_search(*(globalTree), token->strValue, &fooResultPtr);

                if (resultPtr != NULL && ((f_foo_var*)resultPtr)->isDefined && stack->array[stack->topIndex].tableOperation != t_i) {
                    tmpVarArray[varCount] = ((f_foo_var*)resultPtr);
                    varCount++;

                    if (varCount > maxCount)
                        maxCount = varCount;
                }
            }
            else if (token->type == INTNUM_TOKEN || token->type == STRING_TOKEN || token->type == FLOATNUM_TOKEN || (token->type == KEYW_TOKEN && strcmp(token->strValue, "nil") == 0)) {
                f_foo_var *immediateValue = malloc(sizeof(f_foo_var));
                immediateValue->id = NULL;
                immediateValue->stringVal = NULL;
                immediateValue->isImmediate = true;
                immediateValue->isHandle = false;
                immediateValue->isDefined = false;
                immediateValue->VarDesc = NULL;

                if (token->type == INTNUM_TOKEN) {
                    immediateValue->intVal = token->intValue;
                    immediateValue->type = t_integer;
                }
                else if (token->type == FLOATNUM_TOKEN) {
                    immediateValue->numVal = token->dbValue;
                    immediateValue->type= t_number;
                }
                else if (token->type == STRING_TOKEN) {
                    immediateValue->type = t_string;
                    immediateValue->stringVal = strdup(token->strValue);
                }
                else if (token->type == KEYW_TOKEN) {
                    if (strcmp(token->strValue, "nil") != 0) {
                        fprintf(stderr, "should be nil");
                    }

                    immediateValue->type = t_null;
                    //immediateValue->stringVal = strdup(token->strValue);
                }
                else
                    exit(2);

                tmpVarArray[varCount] = immediateValue;
                varCount++;

                if (varCount > maxCount)
                    maxCount = varCount;
            }


        }
        takeToken = true;

        getTableItem(token, tableItem); // create a struct for received token, in the next stage retrieve stack tab value
        Table_operations stackOperation = getIndexValue(stack, stackSearchSymbol(stack, -1, true, true)).tableOperation;



        Table_operators operator = getOperator(stackOperation, tableItem->tableOperation); // get operator from table

        if (readRulesOnly) {
            operator = R;
        }

        if (operator == X) {
            if (varCount > 0)
                (*handle) = tmpVarArray[0];
            freeAllMem(stack, tableItem, previousToken, tmpVarArray);
            return token;
        }

        if (operator == R && stack->array[stack->topIndex].id == E && stack->topIndex - 1 == 0) {
            if (varCount > 0)
                (*handle) = tmpVarArray[0]; // TODO CALL SEMANT ANAL to assign a single value a = b / a = 4
            freeAllMem(stack, tableItem, previousToken, tmpVarArray);
            return token;
        }

        if (operator == R) // if a table rule needs to be used, don't read next token (in the next cycle)
            takeToken = false;
        else {
            if (token->type == KEYW_TOKEN && tableItem->tableOperation != t_nil) {
                operator = R;
                takeToken = false;

                if (stack->topIndex == 1 && stack->array[stack->topIndex].id == E) {
                    if (varCount > 0)
                        (*handle) = tmpVarArray[0]; // TODO CALL SEMANT ANAL to assign a single value a = b / a = 4
                    freeAllMem(stack, tableItem, previousToken, tmpVarArray);
                    return token;
                }
            }
        }

        if (!checkSyntax(stack, tableItem, operator, tmpVarArray)) { // check if syntax is valid, if not, free memory (also fails if EOF)
            readRulesOnly = true; // syntax failed so possible cause is that two IDs are behind each other
            takeToken = false; // do not take any more tokens and just check if the expression is correct
        }

        *previousToken = *token; // used for the last token read so loop ends correctly

        if (takeToken) // if token was taken, free it
            destroyToken(token);
    }

    // TODO DEBUG REMOVE
    //stackPrint(stack);

    (*handle) = tmpVarArray[0];
    freeAllMem(stack, tableItem, previousToken, tmpVarArray);
    destroyToken(token);
    return NULL;
}
