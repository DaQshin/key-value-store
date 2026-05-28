#pragma once

struct List{
    List* next = nullptr;
    List* prev = nullptr;
};

void list_init(List* node){
    node->next = node->prev = node;
}

bool is_list_empty(List* node){
    return node->next == node;
}

void list_detach(List* node){
    List* next = node->next;
    List* prev = node->prev;
    prev->next = next;
    next->prev = prev;
    delete node;
}

void list_insert(List* target, List* head){
    List* prev = head->prev;
    prev->next = target;
    target->prev = prev;
    target->next = head;
    head->prev = target;
}