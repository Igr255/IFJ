/**
 * Jan Knapovsky <xknapo05>
 */

#include <stdbool.h>

#ifndef __Codegen_knp
#define __Codegen_knp

#define FRAME2STRING(frame) ( (frame) == LOCAL_FRAME ? "LF" : ( (frame) == TEMPORARY_FRAME ? "TF" : "GF" ) )

typedef enum{
    LOCAL_FRAME,
    TEMPORARY_FRAME,
    GLOBAL_FRAME
}frame_type;

typedef enum{
    NONE,
    STRING,
    INTEGER,
    NUMBER,
    BOOLE,
    NIL
}var_type;


typedef struct {
    char * name; 
    var_type type; ///< Typ ktery codegen predpoklada ze by promenna mohla mit, (nekontroluje ho!)
    bool isRegister;
}var_desc;

//pomocne => %
//skoky => $ - podminky a cykly
//         $$ - funkce ?

/**
 * @brief var_desc destructor
 * 
 */
void freeVarDesc(var_desc **);


void beginDocument();
void endDocument();

var_desc * getNewTempVar();

var_desc * createIntVar( int);
var_desc * createDblVar( double);
var_desc * createStrVar( char *);
var_desc * createNilVar();

/**
 * @brief Deklaruje proměnnou v lokálním rámci
 * 
 * @param name NAALOKOVANÝ řetězec se jménem proměnné (PŘEBÍRÁ VLASTNICTVÍ PAMĚTI, NEUVOLŇOVAT)
 * @return var_desc* Deskriptor výsledné proměnné
 */
var_desc * declareVar(char * name);

char * printStringAsEscapes(char *);

/**
 * @brief Zkopíruje obsah src do dest
 * 
 * @param dest 
 * @param src 
 */
void move_inst(var_desc * dest, var_desc * src);

//Instrukce pro aritmetiku

/**
 * @brief Sečte dvě proměnné (neošetřuje typy)
 * 
 * @return var_desc* Pomocná proměnná s výsledkem
 */
var_desc * add_inst(var_desc * , var_desc *);

/**
 * @brief Odečte a - b
 * 
 * @param a Proměnná a
 * @param b Proměnná b
 * @return var_desc* Pomocná proměnná s výsledkem (a-b)
 */
var_desc * sub_inst(var_desc * a, var_desc * b);

/**
 * @brief Vynásobí a*b
 * 
 * @param a 
 * @param b 
 * @return var_desc* Pomocná proměnná s výslekdem (a*b)
 */
var_desc * mul_inst(var_desc * a, var_desc * b);

/**
 * @brief Vydělí a/b
 * 
 * @param a Musí být float!
 * @param b Musí být float!
 * @return var_desc* Pomocná proměnná s výsledkem (a/b) (FLOAT!!!)
 */
var_desc * div_inst(var_desc * a, var_desc * b);

/**
 * @brief Celočíselně vydělí (a/b)
 * 
 * @param a Musí výt INT!
 * @param b Musí výt INT!
 * @return var_desc* Pomocná proměnná s výsledkem celočíselného dělení (a/b) INT!
 */
var_desc * idiv_inst(var_desc *a, var_desc *b);

//Logicke instrukce

/**
 * @brief Porovna a<b
 * 
 * @param a 
 * @param b 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (a<b)
 */
var_desc * lt_inst(var_desc * a, var_desc * b);

/**
 * @brief Porovna a>b
 * 
 * @param a 
 * @param b 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (a>b)
 */
var_desc * gt_inst(var_desc * a, var_desc *b);

/**
 * @brief Porovna a==b
 * 
 * @param a 
 * @param b 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (a=b)
 */
var_desc * eq_inst(var_desc * a, var_desc *b);

/**
 * @brief Logický and nad dvěma booly
 * 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (a&&b)
 */
var_desc * and_inst(var_desc * , var_desc *);

/**
 * @brief Logický or nad dvěma booly
 * 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (a||b)
 */
var_desc * or_inst(var_desc *, var_desc *);

/**
 * @brief Logický not nad jedním boolem
 * 
 * @return var_desc* Pomocna promenna typu bool s vysledkem (!a)
 */
var_desc * not_inst(var_desc *);


//Instrukce pro rizeni toku programu

