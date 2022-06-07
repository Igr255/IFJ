/**
 * Adam Ondrousek <xondro09>
 * Filip Karasek <xkaras39>
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "synCheck.h"
#include "codegen-interface.h"
#include "expressionParse.h"

void generate_ifj() {
    beginDocument();
}
void endifj() {
    endDocument();
}


/*
funkce variable_check
zkontroluje jestli jsou var nebo kons. stejných typů
pokud ano, vrátí ret_val která obsahuje výsledek vars/kons. po operaci
pokud ne, semantic_anal vrátí chybu 6 - sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech.
*/

f_foo_var * variable_check(f_foo_var *var1, f_foo_var *var2, Operations op) {
    f_foo_var *ret_var = (f_foo_var *) malloc(sizeof(f_foo_var));
    if (ret_var == NULL) {
        exit(99);
    }
    ret_var->isHandle = false;
    ret_var->isDefined = false;
    ret_var->isImmediate = false;
    if (op == plus) {
        // integer + integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = add_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;

            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = add_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = add_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *Sum = add_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            }
        }
            // number + number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = add_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = add_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = add_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *Sum = add_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            // integer + number nebo number + integer
        else if ((var1->type == t_number && var2->type == t_integer) ||
                 (var1->type == t_integer && var2->type == t_number)) {
            ret_var->isHandle = true;
            ret_var->type = t_number;
            // number + integer
            if (var1->type == t_number && var2->type == t_integer) {
                if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                    var_desc *const1 = createDblVar(var1->numVal);
                    var_desc *convert = createDblVar((float)var2->intVal);
                    var_desc *Sum = add_inst(const1, convert);
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                    var_desc *convert = int2float_inst(var2->VarDesc);
                    var_desc *Sum = add_inst(var1->VarDesc, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                    var_desc *convert = createDblVar((float)var2->intVal);
                    var_desc *Sum = add_inst(var1->VarDesc, convert);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                    var_desc *const1 = createDblVar(var1->numVal);
                    var_desc *convert = int2float_inst(var2->VarDesc);
                    var_desc *Sum = add_inst(const1, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                }
            }
                // integer + number
            else {
                if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                    var_desc *convert = createDblVar((float) var1->intVal);
                    var_desc *const1 = createDblVar(var2->numVal);
                    var_desc *Sum = add_inst(const1, convert);
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                    var_desc *convert = int2float_inst(var1->VarDesc);
                    var_desc *Sum = add_inst(var2->VarDesc, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                    var_desc *convert = int2float_inst(var1->VarDesc);
                    var_desc *const1 = createDblVar(var2->numVal);
                    var_desc *Sum = add_inst(const1, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                    var_desc *convert = createDblVar((float) var1->intVal);
                    var_desc *Sum = add_inst(var2->VarDesc, convert);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                }
            }
        } else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        } else {
            exit(6);
        }
    } else if (op == minus) {
        // integer - integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = sub_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = sub_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = sub_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *Sum = sub_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            }
        }
            // number - number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = sub_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = sub_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = sub_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *Sum = sub_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            // integer - number nebo number - integer
        else if ((var1->type == t_number && var2->type == t_integer) ||
                 (var1->type == t_integer && var2->type == t_number)) {
            ret_var->isHandle = true;
            ret_var->type = t_number;
            // number - integer
            if (var1->type == t_number && var2->type == t_integer) {
                if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                    var_desc *const1 = createDblVar(var1->numVal);
                    var_desc *convert = createDblVar((float) var2->intVal);
                    var_desc *Sum = sub_inst(const1, convert);
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                    var_desc *convert = int2float_inst(var2->VarDesc);
                    var_desc *Sum = sub_inst(var1->VarDesc, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                    var_desc *convert = createDblVar((float) var2->intVal);
                    var_desc *Sum = sub_inst(var1->VarDesc, convert);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                    var_desc *const1 = createDblVar(var1->numVal);
                    var_desc *convert = int2float_inst(var2->VarDesc);
                    var_desc *Sum = sub_inst(const1, convert);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                }
            }
                // integer - number
            else {
                if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                    var_desc *convert = createDblVar((float) var1->intVal);
                    var_desc *const1 = createDblVar(var2->numVal);
                    var_desc *Sum = sub_inst(const1, convert);
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->VarDesc = Sum;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                    var_desc *convert = int2float_inst(var1->VarDesc);
                    var_desc *Sum = sub_inst(convert, var2->VarDesc);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                    var_desc *convert = int2float_inst(var1->VarDesc);
                    var_desc *const1 = createDblVar(var2->numVal);
                    var_desc *Sum = sub_inst(convert, const1);
                    ret_var->VarDesc = Sum;
                    freeVarDesc(&const1);
                    freeVarDesc(&convert);
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                    var_desc *cons = createDblVar((float)var1->intVal);
                    var_desc *res = sub_inst(cons, var2->VarDesc);
                    freeVarDesc(&cons);
                    ret_var->VarDesc = res;
                    ret_var->isHandle = true;
                    ret_var->type = t_number;
                }
            }
        } else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        } else {
            exit(6);
        }
    } else if (op == mul) {
        //integer * integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = mul_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = mul_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = mul_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *Sum = mul_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            }
        }
            // float * float
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = mul_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = mul_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = mul_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *Sum = mul_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            // float * integer
        else if (var1->type == t_number && var2->type == t_integer) {
            ret_var->isHandle = true;
            ret_var->type = t_number;
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *convert = createDblVar((float) var2->intVal);
                var_desc *Sum = mul_inst(const1, convert);
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *convert = int2float_inst(var2->VarDesc);
                var_desc *Sum = mul_inst(var1->VarDesc, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *convert = createDblVar((float) var2->intVal);
                var_desc *Sum = mul_inst(var1->VarDesc, convert);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *convert = int2float_inst(var2->VarDesc);
                var_desc *Sum = mul_inst(const1, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            //integer * number
        else if (var1->type == t_integer && var2->type == t_number) {
            ret_var->isHandle = true;
            ret_var->type = t_number;
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *convert = createDblVar((float) var1->intVal);
                var_desc *const1 = createDblVar(var2->numVal);
                var_desc *Sum = mul_inst(const1, convert);
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *convert = int2float_inst(var1->VarDesc);
                var_desc *Sum = mul_inst(var2->VarDesc, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *convert = int2float_inst(var1->VarDesc);
                var_desc *const1 = createDblVar(var2->numVal);
                var_desc *Sum = mul_inst(const1, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar((float)var1->intVal);
                var_desc *Sum = mul_inst(const1, var2->VarDesc);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        } else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // celočíselné dělení
    else if (op == idiv) {
        ret_var->isHandle = true;
        ret_var->type = t_integer;
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = idiv_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = idiv_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *Sum = idiv_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *Sum = idiv_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_integer;
            }
        } else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // dělení float
    else if (op == fdiv) {
        ret_var->isHandle = true;
        ret_var->type = t_number;
        if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = div_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = div_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *Sum = div_inst(var1->VarDesc, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *Sum = div_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            //number / integer
        else if (var1->type == t_number && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *convert = createDblVar((float) var2->intVal);
                var_desc *Sum = div_inst(const1, convert);
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *convert = int2float_inst(var2->VarDesc);
                var_desc *Sum = div_inst(var1->VarDesc, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *convert = createDblVar((float) var2->intVal);
                var_desc *Sum = div_inst(var1->VarDesc, convert);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *convert = int2float_inst(var2->VarDesc);
                var_desc *Sum = div_inst(const1, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            // integer / number
        else if (var1->type == t_integer && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *convert = createDblVar((float) var1->intVal);
                var_desc *const1 = createDblVar(var2->numVal);
                var_desc *Sum = div_inst(convert, const1);
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *convert = int2float_inst(var1->VarDesc);
                var_desc *Sum = div_inst(convert, var2->VarDesc);
                ret_var->VarDesc = Sum;
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var2->numVal);
                var_desc *convert = int2float_inst(var1->VarDesc);
                var_desc *Sum = div_inst(convert, const1);
                freeVarDesc(&convert);
                freeVarDesc(&const1);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar((float)var1->intVal);
                var_desc *Sum = div_inst(const1, var2->VarDesc);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        }
            //integer / integer
        else if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar((float)var1->intVal);
                var_desc *const2 = createDblVar((float)var2->intVal);
                var_desc *Sum = div_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc != NULL) {
                var_desc *Sum = div_inst(var1->VarDesc, var2->VarDesc);
                ret_var->VarDesc = Sum;
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar((float)var2->intVal);
                var_desc *convert = int2float_inst(var1->VarDesc);
                var_desc *Sum = div_inst(convert, const2);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const2);
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar((float)var1->intVal);
                var_desc *convert = int2float_inst(var2->VarDesc);
                var_desc *Sum = div_inst(const1, convert);
                ret_var->VarDesc = Sum;
                freeVarDesc(&const1);
                freeVarDesc(&convert);
                ret_var->isHandle = true;
                ret_var->type = t_number;
            }
        } else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // operator <
    else if (op == less_than) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer < integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = lt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res = lt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = lt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = lt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }

        }
            // number < number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = lt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res = lt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = lt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = lt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //string < string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = lt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res = lt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = lt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = lt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // operator >
    else if (op == more_than) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer > integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = gt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res = gt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = gt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = gt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //number > number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = gt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res = gt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = gt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = gt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //string > string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = gt_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res = gt_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = gt_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = gt_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // operator <=
    else if (op == less_eq_than) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer <= integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res0 = lt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res0 = lt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res0 = lt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = lt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }

        }
            //number <= number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res0 = lt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res0 = lt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res0 = lt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = lt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }
        }
            // string <= string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res0 = lt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res0 = lt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res0 = lt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = lt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // operator >=
        // jeste chybí more/less or equal then v codegen
    else if (op == more_eq_than) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer >= integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res0 = gt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res0 = gt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res0 = gt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = gt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }

        }
            //number >= number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res0 = gt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res0 = gt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res0 = gt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = gt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }
        }
            //string >= string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res0 = gt_inst(const1, const2);
                var_desc *res1 = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res0 = gt_inst(const1, var2->VarDesc);
                var_desc *res1 = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res0 = gt_inst(var1->VarDesc, const2);
                var_desc *res1 = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            } else {
                var_desc *res0 = gt_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res = or_inst(res0, res1);
                freeVarDesc(&res0);
                freeVarDesc(&res1);
                ret_var->VarDesc = res;
            }
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // op ==
    else if (op == eq) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer == integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //number == number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //string == string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = eq_inst(const1, const2);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                freeVarDesc(&const2);
                ret_var->VarDesc = res;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                freeVarDesc(&res);
                ret_var->VarDesc = res;
            }
        }
            //tady nepredelavat!!
        else if (var2->type == t_null && var1->VarDesc != NULL) {
            var_desc *nVal = createNilVar();
            var_desc *res = eq_inst(nVal, var1->VarDesc);
            freeVarDesc(&nVal);
            ret_var->VarDesc = res;
        } else if (var2->VarDesc != NULL && var1->type == t_null) {
            var_desc *nVal = createNilVar();
            var_desc *res = eq_inst(nVal, var2->VarDesc);
            freeVarDesc(&nVal);
            ret_var->VarDesc = res;
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // op ~=
    else if (op == not_eq) {
        ret_var->isHandle = true;
        ret_var->type = t_bool;
        //integer ~= integer
        if (var1->type == t_integer && var2->type == t_integer) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = eq_inst(const1, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createIntVar(var1->intVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createIntVar(var2->intVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            }
        }
            //number ~= number
        else if (var1->type == t_number && var2->type == t_number) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = eq_inst(const1, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createDblVar(var1->numVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createDblVar(var2->numVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            }
        }
            //string ~= string
        else if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = eq_inst(const1, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const1);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *const1 = createStrVar(var1->stringVal);
                var_desc *res = eq_inst(const1, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&const1);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            } else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *const2 = createStrVar(var2->stringVal);
                var_desc *res = eq_inst(var1->VarDesc, const2);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                freeVarDesc(&const2);
                ret_var->VarDesc = res1;
            } else {
                var_desc *res = eq_inst(var1->VarDesc, var2->VarDesc);
                var_desc *res1 = not_inst(res);
                freeVarDesc(&res);
                ret_var->VarDesc = res1;
            }
        }
            //tady nepredelavat!!
        else if (var2->type == t_null && var1->VarDesc != NULL) {
            var_desc *nVal = createNilVar();
            var_desc *res = eq_inst(nVal, var1->VarDesc);
            freeVarDesc(&nVal);
            var_desc *res1 = not_inst(res);
            freeVarDesc(&res);
            ret_var->VarDesc = res1;
        } else if (var2->VarDesc != NULL && var1->type == t_null) {
            var_desc *nVal = createNilVar();
            var_desc *res = eq_inst(nVal, var2->VarDesc);
            freeVarDesc(&nVal);
            var_desc *res1 = not_inst(res);
            freeVarDesc(&res);
            ret_var->VarDesc = res1;
        } else if ((var1->type == t_null && var2->type != t_null) || (var1->type != t_null && var2->type == t_null)) {
            exit(8);
        } else {
            exit(6);
        }
    }
        // op #"a" pro len volat jako variable_check(string, null, op len)
    else if (op == len) {
        ret_var->isHandle = true;
        ret_var->type = t_integer;
        if (var2->type == t_string && var1 == NULL) {
            if (var2->VarDesc == NULL) {
                var_desc *tmpstr = createStrVar(var2->stringVal);
                ret_var->VarDesc = strlen_inst(tmpstr);
                freeVarDesc(&tmpstr);
            }
            else {
                ret_var->VarDesc = strlen_inst(var2->VarDesc);
            }
        }
        else if (var2->type == t_null) {
            exit(8);
        }
        else {
            exit(6);
        }
    }
        //op "a".."b"
    else if (op == concat) {
        ret_var->isHandle = true;
        ret_var->type = t_string;
        if (var1->type == t_string && var2->type == t_string) {
            if (var1->VarDesc == NULL && var2->VarDesc == NULL) {
                var_desc *str1 = createStrVar(var1->stringVal);
                var_desc *str2 = createStrVar(var2->stringVal);
                ret_var->VarDesc = concat_inst(str1, str2);
                freeVarDesc(&str1);
                freeVarDesc(&str2);
            }
            else if (var1->VarDesc == NULL && var2->VarDesc != NULL) {
                var_desc *str1 = createStrVar(var1->stringVal);
                ret_var->VarDesc = concat_inst(str1, var2->VarDesc);
                freeVarDesc(&str1);
            }
            else if (var1->VarDesc != NULL && var2->VarDesc == NULL) {
                var_desc *str2 = createStrVar(var2->stringVal);
                ret_var->VarDesc = concat_inst(var1->VarDesc, str2);
                freeVarDesc(&str2);
            }
            else {
                ret_var->VarDesc = concat_inst(var1->VarDesc, var2->VarDesc);
            }
        }
        else if (var1->type == t_null || var2->type == t_null) {
            exit(8);
        }
        else {
            exit(6);
        }
    }
    else {
        // semantic error i guess
        exit(6);
    }
    return ret_var;
}



