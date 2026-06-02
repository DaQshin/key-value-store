#pragma once

struct HeapItem {
    uint64_t val;
    size_t* ref;
};

inline size_t heap_left(size_t pos) {
    return (2 * pos + 1);
}

inline size_t heap_right(size_t pos){
    return (2 * pos + 2);
}

inline size_t heap_parent(size_t pos){
    return (pos + 1 / 2 - 1);
}

static void heap_up(HeapItem* a, size_t pos);
static void heap_down(HeapItem* a, size_t pos, size_t len);
void heap_update(HeapItem* a, size_t pos);
void heap_del(HeapItem* a, size_t pos);