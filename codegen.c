/**
 * Jan Knapovsky <xknapo05>
 */

#include "codegen-interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ACTIVE_REGS 5
#define STACK_PREALLOC_LEN 1

#define PRINT_DBG printf("BREAK\n");
#define OUTFILE ( inWhile ? odkladiste : stdout)
#define CURRENTPOOL ( inLocalContext ? localRegisterPool : globalRegisterPool)


typedef struct{
    char **nameArray;
    int len;
    int allocated;
}branchStack;

branchStack ifsStack = {.nameArray=NULL,.len=0,.allocated=0};
branchStack elsesStack = {.nameArray=NULL,.len=0,.allocated=0};
branchStack whilesStack = {.nameArray = NULL, .len=0, .allocated = 0};
char * preparingWhileName = NULL;


void pushBranchStack(branchStack * stack, char* name){
    if(stack->allocated == stack->len){
        stack->nameArray = realloc(stack->nameArray, (stack->allocated + STACK_PREALLOC_LEN)*sizeof(char*));
        if(!stack->nameArray){exit(99);}
        stack->allocated += STACK_PREALLOC_LEN;
    }
    stack->nameArray[stack->len++] = name;
    return;
}

char* popBranchStack(branchStack * stack){
    if(stack->len == 0){
        fprintf(stderr, "NEMAM ZADNY ZAPOCNUTY IF! SEMANTAK TY KOKOT JEDEN! GET READY FOR A SEGFAULT FUCKER");
        return NULL;
    }
    char * ret = stack->nameArray[--stack->len];
    return ret;
}




bool frameCreated = false;
unsigned int functionCtr = 0;

unsigned int inWhile = 0;
unsigned long long int numberOfTemps = 0;

unsigned long long int ifsCount = 0;
unsigned long long int whilesCount = 0;

size_t velilostOdkladiste = 0;
char *odlozenyText = NULL;
FILE *odkladiste = NULL;







var_desc * globalRegisterPool[MAX_ACTIVE_REGS];
var_desc * localRegisterPool[MAX_ACTIVE_REGS];
bool inLocalContext = false;

void incrementWhile(){
    if(inWhile == 0){
        odkladiste = open_memstream(&odlozenyText, &velilostOdkladiste);
        if(!odkladiste){exit(99);}

        printf("# begin predeclaration segment\n");
    }
    
    inWhile++;
}

void decrementWhile(){
    inWhile--;
    if(inWhile == 0){
        fclose(odkladiste);
        odkladiste = NULL;
        velilostOdkladiste = 0;
        printf("# end predeclaration segment\n%s", odlozenyText);
        free(odlozenyText);
        odlozenyText = NULL;
    }
}

void clearRegisterPoolLocal(){
    for(int i = 0; i <MAX_ACTIVE_REGS; i++){
        if(localRegisterPool[i]){
            free(localRegisterPool[i]->name);
            free(localRegisterPool[i]);
            localRegisterPool[i] = NULL;
        }
    }
}

void clearRegisterPoolGlobal(){
    for(int i = 0; i <MAX_ACTIVE_REGS; i++){
        if(globalRegisterPool[i]){
            free(globalRegisterPool[i]->name);
            free(globalRegisterPool[i]);
            globalRegisterPool[i] = NULL;
        }
    }
}

void addToPool(var_desc * reg, var_desc * pool[]){
    int i = 0;
    for(; i<MAX_ACTIVE_REGS && pool[i]; i++){}
    if(i == MAX_ACTIVE_REGS){
        free(reg->name);
        free(reg);
        return;
    }
    pool[i] = reg;
}

var_desc * getFreeRegister(var_desc * pool[]){
    int i = MAX_ACTIVE_REGS - 1;
    for(; i>= 0 && (!(pool[i])); i--){}
    if(i < 0){
        return NULL;
    }
    var_desc *ret = pool[i];
    pool[i] = NULL;
    return ret;
}

void freeVarDesc(var_desc ** tbDestroyed){
    if(!tbDestroyed){
        fprintf(stderr, "FUKCING BAD FREE. YOU TRYING TO DO A SEGFAULT BY FREEING A POINTER TO POINTER, THAT IS NOT POINTING TO A POINTER!");
        return;
    }

    if(!*tbDestroyed){
        return;
    }

    if((*tbDestroyed)->isRegister){
        addToPool(*tbDestroyed, CURRENTPOOL);
        *tbDestroyed = NULL;
    }

    if(*tbDestroyed){
        if((*tbDestroyed)->name){
            free((*tbDestroyed)->name);
        }
        free(*tbDestroyed);
        *tbDestroyed = NULL;
    }
}







