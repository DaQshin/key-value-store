#pragma once

#include "avl.h"
#include "hashtable.h"

struct ZSet{
    AVLNode* tree = nullptr;
    HMap hmap;
};

struct ZNode{
    AVLNode root;
    HNode hnode;
    double score = 0;
    size_t len = 0;
    char name[0];
};

bool zset_insert(ZSet* zset, const char* name, size_t len, double score);
ZNode* zset_lookup(ZSet* zset, const char* name, size_t len);
void zset_delete(ZSet* zset, ZNode* node);  
void zset_clear(ZSet* zset);
ZNode* zset_seekge(ZSet* zset, double score, const char* name, size_t len);
ZNode* znode_offset(ZNode* node, int64_t offset);