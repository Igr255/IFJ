/**
 * Jan Knapovsky <xknapo05>
 */

#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define IN_BUFFER_PREALLOC 40
#define OUTPUT_ACC_PREALLOC 5




//#########################################################
//Private (accessible only within this document)
//#########################################################

//Types
    //inputBuffer
    typedef struct strct_inputBuffer {       //struktura charakterizujici vstupni frontu
        char * data;             //to be allocated
        int length;
        int allocated;
        char * head;            //pointer into *data
        bool inputEnd;
    } inputBuffer_t;             

    //outputAccumulator
    typedef struct strct_outAcc{            //struktura charakterizujici vystupni buffer
        char * data;            //sem se budou stradat postupne znaky tokenu (alokovane misto)
        int length;             //delka dosavad nactenych dat
        int allocated;          //prealokovana delka
        char *head;             //pointer do data, ukazuje kam se bude zapisovat
    }outAcc_t;

    //fsmState
    typedef enum{                           //stavy fsm
        S_BEGIN,
        S_ID_TOKEN_F,
        S_INT_NUM_TOKEN_F,
        S_EXPECT_DECIMAL,
        S_DECIMAL_NUM_TOKEN_F,
        S_EXPECT_EXPONENT,
        S_GOT_EXP_SIGN,
        S_EXP_NUMBER_TOKEN_F,
        S_STRING_READING,
        S_EXPECT_ESCAPE,
        S_ASCII_VAL1,
        S_ASCII_VAL2,
        S_ASCII_VAL3,
        S_STRING_LITERAL_F,
        S_MINUS_OPERATOR_F,
        S_LINE_COMMENT_F,
        S_MBY_BLOCK_COMMENT,
        S_BLOCK_COMMENT,
        S_EXPECT_END_BLOCK_COMMENT,
        S_END_COMMENT_F,
        S_OPERATOR_TOKEN_F,
        S_BRACKET_TOKEN_F,
        S_DIVIDE_OPERATOR_F,
        S_SCMP_OPERATOR_F,
        S_EXPECT_NIL_CMP,
        S_EXPECT_CONCAT,
        S_COLON_TOKEN_F,
        S_EOF_F
    }fsmState_t;

    fsmState_t glfinalStates[] = {                        //koncove stavy fsm
        S_ID_TOKEN_F,//
        S_INT_NUM_TOKEN_F,//
        S_DECIMAL_NUM_TOKEN_F,//
        S_EXP_NUMBER_TOKEN_F,//
        S_STRING_LITERAL_F,//
        S_MINUS_OPERATOR_F,//
        S_LINE_COMMENT_F,//
        S_END_COMMENT_F,//
        S_OPERATOR_TOKEN_F,//
        S_BRACKET_TOKEN_F, //
        S_DIVIDE_OPERATOR_F,//
        S_SCMP_OPERATOR_F, //
        S_COLON_TOKEN_F,
        S_EOF_F
    };


//GLOBAL VARS##############################################
//jsem prase ja vim, ale je to lepsi nez se srat s pameti

inputBuffer_t * glInput = NULL;
outAcc_t * glOutAcc = NULL;

unsigned int _lineCounter = 0;


//Functions

//outputAccumulator methods

    //output accumulator constructor
    outAcc_t * initOutAcc(void)
    {
        outAcc_t *ret = malloc(sizeof(outAcc_t));
        if (!ret)
        {
            exit(99);
        }
        
        ret->data = NULL;
        ret->length = 0;
        ret->allocated = 0;
        ret->head = NULL;

        return ret;
    }

    //output accumulator destructor
    void destOutAcc(outAcc_t *acc)
    {
        free(acc->data);
        free(acc);
        acc = NULL;
    }

    //Resetuje output accumulator
    void resetOutAcc(outAcc_t *acc) 
    {
        free(acc->data);
        acc->data = NULL;
        acc->length = 0;
        acc->allocated = 0;
        acc->head = NULL;
    }


    //prida znak 'x' do akumulatoru 'acc'
    bool addToOutAcc(char x, outAcc_t *acc)
    {
        if(!acc->data){//osetrit prazdny akumulator
            acc->data = calloc(OUTPUT_ACC_PREALLOC, sizeof(char));
            if(!acc->data){
                exit(99);
            }

            acc->head = acc->data;
            acc->allocated += OUTPUT_ACC_PREALLOC;
        }

        *acc->head = x; //pridat znak za hlavu
        acc->head++;
        acc->length++;
        
        if(acc->length == acc->allocated - 1){
            void *allocNew = calloc(OUTPUT_ACC_PREALLOC + acc->allocated, sizeof(char));
            if(!allocNew){
                exit(99);
            }

            memcpy(allocNew,acc->data,acc->allocated);
            free(acc->data);
            acc->data = allocNew;
            acc->allocated += OUTPUT_ACC_PREALLOC;
            acc->head = &acc->data[acc->length];
        }

        return true;
    }

    //vrati nove !NAALOKOVANY! string, ktery odpovida obsahu akumulatoru 'acc' (pak free(string) a destroy(acc) musis udelat, jinak memleaks)
    char * getStringAcc(outAcc_t *acc)
    {
        char *ret = calloc(acc->length+1,sizeof(char));
        if(!ret){
            exit(99);
        }
        
        memcpy(ret,acc->data,acc->length);

        return ret;
    }