void beginDocument(){
    printf("# - Begin document\n"
        ".IFJcode21\n"
        "CREATEFRAME\n"
        "PUSHFRAME\n"
        "CREATEFRAME\n"
    );
    PRINT_DBG

    for(int i = 0; i < MAX_ACTIVE_REGS; i++){
        globalRegisterPool[i] = NULL;
        localRegisterPool[i] = NULL;
    }
}

void endDocument(){
    if(ifsStack.nameArray){free(ifsStack.nameArray);}
    if(whilesStack.nameArray){free(whilesStack.nameArray);}
    if(elsesStack.nameArray){free(elsesStack.nameArray);}
    clearRegisterPoolGlobal();
    clearRegisterPoolLocal();

    printf("\nJUMP ___std_lib\n\n# - function tointeger(f : number)\nLABEL tointeger\n    PUSHFRAME\n    DEFVAR LF@*return000\n    JUMPIFEQ tointeger_ret_nil LF@f nil@nil\n\n        FLOAT2INT LF@*return000 LF@f\n        JUMP tointeger_return\n    \n    LABEL tointeger_ret_nil\n\n        MOVE LF@*return000 nil@nil\n    \n    LABEL tointeger_return\n\n    POPFRAME\n    RETURN\n\n\n# - function substr(s : string, i : number, j : number)\nLABEL substr\n    PUSHFRAME\n    DEFVAR LF@*return000\n\n    JUMPIFEQ substr_error_nil LF@s nil@nil\n    JUMPIFEQ substr_error_nil LF@i nil@nil\n    JUMPIFEQ substr_error_nil LF@j nil@nil\n\n    FLOAT2INT LF@i LF@i\n    FLOAT2INT LF@j LF@j\n\n    DEFVAR LF@slen\n\n    STRLEN LF@slen LF@s\n\n    DEFVAR LF@cond\n    GT LF@cond LF@i LF@j\n    JUMPIFEQ substr_return_empty LF@cond bool@true\n\n    GT LF@cond LF@j LF@slen\n    JUMPIFEQ substr_return_empty LF@cond bool@true\n\n    #TODO - IMPLEMENT!!!!\n    #fucking loops\n\n    MOVE LF@*return000 string@substr\\032is\\032not\\032implemented\n\n    JUMP substr_ret\n    LABEL substr_return_empty\n    MOVE LF@*return000 string@\n\n    LABEL substr_ret\n    POPFRAME\n    RETURN\n    LABEL substr_error_nil\n    EXIT int@8\n\n\n# - function chr(i : integer)\nLABEL chr\n    PUSHFRAME\n    DEFVAR LF@*return000\n\n    JUMPIFEQ chr_error_nil LF@i nil@nil\n\n    DEFVAR LF@cond1\n    DEFVAR LF@cond2\n    GT LF@cond1 LF@i int@255\n    LT LF@cond2 LF@i int@0\n    OR LF@cond1 LF@cond1 LF@cond2\n    JUMPIFEQ chr_ret_nil LF@cond1 bool@true\n\n    INT2CHAR LF@*return000 LF@i\n    \n    JUMP chr_ret\n\n    LABEL chr_ret_nil\n    MOVE LF@*return000 nil@nil\n\n    LABEL chr_ret\n\n    POPFRAME\n    RETURN\n    LABEL chr_error_nil\n    EXIT int@8\n\n\n\nLABEL ___std_lib\n");

}

void move_inst(var_desc * dest, var_desc * src){
    fprintf(OUTFILE, "MOVE LF@%s LF@%s\n", dest->name, src->name);
}

var_desc * getNewTempVar(){
    var_desc *ret = getFreeRegister(CURRENTPOOL);
    if(ret){
        return ret;
    }

    size_t velikost = 0;
    char *name = NULL;
    FILE *nameStream = open_memstream(&name, &velikost);
    if(!nameStream){exit(99);}

    fprintf(nameStream, "%%%s%%%llu", "temp", numberOfTemps++);
    fclose(nameStream);

    ret = declareVar(name);
    ret->isRegister = true;
    return ret;
}

var_desc * declareVar(char * name){
    printf("DEFVAR LF@%s\n", name);
    var_desc * ret = malloc(sizeof(var_desc));
    if(!ret){exit(99);}
    ret->name = name;
    ret->type = NONE;
    ret->isRegister = false;
    return ret;
}

