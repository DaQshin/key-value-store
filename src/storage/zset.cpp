#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include "zset.h"
#include "utils.h"

static ZNode* znode_new(const char* name, size_t len, double score){
    ZNode* node = (ZNode*)malloc(sizeof(ZNode) + len);
    if(!node) printf("ZNode initialization failed.");  

    avl_init(&node->root);
    node->hnode.next = nullptr;
    node->hnode.hash = str_hash((const uint8_t*)name, len);
    node->score = score;
    node->len = len;
    memcpy(&node->name[0], name, len);
    return node;
}

static void znode_del(ZNode* node){
    delete node;
}

static bool zless(AVLNode* lhs, AVLNode* rhs){
    ZNode* zl = container_of(lhs, ZNode, root);
    ZNode* zr = container_of(rhs, ZNode, root);
    if(zl->score != zr->score) return zl->score < zr->score;

    int rv = memcmp(zl->name, zr->name, std::min(zl->len, zr->len));
    return rv != 0 ? (rv < 0) : (zl->len < zr->len);
}

static bool zless(AVLNode* node, double score, const char* name, size_t len){
    ZNode* zn = container_of(node, ZNode, root);
    if(zn->score != score) return zn->score < score;

    int rv = memcmp(zn->name, name, min_strlen(zn->name, name));
    return rv != 0 ? (rv < 0) : (zn->len < len);

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

static bool hcmp(HNode* node, HNode* key){
    ZNode* znode = container_of(node, ZNode, hnode);
    ZNode* zkey = container_of(key, ZNode, hnode);
    if(znode->len != zkey->len) return false;

    return memcmp(znode->name, zkey->name, znode->len) == 0;
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
    HNode* found = hm_lookup(&zset->hmap, &key.node, &hcmp);

    return found ? container_of(found, ZNode, hnode) : nullptr;
}

static void zset_update(ZSet* zset, ZNode* node, double score){
    zset->tree = avl_del(&node->root);
    avl_init(&node->root);
    node->score = score;
    tree_insert(zset, node);
}

bool zset_insert(ZSet* zset, const char* name, size_t len, double score){
    if(ZNode* node = zset_lookup(zset, name, len)) {
        zset_update(zset, node, score);
        return false;
    }

    ZNode* node = znode_new(name, len, score);
    hm_insert(&zset->hmap, &node->hnode);
    tree_insert(zset, node);
    return true;
}

void zset_delete(ZSet* zset, ZNode* node){
    HKey key;
    key.node = node->hnode;
    key.len = node->len;
    key.name = node->name;
    HNode* found = hm_delete(&zset->hmap, &node->hnode, &hcmp);
    
    zset->tree = avl_del(&node->root);
    znode_del(node);
}   

ZNode* zset_seekge(ZSet* zset, double score, const char* name, size_t len){
    AVLNode* found = nullptr;
    for(AVLNode* node = zset->tree; node;){
        if(zless(node, score, name, len)){
            node = node->right;
            continue;
        }
        
        found = node;
        node = node->left;
    }

    return found ? container_of(found, ZNode, root) : nullptr;
}

void tree_dispose(AVLNode* node){
    if(!node) return;

    tree_dispose(node->left);
    tree_dispose(node->right);

    znode_del(container_of(node, ZNode, root));
}

void zset_clear(ZSet* zset){
    hm_clear(&zset->hmap);
    tree_dispose(zset->tree);
    zset->tree = nullptr;
}