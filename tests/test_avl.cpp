#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <set>
#include "avl.h"

#define container_of(ptr, type, member) ({
    const typeof( ((type *)0)->member)* __mptr = ptr;
    (type *)((char *) __mptr - offsetof(type, member));  
})

struct Data{
    AVLNode* node;
    uint32_t val = 0;
};

struct Container {
    AVLNode* root = nullptr;
};

static void add(Container& c, uint32_t val){
    Data* data = new Data();
    avl_init(&data->node);
    data->val = val;

    AVLNode* cur = nullptr;
    AVLNode** from = &c.root;
    while(*from){
        cur = *from;
        uint32_t node_val = container_of(cur, Data, node)->val;
        from = (val < node_val) ? &cur->left : &cur->right;
    }

    *from = &data->node;
    data->node.parent = cur;
    c.root = avl_fix(&data->node);
}

static bool del(Container& c, uint32_t val){
    AVLNode* cur = c.root;
    while(cur){
        uint32_t node_val = container_of(cur, Data, node)->val;
        if(node_val == val) break;
        
        cur = val < node_val ? cur->left : cur->right;
    }

    if(!cur) return 0;

    c.root = avl_del(cur);
    delete container_of(cur, Data, node);
    return 1;
}

int main(){

}