var_desc * createNilVar(){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "MOVE LF@%s nil@nil\n", ret->name);

    ret->type = NIL;

    return ret;
}

var_desc * createIntVar( int val) {
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "MOVE LF@%s int@%d\n", ret->name, val);

    ret->type = INTEGER;

    return ret;
}

var_desc * createDblVar( double val){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "MOVE LF@%s float@%a\n", ret->name, val);

    ret->type = NUMBER;

    return ret;
}

var_desc * createStrVar( char *str){
    var_desc *ret = getNewTempVar();
    char * strEscapes = printStringAsEscapes(str);
    fprintf(OUTFILE, "MOVE LF@%s string@%s\n", ret->name, strEscapes);

    free(strEscapes);

    ret->type = STRING;
    return ret;
}

char * printStringAsEscapes(char * str){
    size_t velikost = 0;
    char *strEscapes = NULL;
    FILE *strStream = open_memstream(&strEscapes, &velikost);
    if(!strStream){exit(99);}
    
    for(int i = 0; i<= strlen(str)-1; i++){
        fprintf(strStream, "\\%03d", str[i]);
    }

    fclose(strStream);

    return strEscapes;
}

var_desc * add_inst(var_desc * a, var_desc *b){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "ADD LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = (a->type == b->type) ? a->type : NONE;

    return ret;
}

var_desc * sub_inst(var_desc * a, var_desc * b){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "SUB LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = (a->type == b->type) ? a->type : NONE;

    return ret;
}

var_desc * mul_inst(var_desc * a, var_desc * b){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "MUL LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = (a->type == b->type) ? a->type : NONE;

    return ret;
}

var_desc * div_inst(var_desc * a, var_desc * b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "DIV LF@%s LF@%s LF@%s\n", ret->name, a->name, b ->name);

    ret->type = (a->type == b->type && a->type == NUMBER) ? a->type : NONE; 

    return ret;
}

var_desc * idiv_inst(var_desc *a, var_desc *b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "IDIV LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = (a->type == INTEGER && a->type == b->type) ? INTEGER : NONE;

    return ret;
}

var_desc * lt_inst(var_desc * a, var_desc * b){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "LT LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);


    ret->type = BOOLE;

    return ret;
}

var_desc * gt_inst(var_desc * a, var_desc *b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "GT LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = BOOLE;

    return ret;
}

var_desc * eq_inst(var_desc * a, var_desc *b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "EQ LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = BOOLE;

    return ret;
}

var_desc * and_inst(var_desc * a, var_desc * b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "AND LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = BOOLE;

    return ret;
}

var_desc * or_inst(var_desc * a, var_desc * b){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "OR LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = BOOLE;

    return ret;
}

var_desc * not_inst(var_desc * a){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "NOT LF@%s LF@%s\n", ret->name, a->name);

    ret->type = BOOLE;
    return ret;
}




//IFy
char * createIfName(){
    size_t velikost = 0;
    char *ifName = NULL;
    FILE *NameStream = open_memstream(&ifName, &velikost);
    if(!NameStream){exit(99);}
    
    fprintf(NameStream, "$if$%.3llu", ifsCount++);

    fclose(NameStream);

    return ifName;
}

void beginIf(var_desc * condResult){
    char * name = createIfName();
    incrementWhile();

    fprintf(OUTFILE, "JUMPIFNEQ %s$else LF@%s bool@true\n", name, condResult->name);

    pushBranchStack(&ifsStack, name);

    return;
}

void beginElse(){
    char *ifName = popBranchStack(&ifsStack);
    
    fprintf(OUTFILE, "JUMP %s$endif\n", ifName);
    fprintf(OUTFILE, "LABEL %s$else\n", ifName);

    pushBranchStack(&elsesStack, ifName);

    return;
}

void endIf(){
    char *ifName = popBranchStack(&elsesStack);

    fprintf(OUTFILE, "LABEL %s$endif\n", ifName);

    free(ifName);

    decrementWhile();
}


var_desc * int2float_inst(var_desc *arg){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "INT2FLOAT LF@%s LF@%s\n", ret->name, arg->name);

    ret->type = NUMBER;
    return ret;
}

var_desc * float2int_inst(var_desc *arg){
    var_desc * ret = getNewTempVar();

    fprintf(OUTFILE, "FLOAT2INT LF@%s LF@%s\n", ret->name, arg->name);

    ret->type = INTEGER;
    return ret;
}

