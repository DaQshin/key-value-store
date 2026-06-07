#include "heap.h"
#include <vector>
#include <map>
#include <gtest/gtest.h>

class HeapTest: public ::testing::Test{
protected: 
    struct Data{
        size_t heap_idx = -1;
    };

    struct Container{
        std::vector<HeapItem> heap;
        std::multimap<uint64_t, Data*> map;
    };

    void dispose(Container& c);
    void add(Container& c, uint64_t val);
    void del(Container& c, uint64_t val);
    void verify(Container& c);
};

void HeapTest::dispose(Container& c){
    for(auto p: c.map){
        delete p.second;
    }
}

void HeapTest::add(Container& c, uint64_t val){
    Data* d = new Data();
    c.map.insert(std::make_pair(val, d));
    HeapItem t;
    t.val = val;
    t.ref = &d->heap_idx;
    c.heap.push_back(t);
    heap_update(c.heap.data(), c.heap.size() - 1, c.heap.size());
}

void HeapTest::verify(Container& c){
    ASSERT_EQ(c.heap.size(), c.map.size()) << "Heap size does not match with Map size";
    for(size_t pos = 0; pos < c.heap.size(); pos++){
        size_t l = heap_left(pos);
        size_t r = heap_right(pos);

        ASSERT_TRUE(l >= c.heap.size() || c.heap[pos].val <= c.heap[l].val);
        ASSERT_TRUE(r >= c.heap.size() || c.heap[pos].val <= c.heap[r].val);
        ASSERT_EQ(*c.heap[pos].ref, pos);
    }
}

void HeapTest::del(Container& c, uint64_t val){
    auto it = c.map.find(val);
    ASSERT_NE(it, c.map.end());
    Data* d = it->second;
    ASSERT_EQ(c.heap.at(d->heap_idx).val, val);
    ASSERT_EQ(c.heap.at(d->heap_idx).ref, &d->heap_idx);

    c.heap[d->heap_idx] = c.heap.back();
    c.heap.pop_back();
    if (d->heap_idx < c.heap.size()) {
        heap_update(c.heap.data(), d->heap_idx, c.heap.size());
    }
    delete d;
    c.map.erase(it);
}

TEST_F(HeapTest, AddValues){
    for(uint32_t i = 0; i < 50; i++){
        Container c;
        for(uint32_t j = 0; j < 50; j++){
            add(c, j);
        }

        verify(c);
        add(c, i);
        verify(c);
        dispose(c);
    }
}

TEST_F(HeapTest, DeleteValues){
    for(uint32_t i = 0; i < 50; i++){
        Container c;
        for(uint32_t j = 0; j < 50; j++){
            add(c, j);
        }

        verify(c);
        del(c, i);
        verify(c);
        dispose(c);
    }
}