/*
funkce move_eq přiřadí hodnotu dané variable
a = 1
a= c
atd...
do syntax_anal už nejde, není důvod (si myslím)
*/
// možná to bude ještě potřeba vracet syntaktaku kdyby byl kod
/*
a=5
if a == 5
...
*/

void hard_move(f_foo_var *var, var_desc *handle) {
    move_inst(var->VarDesc, handle);
    freeVarDesc(&handle);
}

void move_eq(f_foo_var *var1, f_foo_var *var2) {
    if (var1->type == t_integer && var2->type == t_integer) {
        if (var2->VarDesc == NULL) {
            var_desc *VarVal = createIntVar(var2->intVal);
            move_inst(var1->VarDesc, VarVal);
            freeVarDesc(&VarVal);
        }
        else if (var2->VarDesc != NULL) {
            move_inst(var1->VarDesc, var2->VarDesc);
        }
    }
    else if (var1->type == t_number && var2->type == t_integer) {
        if (var2->VarDesc == NULL) {
            var_desc *VarVal = createDblVar((float)var2->intVal);
            move_inst(var1->VarDesc, VarVal);
            freeVarDesc(&VarVal);
        }
        else {
            var_desc *VarVal = int2float_inst(var2->VarDesc);
            move_inst(var1->VarDesc, VarVal);
            freeVarDesc(&VarVal);
        }
    }
    else if (var1->type == t_number && var2->type == t_number) {
        if (var2->VarDesc == NULL) {
            var_desc *VarVal = createDblVar(var2->numVal);
            move_inst(var1->VarDesc, VarVal);
            freeVarDesc(&VarVal);
        }
            // je to handle
        else {
            move_inst(var1->VarDesc, var2->VarDesc);
        }
    }
    else if (var1->type == t_string && var2->type == t_string) {
        if (var2->VarDesc == NULL) {
            var_desc *VarVal = createStrVar(var2->stringVal);
            move_inst(var1->VarDesc, VarVal);
            freeVarDesc(&VarVal);
        }
        else {
            move_inst(var1->VarDesc, var2->VarDesc);
        }
    }
    else if (var2->type == t_null) {
        if (var2->VarDesc == NULL) {
            var_desc *nVal = createNilVar();
            move_inst(var1->VarDesc, nVal);
            freeVarDesc(&nVal);
        }
        else {
            move_inst(var1->VarDesc, var2->VarDesc);
        }
    }
        // doufám že to jsou všechny validní možnosti přiřazení
    else {
        exit(4);
    }
}

