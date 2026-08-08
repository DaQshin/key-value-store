#include "hashtable.h"
#include "utils/utils"
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <unordered_map>

class HashtableTest {
protected:

    struct Data{
        std::string key;
        std::string value;
        HNode node;
    };

    struct Container{
        struct HTable htable;
        std::vector<std::vector<Data>> data;
        std::unordered_map<std::string, std::string> ref_table;
    };

    void init_table(Container& c, size_t n);
    void add(Container& c, std::string key, std::string val);
    void del(Container& c, std::string val);
    void verify(Container& c);
    void dispose(Container& c);

    bool data_eq(HNode* lhs, HNode* rhs){
        struct Data* l = container_of(lhs, struct Data, node);
        struct Data* r = container_of(rhs, struct Data, node);
        return l->key == r->key && l->value == r->value;
    }
};

void HashtableTest::init_table(Container& c, size_t n){
    REQUIRE(n > 0 && ((n - 1) & n) == 0);
    h_init(&c.htable, n);
    c.ref_table.resize(n);
}

void HashtableTest::dispose(Container& c){
    delete c.htable;
    c.data.clear();
    c.ref_table.clear();
}

void HashtableTest::add(Container& c, std::string key, std::string val){
    struct Data d{};
    d.key = key;
    d.value = val;
    d.node.hash = (uint64_t)str_hash((const uint8_t*)key.data(), key.size());
    h_insert(&c.htable, &d.node);
    size_t idx = d.node.hash & (c.data.size() - 1);
    c.data[idx].push_back(d);
    c.ref_table[key] = value;

}

void HashtableTest::del(Container& c, HNode* node){
    HNode** from = h_lookup(&c.htable, node, &data_eq);
    
    ASSERT(*from != nullptr);

    h_detach(&c.htable, from);
}

void verify(Container& c){
    ASSERT(c.htable.table.size() == c.size());
    
    for(auto& p: c.ref_table){
        HNode* node = new HNode();
        node->hash = (uint64_t)str_hash((const uint8_t*)p.first.data(), p.first.size());
        size_t idx = node->hash & (data.size() - 1);
        
    }
}