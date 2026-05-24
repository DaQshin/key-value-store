#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio,h>
#include "zset.h"
#include "utils.h"

static ZNode* znode_new(const char* name, size_t len, double score){
    ZNode* node = (ZNode*)malloc(sizeof(ZNode) + len);
    if(!node) printf("ZNode initialization failed.");  

    avl_init(&node->root);
    hnode->next = nullptr;
    hnode->hash = str_hash((const uint8_t*)name, len);
    node->score = score;
    node->len = len;
    memcpy(&node->name[0], name, len);
    return node;
}

static void znode_del(ZNode* node) return free(node);

static bool zless(AVLNode* lhs, AVLNode* rhs){
    ZNode* zl = container_of(lhs, ZNode, AVLNode);
    ZNode* zr = container_of(rhs, ZNode, AVLNode);
    if(zl->score != zr->score) return zl->score < zr->score;

    int rv = memcmp(zl->name, zr->name, min(zl->len, zr->len));
    return rv != 0 ? (rv < 0) : (zl->len < zr->len);
}

static void tree_insert(ZSet* zset, ZNode* node){
    AVLNode* parent = nullptr;
    AVLNode** from = &zset->tree;
    while(*from){
        parent = *from;
        from = zless(&node->root, parent) ? &parent->left : &parent->right;
    }
    *from = &node->root;
    node->root.parent = parent;
    zset->tree = avl_fix(*from);
}

struct HKey{
    HNode node;
    size_t len = 0;
    const char* name = nullptr;
};

ZNode* zset_lookup(ZSet* zset, const char* name, size_t len){
    HKey key;
    key.node.hash = str_hash((const uint8_t*)name, len);
    key.len = len;
    key.name = name;
    Node* found = hm_lookup(&zset->hmap, &key.node, &hcmp);

    return found ? container_of(found, ZNode, HNode) : nullptr;
}

static void zset_update(Zset* zset, ZNode* node,  double score){
    zset->tree = avl_del(&node->root);
    avl_init(&node->root);
    node->score = score;
    tree_insert(zset, node);
}


static void zset_insert(ZSet* zset, const char* name, size_t len, double score){
    if(ZNode* node = zset_lookup(zset, name, len)) {
        zset_update(zset, node, score);
    }

    ZNode* node = znode_new(name, len, score);
    hm_insert(&zset->hmap, &node->hnode);
    tree_insert(zset, node);
}

static void zset_delete(ZSet* zset, ZNode* node){
    HKey key;
    key.node = node->hnode;
    key.len = node->len;
    key.name = node->name;
    HNode* found = hm_delete(&zset->hmap, &node->hnode, &hcmp);
    
    zset->tree = avl_del(&node->root);
    znode_del(node);
}   