var_desc * int2char_inst(var_desc *arg){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "INT2CHAR LF@%s LF@%s\n", ret->name, arg->name);

    ret->type = STRING;
    return ret;
}


//WHILY

char * createWhileName(){
    size_t velikost = 0;
    char *whileName = NULL;
    FILE *NameStream = open_memstream(&whileName, &velikost);
    if(!NameStream){exit(99);}
    
    fprintf(NameStream, "$while$%.3llu", whilesCount++);

    fclose(NameStream);

    return whileName;
}

void announceWhile(){
    char * whileName = createWhileName();
    incrementWhile();
    
    fprintf(OUTFILE, "LABEL %s$begin\n", whileName);

    preparingWhileName = whileName;
    return;
}

void whileCondition(var_desc *condition){
    char * whileName = preparingWhileName;
    preparingWhileName = NULL;

    fprintf(OUTFILE, "JUMPIFNEQ %s$end LF@%s bool@true\n",whileName , condition->name);

    pushBranchStack(&whilesStack, whileName);
    return;
}

void whileEnd(){
    char * whileName = popBranchStack(&whilesStack);

    fprintf(OUTFILE, "JUMP %s$begin\n", whileName);
    fprintf(OUTFILE, "LABEL %s$end\n", whileName);

    free(whileName);

    decrementWhile();
}


//Volani funkci

void prepareArg(var_desc * src, const char * argName){
    if(!frameCreated){
        fprintf(OUTFILE, "\n\n#FUNCTION CALL BEGINNING HERE\nCREATEFRAME\n");
        frameCreated = true;
    }
    fprintf(OUTFILE, "DEFVAR TF@%s\n", argName);
    fprintf(OUTFILE, "MOVE TF@%s LF@%s\n", argName, src->name);
}

void beginFunction(const char * name){
    fprintf(OUTFILE, "\n\n# DECLARATION OF FUNCTION: %s\n", name);
    fprintf(OUTFILE, "JUMP _%u_end\n", functionCtr);
    fprintf(OUTFILE, "LABEL %s\n", name);
    fprintf(OUTFILE, "PUSHFRAME\n");
    frameCreated = false;
    inLocalContext = true;
    
}

var_desc * getArg(const char * name){
    var_desc *ret = malloc(sizeof(var_desc));
    ret->name = strdup(name);
    ret->type = NONE;
    ret->isRegister = false;
    return  ret;
}

void stageReturn(var_desc * src, int index){
    fprintf(OUTFILE, "DEFVAR LF@*return%.3d\n", index);
    fprintf(OUTFILE, "MOVE LF@*return%.3d LF@%s\n", index, src->name);
}

var_desc * getReturn(int index){
    var_desc *ret = getNewTempVar();
    fprintf(OUTFILE, "MOVE LF@%s TF@*return%.3d\n", ret->name, index);
    
    return ret;
}

void generateFCall(const char * name){
    if(!frameCreated){
        fprintf(OUTFILE, "\n\n#FUNCTION CALL BEGINNING HERE\nCREATEFRAME\n");
        frameCreated = true;
    }
    fprintf(OUTFILE, "\n\t#CALLING %s\nCALL %s\n", name, name);
    frameCreated = false;
    return;
}

void returnFunction(){
    fprintf(OUTFILE, "POPFRAME\nRETURN\n");
}

void endFunction(){
    returnFunction();
    fprintf(OUTFILE, "LABEL _%u_end\n", functionCtr++);
    inLocalContext = false;
    clearRegisterPoolLocal();
}

//Specialni instrukce

var_desc * read_inst(var_type type){
    var_desc * ret = getNewTempVar();
    
    fprintf(OUTFILE, "READ LF@%s %s\n", ret->name, (type == INTEGER ? "int" : (type == NUMBER ? "float" : "string" )));

    ret->type = type;

    return ret;    
}

void write_inst(var_desc *sym){
    fprintf(OUTFILE, "WRITE LF@%s\n", sym->name);
}


//Práce s řetězci

var_desc * concat_inst(var_desc* a, var_desc*b){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "CONCAT LF@%s LF@%s LF@%s\n", ret->name, a->name, b->name);

    ret->type = STRING;

    return ret;
}

var_desc * strlen_inst(var_desc *str){
    var_desc *ret = getNewTempVar();

    fprintf(OUTFILE, "STRLEN LF@%s LF@%s\n", ret->name, str->name);

    ret->type = INTEGER;

    return ret;
}