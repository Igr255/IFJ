/**
 * Adam Ondrousek <xondro09>
 * Filip Karasek <xkaras39>
 */

#ifndef Bin_tree_h
#define Bin_tree_h

#include <stdbool.h>

typedef enum structType
{
    function,
    variable,
    none
}structType;

typedef struct bst_node {
  char *key;               
  void *content;             
  struct bst_node *left;  
  struct bst_node *right;
} bst_node_t;

void bst_init(bst_node_t **tree);
bool bst_search(bst_node_t *tree, char *key, bst_node_t **result);
void bst_insert(bst_node_t **tree, char *key, void *data);
void replace_by_r_most(bst_node_t *replace, bst_node_t **tree);
void bst_delete (bst_node_t **tree, char *key);
void bst_dispose (bst_node_t **tree, structType stuctType);

#endif