bool assing_check(f_foo_var *var1, f_foo_var *var2) {
    bool check = true;
    if (var2->type == t_null) {}
    else if (var1->type == var2->type) {}
    else if (var1->type == t_number && var2->type == t_integer) {}
    else {
        check = false;
    }
    return check;
}

//if
void if_head(f_foo_var * var_condition) {
    if (var_condition->type == t_bool) {
        beginIf(var_condition->VarDesc);
        freeVarDesc(&var_condition->VarDesc);
    }
    else {
        freeVarDesc(&var_condition->VarDesc);
        exit(4);
    }
}
void else_head() {
    beginElse();
}
void end_if() {
    endIf();
}
//while


void while_announce() {
    announceWhile();
}
void while_start(f_foo_var * condition) {
    if (condition->type == t_bool) {
        whileCondition(condition->VarDesc);
        freeVarDesc(&condition->VarDesc);
        free(condition);
    } else {
        freeVarDesc(&condition->VarDesc);
        free(condition);
        exit(4);
    }
}
void while_end() {
    whileEnd();
}

var_desc *decVar (f_foo_var *var) {
    char * name = strdup(var->id);
    var_desc * Var = declareVar(name);
    return Var;
}


/* DEKLARACE FUNKCE */
void funDec_s(f_foo_str *decFun) {
    beginFunction(decFun->id);

    for (int i = 0; i < decFun->argsCount; i++) {
        decFun->args[i]->VarDesc = getArg(decFun->args[i]->id);
    }
}
void to_return(f_foo_str *calledFun, f_foo_var **ret, int expressionCount) {
    if (expressionCount != calledFun->retArgsCount) {
        exit(5);
    }
    for (int j = 0; j < calledFun->retArgsCount; j++) {

        if (ret[j]->isImmediate) {
            if (ret[j]->type == t_integer) {
                ret[j]->VarDesc = createIntVar(ret[j]->intVal);
            }

            if (ret[j]->type == t_number) {
                ret[j]->VarDesc = createDblVar(ret[j]->numVal);
            }

            if (ret[j]->type == t_string) {
                ret[j]->VarDesc = createStrVar(ret[j]->stringVal);
            }
        }

        stageReturn(ret[j]->VarDesc, j);
    }
}
void funDec_e() {
    endFunction();
}

