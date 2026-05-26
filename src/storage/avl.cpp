#include <assert.h>
#include <algorithm>
#include "avl.h"
#include "utils.h"

static void avl_update(AVLNode* node){
    node->height = 1 + std::max(avl_height(node->left), avl_height(node->right));
    node->count = 1 + avl_count(node->left) + avl_count(node->right);
}

static AVLNode* rot_left(AVLNode* node){
    AVLNode* parent = node->parent;
    AVLNode* right = node->right;
    AVLNode* inner = right->left;

    node->right = inner;
    if (inner) inner->parent = node;

    right->left = node;
    node->parent = right;
    right->parent = parent;

    avl_update(node);
    avl_update(right);

    return right;
}

static AVLNode* rot_right(AVLNode* node){
    AVLNode* parent = node->parent;
    AVLNode* left = node->left;
    AVLNode* inner = left->right;

    node->left = inner;
    if (inner) inner->parent = node;

    left->right = node;
    node->parent = left;
    left->parent = parent;

    avl_update(node);
    avl_update(left);

    return left;
}

static AVLNode* avl_fix_left(AVLNode* node){
    if(avl_height(node->left->left) < avl_height(node->left->right))
        node->left = rot_left(node->left);

    return rot_right(node);
}

static AVLNode* avl_fix_right(AVLNode* node){
    if(avl_height(node->right->right) < avl_height(node->right->left))
        node->right = rot_right(node->right);

    return rot_left(node);
}

AVLNode* avl_fix(AVLNode* node){
    while(true){
        AVLNode** from = &node;
        AVLNode* parent = node->parent;

        if(parent){
            from = parent->left == node ? &parent->left : &parent->right;
        }

        avl_update(node);

        uint32_t lh = avl_height(node->left);
        uint32_t rh = avl_height(node->right);

        if(lh == rh + 2){
            *from = avl_fix_left(node);
        }
        else if(rh == lh + 2){
            *from = avl_fix_right(node);
        }

        if(!parent) return *from;

        node = parent;
    }
}

static AVLNode* avl_del_one(AVLNode* node){
    assert(!node->left || !node->right);
    AVLNode* child = node->left ? node->left : node->right;
    AVLNode* parent = node->parent;

    if(child){
        child->parent = parent;
    }

    if(!parent) return child;

    AVLNode** from = parent->left == node ? &parent->left : &parent->right;
    *from = child;

    return avl_fix(parent);
}

AVLNode *avl_del(AVLNode *node) {
    if (!node->left || !node->right) {
        return avl_del_one(node);
    }
    AVLNode *victim = node->right;
    while (victim->left) {
        victim = victim->left;
    }
    AVLNode *root = avl_del_one(victim);
    *victim = *node;    
    if (victim->left) {
        victim->left->parent = victim;
    }
    if (victim->right) {
        victim->right->parent = victim;
    }
    AVLNode **from = &root;
    AVLNode *parent = node->parent;
    if (parent) {
        from = parent->left == node ? &parent->left : &parent->right;
    }
    *from = victim;
    return root;
}

AVLNode* avl_offset(AVLNode* node, int64_t offset){
    for(; offset > 0 && node; offset--){
        node = successor(node);
    }

    for(; offset < 0 && node; offset++){
        node = predecessor(node);
    }
}

static AVLNode* successor(AVLNode* node){
    if(node->right){
        AVLNode* n = node->right;
        while(n->left) n = n->left;
        return n;
    }

    while(AVLNode* parent = node->parent){
        if(node == parent->left) return node;
        node = parent;
    }

    return nullptr;
}

static AVLNode* predecessor(AVLNode* node){
    if(node->left){
        AVLNode* n = node->left;
        while(n->right) n = n->right;
    }

    while(AVLNode* parent = node->parent){
        if(node == parent->right) return node;
        node = parent;
    }

    return nullptr;
}
