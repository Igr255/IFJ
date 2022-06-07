/**
 * Igor Hanus <xhanus19>
 */

#include "stdbool.h"
#include "expressionParse.h"

#ifndef _STACK_H
#define _STACK_H

#define MAX_SIZE 500

typedef struct {
    Table_operations tableOperation;
    bool isOperation;
    Operations id;
    // later also a type and shit for stupid chujovina analyzator semantika hej
} Table_item;

typedef struct {
    Table_item array[MAX_SIZE];
    int topIndex;
} Stack;

bool stackEmpty(Stack *stack);
bool stackFull(Stack *stack);
void stackInit(Stack *stack);
bool stackPush(Stack *stack, Table_item value);
bool stackPop(Stack *stack);
Table_item stackTop(Stack *stack);
bool stackPushBeforeOperation(Stack *stack, Table_item *newTableItem);
int stackSearchSymbol(Stack *stack, Operations symbol, bool isOperation, bool ignoreSymbol);
int stackSearchLastOperation(Stack *stack);
Table_item getIndexValue(Stack *stack, int index);
bool popMultiple(Stack *stack, int count);
void popByIndex(Stack *stack, int index);
void stackPrint(Stack *stack);

#endif //_STACK_H