void funDec_ret() {
    returnFunction();
}
/* KONEC DEKLARACE FUNKCE */



/* VOLÁNÍ FUNKCE */
void fun_arg_check(f_foo_str* calledFunction, f_foo_var **tmpArgArray, int argCount) {

    if (calledFunction->argsCount != argCount) {
        exit(4); // TODO pls checknite ci je good err code
    }

    for (int i = 0; i < argCount; i++) {
        if (calledFunction->args[i]->type == tmpArgArray[i]->type) {}
        else if (calledFunction->args[i]->type == t_number && tmpArgArray[i]->type == t_integer) {}
        else if (tmpArgArray[i]->type == t_null) {}
        else {
            exit(4);
        }
    }
}

var_desc ** fun_call(f_foo_str *calledFun, f_foo_var **args, int argCount) {
    fun_arg_check(calledFun, args, argCount);
    var_desc **ret_h = malloc(calledFun->retArgsCount * sizeof(var_desc*));
    if (strcmp(calledFun->id, "write") == 0) {
        for (int j = 0; j < argCount; j++) {
            if (args[j]->VarDesc != NULL) {
                write_inst(args[j]->VarDesc);
            }
            else {
                if (args[j]->type == t_number) {
                    var_desc *var = createDblVar(args[j]->numVal);
                    write_inst(var);
                    freeVarDesc(&var);
                }
                else if (args[j]->type == t_string) {
                    var_desc *var = createStrVar(args[j]->stringVal);
                    write_inst(var);
                    freeVarDesc(&var);
                }
                else if (args[j]->type == t_integer) {
                    var_desc *var = createIntVar(args[j]->intVal);
                    write_inst(var);
                    freeVarDesc(&var);
                }
                else {
                    var_desc *var = createNilVar();
                    write_inst(var);
                    freeVarDesc(&var);
                }
            }
        }
    }
    else if (strcmp(calledFun->id, "reads") == 0) {
        ret_h[0] = read_inst(STRING);
    }
    else if (strcmp(calledFun->id, "readi") == 0) {
        ret_h[0] = read_inst(INTEGER);
    }
    else if (strcmp(calledFun->id, "readn") == 0) {
        ret_h[0] = read_inst(NUMBER);
    }
    else {
        for (int i = 0; i < argCount; i++) {
            if (args[i]->VarDesc != NULL) {

                if (args[i]->type == t_integer && calledFun->args[i]->type == t_number) {
                    var_desc *tmpVarDesc = int2float_inst(args[i]->VarDesc);
                    prepareArg(tmpVarDesc, calledFun->args[i]->id);
                } else {
                    prepareArg(args[i]->VarDesc, calledFun->args[i]->id);
                }
            }
            else {
                if (args[i]->type == t_number) {
                    var_desc *arg_n = createDblVar(args[i]->numVal);
                    prepareArg(arg_n, calledFun->args[i]->id);

                    freeVarDesc(&arg_n);
                } else if (args[i]->type == t_integer) {
                    // pretipovani na number
                    if (calledFun->args[i]->type == t_number) {
                        var_desc *arg_n = createDblVar((float) args[i]->intVal);
                        prepareArg(arg_n, calledFun->args[i]->id);

                        freeVarDesc(&arg_n);
                    } else {
                        var_desc *arg_i = createIntVar(args[i]->intVal);
                        prepareArg(arg_i, calledFun->args[i]->id);

                        freeVarDesc(&arg_i);
                    }
                } else if (args[i]->type == t_string) {
                    var_desc *arg_s = createStrVar(args[i]->stringVal);
                    prepareArg(arg_s, calledFun->args[i]->id);

                    freeVarDesc(&arg_s);
                } else if (args[i]->type == t_null) {
                    var_desc *arg_nil = createNilVar();
                    prepareArg(arg_nil, calledFun->args[i]->id);

                    freeVarDesc(&arg_nil);
                }
            }
        }
        generateFCall(calledFun->id);
        for (int j = 0; j < calledFun->retArgsCount; j++) {
            ret_h[j] = getReturn(j);
        }
    }
    return ret_h;
}
/* KONEC VOLANI FUNKCE */
