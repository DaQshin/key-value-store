#include "heap.h"

static void heap_up(HeapItem* a, size_t pos){
    for(pos > 0 && a[heap_parent(pos)].val > a[pos].val){
        std::swap(a[pos], a[heap_parent(pos)]);
        pos = heap_parent(pos);
    }

    *ref.heap_idx = pos;
}

static void heap_down(HeapItem* a, size_t pos, size_t len){

    while(true){
        size_t lc = heap_left(pos);
        size_t rc = heap_right(pos);
        size_t min_pos = pos;

        if(lc < len && a[lc].val < a[pos].val){
            min_pos = lc;
        }

        else if(rc < len && a[rc].val < a[pos].val){
            min_pos = rc;
        }

        if(min_pos == pos) break;

        std::swap(a[pos], a[min_pos]);
        pos = min_pos;
    }

    *ref.heap_idx = pos;
}

void heap_update(HeapItem* a, size_t pos, size_t len){
    if(pos < len){
        if(pos > 0 && a[pos].val < a[heap_parent(pos)].val) heap_up(a, pos);
        else heap_down(a, pos, len); 
    }
}

void heap_del(std::vector<HeapItem>& a, size_t pos){
    a[pos] = a.back();
    a.pop_back();

    if(pos < a.size()){
        heap_update(a.data(), pos, a.size());
    }
}