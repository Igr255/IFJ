/**
 * Adam Ondrousek <xondro09>
 * Filip Karasek <xkaras39>
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "synCheck.h"
#include "expressionParse.h"

void generate_ifj();
f_foo_var * variable_check(f_foo_var *var1, f_foo_var *var2, Operations type);
bool assing_check(f_foo_var *var1, f_foo_var *var2);
void endifj();
void move_eq(f_foo_var *var1, f_foo_var *var2);
void if_head(f_foo_var * var_condition);
void else_head();
void end_if();
void while_announce();
void while_start(f_foo_var * condition);
void while_end();
void fun_arg_check(f_foo_str* calledFunction, f_foo_var **tmpArgArray, int argCount);
var_desc *decVar (f_foo_var *var);
void funDec_s(f_foo_str *decFun);
void to_return(f_foo_str *calledFun, f_foo_var **ret, int expressionCount);
void funDec_e();
void funDec_ret();
void hard_move(f_foo_var *var, var_desc *handle);
var_desc ** fun_call(f_foo_str *calledFun, f_foo_var **args, int argCount);

#endif