//InputBuffer methods

    //inputBuffer_t constructor
    inputBuffer_t * initInBuffer(void)          
    {
        inputBuffer_t *ret = malloc(sizeof(inputBuffer_t));
        if (!ret)
        {
            exit(99);
        }

        ret->data = NULL;
        ret->length = 0;
        ret->allocated = 0;
        ret->head = NULL;
        ret->inputEnd = false;

        return ret;
    }

    //inputBuffer_t destructor
    void destInBuffer(inputBuffer_t *buff)      
    {
        free(buff->data);
        free(buff);
        buff = NULL;//<- nic nedela :pepeLa:
    }

    //naplni input buffer dalsim radkem ze stdin
    bool fillInBuffer(inputBuffer_t *buff)      
    {  
        if (buff->inputEnd){    //pokud vstup uz skoncil, tak odmitni doplnit
            return false;
        }

        if (buff->data){    //pokud uz tam neco je, tak to vyhod
            free(buff->data);
        }

        buff->data = NULL;   //zresetuj strukturu (jsem linej :)
        buff->length = 0;
        buff->allocated = 0;
        buff->head = NULL;
        buff->inputEnd = false;

        buff->data = calloc(IN_BUFFER_PREALLOC, sizeof(char));
        if(!buff->data){
            exit(99);
        }

        buff->allocated += IN_BUFFER_PREALLOC;
        buff->head = buff->data;

        for(int x = getchar(); x != '\n'; x = getchar())
        {
            if(x==EOF){
                buff->inputEnd = true;
                break;
            }
            *(buff->head) = x;
            buff->head++;
            buff->length++;

            if(buff->length == buff->allocated - 1)
            {
                void *allocNew = calloc(IN_BUFFER_PREALLOC + buff->allocated, sizeof(char));
                if (!allocNew)
                {
                    exit(99);
                }
                memcpy(allocNew, buff->data, buff->allocated);
                free(buff->data);
                buff->data = allocNew;
                buff->allocated += IN_BUFFER_PREALLOC;
                buff->head = &buff->data[buff->length];
            }
        }
        buff->head = buff->data;

        _lineCounter++;  //inkrementuje globalni line counter (pouzitelny pro generovani erroru)

        return true;

    }

    //vyprazdni vstupni buffer (zahodi radek)
    void flushInBuffer(inputBuffer_t *buff)
    {
        free(buff->data);
        buff->data = NULL;
        buff->length = 0;
        buff->allocated = 0;
        buff->head = NULL;
    }

    //vrati prvni znak z fronty
    char getInBufferHead(inputBuffer_t *buff)   
    {

        if (buff->data == NULL)//osetrit prazdnou frontu
        {
            if(!fillInBuffer(buff)){
                return '\0';
            }
        }

        char ret = *buff->head;
        if(ret == '\0'){//misto '\n' vratit ' '. ('\n' odpovida konci retezce, tj. '\0') viz fillInBuffer()
            if(buff->inputEnd){ //pokud EOF, vrati '\0'
                return '\0';
            }
            return ' ';
        }

        return ret;
    }

    //vrati a vyhodi prvni znak fronty
    char popInBuffer(inputBuffer_t *buff)       
    {

        if (buff->data == NULL)//osetrit prazdnou frontu
        {
            if(!fillInBuffer(buff)){
                return '\0';
            }
        }

        char ret = *buff->head;
        if(ret == '\0'){//osetrit konec radku/odradkovani nahradit ' '.
            if(!fillInBuffer(buff)){//pokud se nepovede doplnit vratit konec souboru, ne ' '.
                return '\0';
            }

            ret =  ' ';
            return ret;
        }

        buff->head++;
        return ret;
    }

