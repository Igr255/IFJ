/**
 * Adam Ondrousek <xondro09>
 * Filip Karasek <xkaras39>
 */

#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synCheck.h"

// inicializace
void bst_init(bst_node_t **tree) {
    *tree = NULL;
}

// vyhledavani
bool bst_search(bst_node_t *tree, char *key, bst_node_t **result) {
    if (!tree) {
        // error status
        return false;
    }
    if (strcmp(key, tree->key) == 0) {
        *result = (tree);
        return true;
    }
    if (strcmp(key, tree->key) > 0) {
        return bst_search(tree->right, key, result);
    }
    else {
        return bst_search(tree->left, key, result);
    }
}

//vlozeni
void bst_insert(bst_node_t **tree, char *key, void *data) {
    if (!(*tree)) {
        (*tree) = malloc(sizeof(struct bst_node));
        if (&tree == NULL) {
            return;
        }
        (*tree)->key = key;
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        (*tree)->content = data;
        return;
    }
    if (strcmp(key, (*tree)->key) == 0) {
        free((*tree)->key);
        (*tree)->key = key;

        (*tree)->content = data;
        return;
    }
    if (strcmp(key, (*tree)->key) > 0) {
        bst_insert(&(*tree)->right, key, data);
        return;
    }
    else {
        bst_insert(&(*tree)->left, key, data);
        return;
    }
}

void replace_by_r_most(bst_node_t *replace, bst_node_t **tree) {
    if ((*tree)->right->right != NULL) {
        replace_by_r_most(replace, &(*tree)->right);
    }
    else {
        bst_node_t *new = (*tree)->right;
        replace->key = new->key;
        replace->content = new->content;
        if (new->left != NULL) {
            (*tree)->right = new->left;
        }
        free(new);
        (*tree)->right = NULL;
    }
}

void bst_delete(bst_node_t **tree, char *key) {
    // strom je prazdny
    if (*tree == NULL) {
        return;
    }
    // mazaný prvek
    if (strcmp(key, (*tree)->key) == 0) {
        // pomocna promena pri mazani prvku
        bst_node_t *tmp;
        // nemá žádného potomka
        if ((*tree)->left == NULL && (*tree)->right == NULL) {
            free(*tree);
            *tree = NULL;
        }
        // má oba potomky
        else if ((*tree)->left != NULL && (*tree)->right != NULL) {
            replace_by_r_most(*tree, &(*tree)->left);
            return;
        }
        else {
            // má jen levého potomka
            if ((*tree)->left != NULL) {
                tmp = (*tree)->left;
                free(*tree);
                *tree = tmp;
            }
                // má jen pravého potomka
            else {
                tmp = (*tree)->right;
                free(*tree);
                *tree = tmp;
            }
        }
    }
    // rekurzivní hledání mazaného prvku
    else {
        if (strcmp(key, (*tree)->key) > 0) {
            bst_delete(&(*tree)->right, key);
        }
        else {
            bst_delete(&(*tree)->left, key);
        }
    }
}

void bst_dispose (bst_node_t **tree, structType type)
{
    if (*tree == NULL) {
        return;
    } 
    else {
        bst_dispose(&(*tree)->left, type);
        bst_dispose(&(*tree)->right, type);

        if (type == variable) {
            free(((f_foo_var *) (*tree)->content)->id);
            free(((f_foo_var *) (*tree)->content)->originalId);

            if (((f_foo_var *) (*tree)->content)->VarDesc != NULL) {
                freeVarDesc(&((f_foo_var *) (*tree)->content)->VarDesc);
            }

            free(((f_foo_var *) (*tree)->content));
        }
        else if (type == function) {

            f_foo_str * debug = ((f_foo_str *)((*tree)->content));

            bst_dispose(&(((f_foo_str *)((*tree)->content))->localVars), variable);

            if (((f_foo_str *)((*tree)->content))->opCount > 0) {
                for (int i = 0; i < ((f_foo_str *) ((*tree)->content))->opCount; i++) {
                    free(((f_foo_str *) ((*tree)->content))->opArray[i]->id);
                    ((f_foo_str *) ((*tree)->content))->opArray[i]->id = NULL;

                    bst_dispose(&(((f_foo_str *)((*tree)->content))->opArray[i]->localScopeVars), variable);

                    free(((f_foo_str *) ((*tree)->content))->opArray[i]);
                    ((f_foo_str *) ((*tree)->content))->opArray[i] = NULL;
                }
            }
            if (((f_foo_str *) ((*tree)->content))->opArray != NULL)
                free(((f_foo_str *) ((*tree)->content))->opArray);

            if (((f_foo_str *)((*tree)->content))->argsCount > 0) {
                for (int i = 0; i < ((f_foo_str *)((*tree)->content))->argsCount; i++) {

                    if (((f_foo_str *)((*tree)->content))->args[i]->VarDesc != NULL)
                        freeVarDesc(&(((f_foo_str *)((*tree)->content))->args[i]->VarDesc));

                    free(((f_foo_str *)((*tree)->content))->args[i]->originalId);
                    free(((f_foo_str *)((*tree)->content))->args[i]->id);
                    free(((f_foo_str *)((*tree)->content))->args[i]->stringVal);
                    free(((f_foo_str *)((*tree)->content))->args[i]);
                }
            }
            if (((f_foo_str *)((*tree)->content))->args != NULL)
                free(((f_foo_str *)((*tree)->content))->args);

            free(((f_foo_str *)((*tree)->content))->ret_args);
            free(((f_foo_str *)((*tree)->content))->id);
            free(((f_foo_str *)((*tree)->content)));
            (*tree)->content = NULL;
        }

        free(*tree);
        *tree = NULL;
    }
}
