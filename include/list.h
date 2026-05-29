#pragma once

struct DList{
    DList* next = nullptr;
    DList* prev = nullptr;
};

inline void dlist_init(DList* node){
    node->next = node->prev = node;
}

inline bool dlist_empty(DList* node){
    return node->next == node;
}

void dlist_detach(DList* node){
    DList* prev = node->prev;
    DList* next = node->next;
    next->prev = prev;
    prev->next = next;
    
    node->next = node->prev = nullptr;
}

void dlist_insert_before(DList* target, DList* node){
    DList* prev = target->prev;

    prev->next = node;
    node->prev = prev;

    node->next = target;
    target->prev = node;
}