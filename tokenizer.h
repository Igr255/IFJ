/**
 * Jan Knapovsky <xknapo05>
 */

#include <stdbool.h>

#ifndef __Tokenizer_knp
#define __Tokenizer_knp


typedef enum{
    ERR_TOKEN,              //chybný token na vstupu
    KEYW_TOKEN,             //klicove slovo
    ID_TOKEN,               //identifikator
    INTNUM_TOKEN,           //integer konstanta
    FLOATNUM_TOKEN,         //Floar konstanta
    STRING_TOKEN,           //String konstatna
    OP_TOKEN,               //operator
    CTRLCHAR_TOKEN,         //zavorka / dvojtecka
    EOF_TOKEN               //konec vstupu
} tokenType_t;


typedef struct s_token_t{   //struktura tokenu
    char *strValue;         //hodnota ve tvaru string
    long int intValue;      //pokud intValid == true, pak toto je hodnota prevedena na int; jinak ndef
    double dbValue;         //pokud dbValid == true, pak toto je hodnota prevedena na double; jinak ndef
    bool dbValid;           //viz dbValue
    bool intValid;          //viz intValue
    tokenType_t type;       //typ tokenu z tokenType_t
} token_t;


int testF(int a, int b);

token_t * getNextToken();
void destroyToken(token_t *token);  //destruktor tokenu
void tokenizerExit();       //destruktor tokenizeru (vycisti globalni promenne)      



#endif