/**
 * @brief Začne if statement
 * 
 * @param condResult Promenna s vysledkem podminky if-u (pokud true, provede se vetev true, pokud false, provede se vetev else)
 * 
 */
void beginIf(var_desc * condResult);

/**
 * @brief Ukonci true cast if statementu a zacne jeho else cast
 * 
 * 
 */
void beginElse();

/**
 * @brief Ukonci else cast if statementu a ukonci if statement
 * 
 *
 */
void endIf();

/**
 * @brief Ohlásí začátek while cyklu codegenu. NUTNO ZAVOLAT PRED VYHODNOCOVANIM PODMINKY!
 * 
 */
void announceWhile();

/**
 * @brief Podminka pro skok a ukonceni while cyklu
 * 
 * @param condition Pokud true, pokracuj v cyklu; Pokud false ukonci cyklus
 */
void whileCondition(var_desc *condition);

/**
 * @brief Ukonci while cyklus
 * 
 */
void whileEnd();





//Conversion instrukce

/**
 * @brief Vrátí hodnotu integeru jako number (v novem registru)
 * 
 * @return var_desc* Novy registr s hodnotou ( (float)arg )
 */
var_desc * int2float_inst(var_desc *);

/**
 * @brief Vrati hodnotu number-u jako integer (osekne desetinou cast)
 * 
 * @return var_desc* Novy registr s hodnotou ( (int)arg )
 */
var_desc * float2int_inst(var_desc *);

/**
 * @brief Vrátí konverzi int na char
 * 
 * @return var_desc* 
 */
var_desc * int2char_inst(var_desc *);


//Volání funkcí
//##########################################

/**
 * @brief Připraví argument pro funkci. Nutno udělat pro všechny argumenty před zavolanim funkce.
 * 
 * @param src Zdroj hodnoty argumentu
 * @param argName Jmeno argumentu, ktere vystupuje ve funkci
 */
void prepareArg(var_desc * src, const char * argName);

/**
 * @brief Vytvoří funkci
 * 
 * @param name Jméno funkce, pomocí kterého bude moct být volána
 */
void beginFunction(const char * name);

/**
 * @brief Vrátí vardescriptor argumentu funkce, nutno použít pro použití argumentů
 * 
 * @param name jméno argumentu, které bude použito i ve funkci prepareArg()
 * @return var_desc* 
 */
var_desc * getArg(const char * name);

//funkce overjump _

/**
 * @brief Připraví hodnotu k vrácení
 * 
 * @param src Zdroj hodnoty
 * @param index Index, pro vracení více než 1 hodnoty
 */
void stageReturn(var_desc * src, int index);

/**
 * @brief Provede návrat z funkce. Před tím je potřeba připravit vsechny navratove hodnoty
 * 
 */
void returnFunction();

/**
 * @brief Ukonci funkci
 * 
 */
void endFunction();

/**
 * @brief Získá hodnotu returnValue funkce která se právě vrátila z volání
 * 
 * @param index Index hodnoty, který byl zadán i do stageReturn();
 * @return var_desc* Nový registr s návratovou hodnotou
 */
var_desc * getReturn(int index);

/**
 * @brief Provede zavolání funkce. Před tím je potřeba připravit všechny argumenty, potom je potřeba ihned získat všechny návratové hodnoty.
 * 
 * @param name 
 */
void generateFCall(const char * name);


//Specialni instrukce

/**
 * @brief Načte ze stdin hodnotu typu 'type'
 * 
 * @param type typ načtené hodnoty
 * @return var_desc* registr s načtenou hodnotou
 */
var_desc * read_inst(var_type type);


/**
 * @brief Vypíše 'sym' na stdout
 * 
 * @param sym Registr/proměnná k vypsání.
 */
void write_inst(var_desc *sym);


//Práce se stringy

/**
 * @brief Zkonkatenuje stringy a a b
 * 
 * @param a 
 * @param b 
 * @return var_desc* Nový registr obsahující zkonkatenovaný string
 */
var_desc * concat_inst(var_desc* a, var_desc*b);

/**
 * @brief Vrátí délku řetězce
 * 
 * @return var_desc* Registr s délkou řetězce (int)
 */
var_desc * strlen_inst(var_desc *);

#endif