//usefull functions
    //returns true if character is in string
    bool isCharInSet(char x, char *set)
    {
        bool ret = false;
        for(int i = 0; set[i]; i++){
            if(set[i] == x){
                ret = true;
            }
        }

        return ret;
    }

    //returns true if input is keyword
    bool isKeyword(char * input){
        char *keywords[] = {"do", "else", "end", "function", "global", "if", "integer", "local", "nil", "number", "require", "return", "string", "then", "while", NULL};

        for(int i = 0; keywords[i]; i++){
            if(!strcmp(input,keywords[i])){
                return true;
            }
        }

        return false;
    }

    //parse string escape squences
    bool parseEscapes(char ** string){
        char * parsedString = calloc(1, strlen(*string));
        if(!parsedString){
            exit(99);
        }

        char * writeHead = parsedString;
        char * readHead = *string;

        for(int i = 0; i<strlen(*string) && *readHead; i++){
            if(*readHead == '\\'){
                readHead++;
                if(*readHead == '"'){
                    *writeHead = '"';
                    writeHead++;
                    readHead++;
                }else if(*readHead == 'n'){
                    *writeHead = '\n';
                    writeHead++;
                    readHead++;
                }else if(*readHead == 't'){
                    *writeHead = '\t';
                    writeHead++;
                    readHead++;
                }else if(*readHead == '\\'){
                    *writeHead = '\\';
                    writeHead++;
                    readHead++;
                }else if((readHead[0] >= '0' && readHead[0] <= '9') && (readHead[1] >= '0' && readHead[1] <= '9') && (readHead[2] >= '0' && readHead[2] <='9')){
                    int asciiID = 0;
                    asciiID += (readHead[0] - '0') * 100;
                    asciiID += (readHead[1] - '0') * 10;
                    asciiID += (readHead[2] - '0');
                    readHead += 3;

                    if( !(asciiID > 0 && asciiID < 256) ){
                        free(parsedString);
                        return false;
                    }

                    *writeHead = (char)asciiID;
                    writeHead++;

                }else{
                    return false;
                }
            }else if(*readHead == '"'){
                readHead++;
            }else{
                *writeHead = *readHead;
                writeHead++;
                readHead++;
            }
        }
        
        free(*string);

        *string = parsedString;

        return true;

    }

    //show error on line
    void showError(){
        int offset = (int)(glInput->head - glInput->data);
        fprintf(stderr, "\n%3d:\t%s\n", _lineCounter, glInput->data);
        fprintf(stderr, "    \t %*c\n\n", offset, '^');
    }
//alliases

    void fsmAcceptInput()
    {
        addToOutAcc(popInBuffer(glInput), glOutAcc);
    }

//#########################################################
//Public (accessible through tokenizer.h)
//#########################################################

void destroyToken(token_t *token){
    if(token->strValue){
        free(token->strValue);
    }
    free(token);
    return;
}

