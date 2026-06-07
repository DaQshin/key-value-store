#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

struct HeapItem {
    uint64_t val;
    size_t* ref;
};

size_t heap_left(size_t pos);
size_t heap_right(size_t pos);
void heap_update(HeapItem* a, size_t pos, size_t len);
void heap_del(std::vector<HeapItem>& a, size_t pos);
void heap_upsert(std::vector<HeapItem>& a, size_t pos, HeapItem t);