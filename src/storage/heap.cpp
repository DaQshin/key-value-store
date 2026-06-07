#include "heap.h"

size_t heap_left(size_t pos) {
    return (2 * pos + 1);
}

size_t heap_right(size_t pos){
    return (2 * pos + 2);
}

static size_t heap_parent(size_t pos){
    return (pos + 1) / 2 - 1;
}

static void heap_up(HeapItem* a, size_t pos){
    HeapItem t = a[pos];
    while(pos > 0 && t.val < a[heap_parent(pos)].val){
        a[pos] = a[heap_parent(pos)];
        *a[pos].ref = pos;
        pos = heap_parent(pos);
    }
    a[pos] = t;
    *a[pos].ref = pos;
}

static void heap_down(HeapItem* a, size_t pos, size_t len){
    HeapItem t = a[pos];
    while(true){

        size_t l = heap_left(pos);
        size_t r = heap_right(pos);

        size_t min_pos = pos;
        uint64_t min_val = t.val;

        if(l < len && a[pos].val < min_val){
            min_pos = l;
            min_val = a[l].val;
        }
        if(r < len && a[pos].val < min_val){
            min_pos = r;
        }

        if(min_pos == pos) break;

        a[pos] = a[min_pos];
        *a[pos].ref = pos;
        pos = min_pos;
    }

    a[pos] = t;
    *a[pos].ref = pos;   
}

void heap_update(HeapItem* a, size_t pos, size_t len){
    if(pos > 0 && a[pos].val < a[heap_parent(pos)].val) heap_up(a, pos);
    else heap_down(a, pos, len); 
}

void heap_del(std::vector<HeapItem>& a, size_t pos){
    a[pos] = a.back();
    a.pop_back();

    if(pos < a.size()){
        *a[pos].ref = pos;
        heap_update(a.data(), pos, a.size());
    }
}

void heap_upsert(std::vector<HeapItem>& a, size_t pos, HeapItem t){
    if(pos < a.size()){
        a[pos] = t;
    }
    else{
        pos = a.size();
        a.push_back(t);
    }
    heap_update(a.data(), pos, a.size());
}