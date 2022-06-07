/**
 * Igor Hanus <xhanus19>
 */

#include "stack.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdbool.h"
#include "expressionParse.h"
#include "stdio.h"

void stackInit(Stack *stack) {
    stack->topIndex = -1;
}

bool stackFull(Stack *stack) {
    return stack->topIndex + 1 == MAX_SIZE;
}

bool stackEmpty(Stack *stack) {
    return stack->topIndex == -1;
}

bool stackPush(Stack *stack, Table_item value) {
    if (stackFull(stack))
        return false;

    stack->topIndex++;
    stack->array[stack->topIndex] = value;

    return true;
}

bool stackPop(Stack *stack) {
    if (stackEmpty(stack))
        return false;

    stack->topIndex--;
    return true;
}

Table_item stackTop(Stack *stack) {
    return stack->array[stack->topIndex];
}

int stackSearchSymbol(Stack *stack, Operations symbol, bool isOperation, bool ignoreSymbol) {
    int searchIndex = stack->topIndex;

    while (searchIndex != -1) {
        if (stack->array[searchIndex].id == symbol || ignoreSymbol) {
            if (stack->array[searchIndex].isOperation == isOperation) {
                return searchIndex;
            }
        }
        searchIndex--;
    }
    return -1;
}

int stackSearchLastOperation(Stack *stack) {
    int searchIndex = stack->topIndex;

    while (searchIndex != -1) {
        if (stack->array[searchIndex].isOperation) {
            return searchIndex;
        }
        searchIndex--;
    }
    return -1;
}

bool stackPushBeforeOperation(Stack *stack, Table_item *newTableItem) {
    if (stackEmpty(stack))
        return false;

    int currentIndex = stack->topIndex;

    if (stack->topIndex != MAX_SIZE - 1) {
        // shift values until an operation is reached
        while (!stack->array[currentIndex].isOperation) {
            stack->array[currentIndex + 1] = stack->array[currentIndex];
            currentIndex--;
        }

        Table_item *tableItem = malloc(sizeof(Table_item));
        tableItem->isOperation = false;
        tableItem->id = less_than; // toto tu fakt ze je len pre pokoj duse nech tak neni random hodnota

        stack->array[currentIndex + 1] = *tableItem;

        stack->topIndex++; // shifted to right
        stackPush(stack, *newTableItem);

        free(tableItem);
    } else {
        return false;
    }

    return true;
}

bool popMultiple(Stack *stack, int count) // neviem pomenovat funkcie 10iq
{
    for (int i = 0; i < count; i++) {
        if (!stackPop(stack))
            return false;
    }

    return true;
}


void popByIndex(Stack *stack, int index) {
    int finalIndex = stack->topIndex - index;

    popMultiple(stack, finalIndex);
}

Table_item getIndexValue(Stack *stack, int index) {
    return stack->array[index];
}

void stackPrint(Stack *stack) {
    fprintf(stderr, "[");
    for (int i = 0; i < stack->topIndex + 1; i++) {
        if (stack->array[i].id == E)
            fprintf(stderr, "E");
        else if (stack->array[i].id == plus)
            fprintf(stderr, "+");
        else if (stack->array[i].id == minus)
            fprintf(stderr, "-");
        else if (stack->array[i].id == mul)
            fprintf(stderr, "*");
        else if (stack->array[i].id == less_than)
            fprintf(stderr, "<");
        else if (stack->array[i].id == end)
            fprintf(stderr, "$");
        else if (stack->array[i].tableOperation == t_i)
            fprintf(stderr, "i");
        else if (stack->array[i].id == left_br)
            fprintf(stderr, "(");
        else if (stack->array[i].id == right_br)
            fprintf(stderr, ")");
        else if (stack->array[i].id == eq)
            fprintf(stderr, "==");
        else if (stack->array[i].id == not_eq)
            fprintf(stderr, "~=");
        else if (stack->array[i].id == more_than)
            fprintf(stderr, ">");
        else if (stack->array[i].id == less_than)
            fprintf(stderr, "<");
        else if (stack->array[i].id == more_eq_than)
            fprintf(stderr, ">=");
        else if (stack->array[i].id == less_eq_than)
            fprintf(stderr, "<=");
        else if (stack->array[i].id == concat)
            fprintf(stderr, "..");
        else if (stack->array[i].id == len)
            fprintf(stderr, "#");
        else if (stack->array[i].id == fdiv)
            fprintf(stderr, "/");
        else if (stack->array[i].id == idiv)
            fprintf(stderr, "//");
        else if (stack->array[i].id == nil)
            fprintf(stderr, "nil");
    }
    fprintf(stderr, "]\n");
}