#include "heap.h"

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <vector>
#include <cstdint>
#include <memory>

class HeapTest {
protected:
    struct Data {
        size_t heap_idx = static_cast<size_t>(-1);
    };

    struct Container {
        std::vector<HeapItem> heap;
        std::multimap<uint64_t, std::unique_ptr<Data>> map;
    };

    void add(Container& c, uint64_t val);
    void del(Container& c, uint64_t val);
    void verify(Container& c);
};


void HeapTest::add(Container& c, uint64_t val){
    Data* d = new Data();

    d->heap_idx = c.heap.size();

    c.map.insert(std::make_pair(val, d));
    
    HeapItem t{};
    t.val = val;
    t.ref = &d->heap_idx;

    c.heap.push_back(t);
    heap_update(c.heap.data(), d->heap_idx, c.heap.size());

}

void HeapTest::verify(Container& c){
    REQUIRE(c.map.size() == c.heap.size());

    for(size_t pos = 0; pos < c.heap.size(); pos++){
        size_t l = heap_left(pos);
        size_t r = heap_right(pos);

        REQUIRE((l >= c.heap.size() || c.heap[l].val >= c.heap[pos].val));
        REQUIRE((r >= c.heap.size() || c.heap[r].val >= c.heap[pos].val));
        REQUIRE(*c.heap[pos].ref == pos);
    }
}

void HeapTest::del(Container& c, uint64_t val){
    auto it = c.map.find(val);
    REQUIRE(it != c.map.end());
    Data* d = it->second.get();

    REQUIRE(c.heap.at(d->heap_idx).val == val);
    REQUIRE(c.heap.at(d->heap_idx).ref == &d->heap_idx);

    heap_del(c.heap, d->heap_idx);
    c.map.erase(it);
}

TEST_CASE_METHOD(HeapTest, "AddValues", "[heap]"){
    for(int i = 0; i < 50; i++){
        Container c;

        for(int j = 0; j < 50; j++){
            add(c, j);
        }

        verify(c);

        add(c, i);

        verify(c);
    }
}

TEST_CASE_METHOD(HeapTest, "DeleteValues", "[heap]"){
    for(int i = 0; i < 50; i++){
        Container c;

        for(int j = 0; j < 50; j++){
            add(c, j);
        }

        verify(c);

        del(c, i);

        verify(c);
    }
}