token_t * getNextToken()
{
    if(!glInput){
        glInput = initInBuffer();
    }
    if(!glOutAcc){
        glOutAcc = initOutAcc();
    }

    tokenType_t typTokenu = ERR_TOKEN;
    bool fsmFinished = false;
    fsmState_t state = S_BEGIN;

    resetOutAcc(glOutAcc);

    (void)typTokenu;


    while(!fsmFinished){
        char inChar = getInBufferHead(glInput);
        
        switch (state){           //see documentation/fsm/scheme.*
            case S_BEGIN:                                       //STATE BEGIN ################################################
                if(isCharInSet(inChar, "0123456789")){          //edge 1
                    fsmAcceptInput();
                    state = S_INT_NUM_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar," \t")){                    //edge 2
                    popInBuffer(glInput);
                    break;
                }
                if(isCharInSet(inChar,"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")){    //edge 3
                    fsmAcceptInput();
                    state = S_ID_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar,"-")){                    //edge 4
                    fsmAcceptInput();
                    state = S_MINUS_OPERATOR_F;
                    break;
                }
                if(isCharInSet(inChar,"+#*")){                  //edge 5
                    fsmAcceptInput();
                    state = S_OPERATOR_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar,"()")){                   //edge 6
                    fsmAcceptInput();
                    state = S_BRACKET_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar, "/")){                   //edge 7
                    fsmAcceptInput();
                    state = S_DIVIDE_OPERATOR_F;
                    break;
                }
                if(isCharInSet(inChar, "<>=")){                 //edge 8
                    fsmAcceptInput();
                    state = S_SCMP_OPERATOR_F;
                    break;
                }
                if(isCharInSet(inChar,"~")){                    //edge 9
                    fsmAcceptInput();
                    state = S_EXPECT_NIL_CMP;
                    break;
                }
                if(isCharInSet(inChar,".")){                    //edge 10
                    fsmAcceptInput();
                    state = S_EXPECT_CONCAT;
                    break;
                }
                if(isCharInSet(inChar,"\"")){                   //edge 11
                    fsmAcceptInput();
                    state = S_STRING_READING;
                    break;
                }
                if(isCharInSet(inChar,":,")){
                    fsmAcceptInput();
                    state = S_COLON_TOKEN_F;
                    break;
                }
                if(inChar == '\0'){
                    state = S_EOF_F;
                    break;
                }
                fsmFinished = true;
                break;
                //######################################################################END STATE BEGIN
            
            case S_ID_TOKEN_F:                                  //STATE ID_TOKEN ######################
                if(isCharInSet(inChar,"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")){          //edge 12
                    fsmAcceptInput();
                    state = S_ID_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //##################################################################END STATE ID_TOKEN

            case S_INT_NUM_TOKEN_F:                             //STATE INT_NUM_TOKEN#################
                if(isCharInSet(inChar, "0123456789")){          //edge 13
                    fsmAcceptInput();
                    state = S_INT_NUM_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar,".")){                    //edge 14
                    fsmAcceptInput();
                    state = S_EXPECT_DECIMAL;
                    break;
                }
                if(isCharInSet(inChar,"Ee")){                   //edge 15
                    fsmAcceptInput();
                    state = S_EXPECT_EXPONENT;
                    break;
                }
                fsmFinished = true;
                break;
                //############################################################END STATE INT_NUM_TOKEN

            case S_EXPECT_DECIMAL:                              //STATE S_EXPECT_DECIMAL#############
                if(isCharInSet(inChar, "0123456789")){          //edge 16
                    fsmAcceptInput();
                    state = S_DECIMAL_NUM_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //###########################################################END STATE EXPECT_DECIMAL

            case S_DECIMAL_NUM_TOKEN_F:                         //STATE DECIMAL_NUM_TOKEN ###########
                if(isCharInSet(inChar,"Ee")){                   //edge 17
                    fsmAcceptInput();
                    state = S_EXPECT_EXPONENT;
                    break;
                }
                if(isCharInSet(inChar, "0123456789")){          //edge ?? fuckup
                    fsmAcceptInput();
                    state = S_DECIMAL_NUM_TOKEN_F;
                    break;
                }

                fsmFinished = true;
                break;
                //########################################################END STATE DECIMAL_NUM_TOKEN

            case S_EXPECT_EXPONENT:                             //STATE EXPECT_EXPONENT##############
                if(isCharInSet(inChar,"0123456789")){           //edge 18
                    fsmAcceptInput();
                    state = S_EXP_NUMBER_TOKEN_F;
                    break;
                }
                if(isCharInSet(inChar,"+-")){                   //edge 19
                    fsmAcceptInput();
                    state = S_GOT_EXP_SIGN;
                    break;
                }
                fsmFinished = true;
                break;
                //######################################################### END STATE EXPECT_EXPONENT

            case S_GOT_EXP_SIGN:                                //STATE GOT_EXP_SIGN#################
                if(isCharInSet(inChar, "0123456789")){          //edge 20
                    fsmAcceptInput();
                    state = S_EXP_NUMBER_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //############################################################ END STATE GOT_EXP_SIGN

            case S_EXP_NUMBER_TOKEN_F:                             //STATE EXP_NUMBER_TOKEN #########
                if(isCharInSet(inChar,"0123456789")){           //edge 21
                    fsmAcceptInput();
                    state = S_EXP_NUMBER_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //############################################################## END EXP_NUMBER_TOKEN

            case S_STRING_READING:                                 //STATE STRING READING############
                if(isCharInSet(inChar, "\"")){                  //edge 23
                    fsmAcceptInput();
                    state = S_STRING_LITERAL_F;
                    break;
                }
                if(isCharInSet(inChar,"\\")){                   //edge 24
                    fsmAcceptInput();
                    state = S_EXPECT_ESCAPE;
                    break;
                }
                fsmAcceptInput();                               //edge 25
                break;
                //################################################################ END STRING READING

            case S_EXPECT_ESCAPE:                             //STATE EXPECT ESCAPE #################
                if(isCharInSet(inChar, "012")){                 //edge 26
                    fsmAcceptInput();
                    state = S_ASCII_VAL1;
                    break;
                }
                if(isCharInSet(inChar,"\"nt\\")){                   //edge 27
                    fsmAcceptInput();
                    state = S_STRING_READING;
                    break;
                }
                fsmFinished = true;
                break;
                //###############################################################END EXPECT ESCAPE

            case S_ASCII_VAL1:                             //STATE ASCII VAL 1
                if(isCharInSet(inChar,"0123456789")){                   //edge 28
                    fsmAcceptInput();
                    state = S_ASCII_VAL2;
                    break;
                }
                fsmFinished = true;
                break;
                //######################################################## END ASCII VAL 1

            case S_ASCII_VAL2:                             //STATE ASCII val 2
                if(isCharInSet(inChar,"0123456789")){                   //edge 29
                    fsmAcceptInput();
                    state = S_STRING_READING;
                    break;
                }
                fsmFinished = true;
                break;
                //####################################################### END ASCII VAL 2

            case S_STRING_LITERAL_F:                             //STATE STRING LITERAL
                fsmFinished = true;
                break;
                //################################################# END STRING LITERAL

            case S_EXPECT_CONCAT:                             //STATE EXP CONCAT
                if(isCharInSet(inChar,".")){                   //edge 30
                    fsmAcceptInput();
                    state = S_OPERATOR_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //################################################ END EXP CONCAT
            
            case S_EXPECT_NIL_CMP:                             //STATE EXPECT NIL CMP
                if(isCharInSet(inChar,"=")) {                   //edge 31
                    fsmAcceptInput();
                    state = S_OPERATOR_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //###############################################  END EXPECT NIL CMP

            case S_SCMP_OPERATOR_F:                             //STATE SCMP OPERATOR
                if(isCharInSet(inChar,"=")){                   //edge 32
                    fsmAcceptInput();
                    state = S_OPERATOR_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //############################################## END SCMP OPERATOR

            case S_DIVIDE_OPERATOR_F:                             //STATE DIVIDE OPERATOR
                if(isCharInSet(inChar,"/")){                   //edge 33
                    fsmAcceptInput();
                    state = S_OPERATOR_TOKEN_F;
                    break;
                }
                fsmFinished = true;
                break;
                //############################################## END DIVIDE OPERATOR

            case S_OPERATOR_TOKEN_F:                             //STATE OPERATOR TOKEN
                fsmFinished = true;
                break;
                //############################################## END OPERATOR TOKEN

            case S_BRACKET_TOKEN_F:                             //STATE BRACKET TOKEN
                fsmFinished = true;
                break;
                //############################################## END BRACKET TOKEN

            case S_COLON_TOKEN_F:                             //STATE COLON TOKEN
                fsmFinished = true;
                break;
                //############################################## END COLON TOKEN

            case S_MINUS_OPERATOR_F:                             //STATE MINUS OPERATOR
                if(isCharInSet(inChar,"-")){                    //edge 34
                    fsmAcceptInput();
                    state = S_LINE_COMMENT_F;
                    break;
                }
                fsmFinished = true;
                break;
                //#############################################  END MINUS OPERATOR
            
            case S_LINE_COMMENT_F:                             //STATE LINE COMMENT
                if(isCharInSet(inChar,"[")){        //edge 35
                    fsmAcceptInput();
                    state = S_MBY_BLOCK_COMMENT;
                    break;
                }                                   // drop line and restart
                flushInBuffer(glInput);
                resetOutAcc(glOutAcc);
                state = S_BEGIN;
                break;
                //############################################ END LINE COMMENT

            case S_MBY_BLOCK_COMMENT:                             //STATE MBY BLOCK COMMENT
                if(isCharInSet(inChar,"[")){                   //edge 36
                    fsmAcceptInput();
                    state = S_BLOCK_COMMENT;
                    break;
                }
                flushInBuffer(glInput);                        // drop line and continue
                resetOutAcc(glOutAcc);
                state = S_BEGIN;
                break;
                //############################################ END MBY BLOCK COMMENT

            case S_BLOCK_COMMENT:                             //STATE BLOCK COMMENT
                if(isCharInSet(inChar,"]")){                   //edge 37
                    fsmAcceptInput();
                    state = S_EXPECT_END_BLOCK_COMMENT;
                    break;                                     
                }   
                fsmAcceptInput();                               //edge 38
                break;
                //########################################### END BLOCK COMMENT
                
            case S_EXPECT_END_BLOCK_COMMENT:                             //STATE EXPECT END BLOCK COMMENT
                if(isCharInSet(inChar,"]")){                   //edge 39
                    fsmAcceptInput();
                    state = S_END_COMMENT_F;
                    break;
                }
                state = S_BLOCK_COMMENT;                        //edge 40
                fsmAcceptInput();
                break;
                //#################################################### END EXPECT END BLOCK COMMENT 

            case S_END_COMMENT_F:                             //STATE END BLOCK S_BLOCK_COMMENT
                resetOutAcc(glOutAcc);
                state = S_BEGIN;
                break;
                //################################################## END END BLOCK COMMENT

            case S_EOF_F:                             //STATE EOF
                fsmFinished = true;
                break;
                //##################################################### END EOF

            default:
                fsmFinished = true;
                break;
        }


    }
    
    char *stringValue = getStringAcc(glOutAcc);


    switch (state)
    {
    
    case S_ID_TOKEN_F:
        if(isKeyword(stringValue)){
            typTokenu = KEYW_TOKEN;
            break;
        }
        typTokenu = ID_TOKEN;
        break;

    case S_INT_NUM_TOKEN_F:                            
        typTokenu = INTNUM_TOKEN;
        break;
    
    case S_DECIMAL_NUM_TOKEN_F:
        typTokenu = FLOATNUM_TOKEN;
        break;
    
    case S_EXP_NUMBER_TOKEN_F:
        typTokenu = FLOATNUM_TOKEN;
        break;
    
    case S_STRING_LITERAL_F:
        typTokenu = STRING_TOKEN;
        break;

    case S_MINUS_OPERATOR_F:
        typTokenu = OP_TOKEN;
        break;
    
    case S_OPERATOR_TOKEN_F:
        typTokenu = OP_TOKEN;
        break;

    case S_BRACKET_TOKEN_F:
        typTokenu = CTRLCHAR_TOKEN;
        break;

    case S_DIVIDE_OPERATOR_F:
        typTokenu = OP_TOKEN;
        break;

    case S_SCMP_OPERATOR_F:
        typTokenu = OP_TOKEN;
        break;

    case S_COLON_TOKEN_F:
        typTokenu = CTRLCHAR_TOKEN;
        break;

    case S_EOF_F:
        typTokenu = EOF_TOKEN;
        break;

    default:
        typTokenu = ERR_TOKEN;
        fprintf(stderr, "Invalid token beginning with \"%s\" on line :%d\n", stringValue, _lineCounter);
        showError();
        break;
    }


    token_t *ret = malloc(sizeof(token_t));
    if(!ret){
        exit(99);
    }

    ret->strValue = stringValue;
    ret->type = typTokenu;
    ret->dbValid = false;
    ret->dbValue = 0;
    ret->intValid = false;
    ret->intValue= 0;

    if(ret->type == STRING_TOKEN){
        if(!parseEscapes(&(ret->strValue))){
            ret->type = ERR_TOKEN;
            fprintf(stderr,"Invalid escape sequence in \"%s\", on line:%d\n", ret->strValue, _lineCounter);
            showError();
        }
    }

    if(ret->type == INTNUM_TOKEN){
        ret->intValue = strtol(ret->strValue, NULL, 10);
        ret->intValid = true;
    }
    if(ret->type == FLOATNUM_TOKEN){
        ret->dbValue = strtod(ret->strValue, NULL);
        ret->dbValid = true;
    }

    

    
    return ret;
}





void tokenizerExit(){
    destInBuffer(glInput);
    glInput = NULL;
    destOutAcc(glOutAcc);
    glOutAcc= NULL;
}

int testF(int a, int b)
{
    

    char *str = malloc(500);

    int x, i = 0;


    while((x = getchar()) !=EOF){
        str[i] = x;
        i++;
    }
    

    bool ques = parseEscapes(&str);

    (void)ques;

    return 1;
}

