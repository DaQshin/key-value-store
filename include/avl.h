#pragma once

#include <stddef.h>
#include <stdint.h>

struct AVLNode {
    struct AVLNode* parent = nullptr;
    struct AVLNode* left = nullptr;
    struct AVLNode* right = nullptr;
    uint32_t height = 0;
    uint32_t count = 0;
};

inline void avl_init(AVLNode* node){
    node->parent = node->left = node->right = nullptr;
    node->height = 1;
    node->count = 1;
}

inline uint32_t get_avl_height(AVLNode* node){ return node ? node->height : 0;}
inline uint32_t get_avl_count(AVLNode* node){return node ? node->count : 0;}

AVLNode* avl_fix(AVLNode* node);
AVLNode* avl_del(AVLNode* node);