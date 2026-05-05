#include <assert.h>
#include "avl.h"

static uint32_t max(uint32_t lhs, uint32_t rhs){
    return lhs > rhs ? lhs : rhs;
}

static void avl_update_height(AVLNode* node){
    node->height = 1 + max(avl_height(node->left), avl_height(node->right));
}

static uint8_t avl_get_height_diff(AVLNode node){
    uintptr_t p = (uintptr_t)node->parent;
    return (p & 0b11);
}

static AVLNode* avl_get_parent(AVLNode* node){
    uintptr_t p = (uintptr_t)node->parent;
    return (AVLNode*)(p & (~0b11) );
}

static AVLNode* avl_rotate_left(AVLNode* node){
    
}