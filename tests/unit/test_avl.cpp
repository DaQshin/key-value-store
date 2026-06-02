#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <set>
#include "avl.h"
#include "utils.h"
#include <gtest/gtest.h>

class AVLTest: public ::testing::Test {
protected:
    struct Data{
        AVLNode node;
        uint32_t val = 0;
    };

    struct Container {
        AVLNode* root = nullptr;
    };

    static void add(Container& c, uint32_t val);
    static bool erase(Container& c, uint32_t val);
    static void avl_verify(AVLNode* parent, AVLNode* node);
    static void extract(AVLNode* node, std::multiset<uint32_t>& extracted);
    static void container_verify(Container& c, std::multiset<uint32_t>& ref);
    static void dispose(Container& c);
    static void insert(uint32_t sz);
    static void insert_dup(uint32_t sz);
    static void remove(uint32_t sz);
};

void AVLTest::add(Container& c, uint32_t val){
    Data* data = new Data();
    avl_init(&data->node);
    data->val = val;

    AVLNode* cur = nullptr;
    AVLNode** from = &c.root;
    while(*from){
        cur = *from;
        uint32_t node_val = container_of(cur, Data, node)->val;
        from = (val < node_val) ? &cur->left : &cur->right;
    }

    *from = &data->node;
    data->node.parent = cur;
    c.root = avl_fix(&data->node);
}

bool AVLTest::erase(Container& c, uint32_t val){
    AVLNode* cur = c.root;
    while(cur){
        uint32_t node_val = container_of(cur, Data, node)->val;
        if(node_val == val) break;
        
        cur = val < node_val ? cur->left : cur->right;
    }

    if(!cur) return 0;

    c.root = avl_del(cur);
    delete container_of(cur, Data, node);
    return 1;
}

void AVLTest::avl_verify(AVLNode* parent, AVLNode* node){
    if(!node) return;

    ASSERT_EQ(node->parent, parent) << "Node parent mismatch";
    avl_verify(node, node->left);
    avl_verify(node, node->right);

    ASSERT_EQ(node->count, 1 + avl_count(node->left) + avl_count(node->right))
    << "Invalid count of the nodes in the left and right subtree";

    uint32_t lh = avl_height(node->left);
    uint32_t rh = avl_height(node->right);

    ASSERT_TRUE(lh == rh || lh + 1 == rh || rh + 1 == lh);
    ASSERT_EQ(node->height, 1 + std::max(lh, rh));

    uint32_t val = container_of(node, Data, node)->val;
    if(node->left){
        ASSERT_EQ(node->left->parent, node);
        ASSERT_LE(container_of(node->left, Data, node)->val, val);
    }
    if(node->right){
        ASSERT_EQ(node->right->parent, node);
        ASSERT_GE(container_of(node->right, Data, node)->val, val);
    }
}

void AVLTest::extract(AVLNode* node, std::multiset<uint32_t>& extracted){
    if(!node) return;

    extract(node->left, extracted);
    extracted.insert(container_of(node, Data, node)->val);
    extract(node->right, extracted);
}

void AVLTest::container_verify(Container& c, std::multiset<uint32_t>& ref){
    avl_verify(nullptr, c.root);
    ASSERT_EQ(avl_count(c.root), ref.size()) 
    << "Number of nodes in the tree != number of elements in the reference set";
    std::multiset<uint32_t> extracted;
    extract(c.root, extracted);
    ASSERT_EQ(extracted, ref) << "some extracted nodes from the tree are not present in the reference set";
}

void AVLTest::dispose(Container& c){
    while(c.root){
        AVLNode* node = c.root;
        c.root = avl_del(c.root);
        delete container_of(node, Data, node);
    }
}

void AVLTest::insert(uint32_t sz){
    for(uint32_t val = 0; val < sz; val++){
        Container c;
        std::multiset<uint32_t> ref;
        for(uint32_t i = 0; i < sz; i++){
            if(i == val) continue;
            
            add(c, i);
            ref.insert(i);
        }

        container_verify(c, ref);

        add(c, val);
        ref.insert(val);
        container_verify(c, ref);
        dispose(c);
    }
}

void AVLTest::insert_dup(uint32_t sz) {
    for (uint32_t val = 0; val < sz; ++val) {
        Container c;
        std::multiset<uint32_t> ref;
        for (uint32_t i = 0; i < sz; ++i) {
            add(c, i);
            ref.insert(i);
        }
        container_verify(c, ref);

        add(c, val);
        ref.insert(val);
        container_verify(c, ref);
        dispose(c);
    }
}

void AVLTest::remove(uint32_t sz){
    for (uint32_t val = 0; val < sz; ++val) {
        Container c;
        std::multiset<uint32_t> ref;
        for (uint32_t i = 0; i < sz; ++i) {
            add(c, i);
            ref.insert(i);
        }
        container_verify(c, ref);

        EXPECT_TRUE(erase(c, val)) << "Failed to erase value" << val;
        ref.erase(val);
        container_verify(c, ref);
        dispose(c);
    }
}

TEST_F(AVLTest, BasicOperation){
    Container c;
    std::multiset<uint32_t> ref;

    container_verify(c, ref);

    add(c, 123);
    ref.insert(123);

    container_verify(c, ref);

    EXPECT_FALSE(!erase(c, 124));
    EXPECT_TRUE(erase(c, 123));

    ref.erase(123);

    container_verify(c, ref);

    dispose(c);
}

TEST_F(AVLTest, SequentialInsertion){

    Container c;
    std::multiset<uint32_t> ref;

    for (uint32_t i = 0; i < 1000; i += 3) {
        add(c, i);
        ref.insert(i);
        container_verify(c, ref);
    }

    dispose(c);
}

TEST_F(AVLTest, RandomInsertion){

    Container c;
    std::multiset<uint32_t> ref;

    for (uint32_t i = 0; i < 100; i++) {
        uint32_t val = (uint32_t)rand() % 1000;
        add(c, val);
        ref.insert(val);
        container_verify(c, ref);
    }

    dispose(c);
}

TEST_F(AVLTest, RandomDeletion){

    Container c;
    std::multiset<uint32_t> ref;

    for (uint32_t i = 0; i < 200; i++) {
        uint32_t val = (uint32_t)rand() % 1000;
        auto it = ref.find(val);
        if (it == ref.end()) {
            EXPECT_FALSE(!erase(c, val));
        } else {
            EXPECT_TRUE(erase(c, val));
            ref.erase(it);
        }
        container_verify(c, ref);
    }

    dispose(c);
}

TEST_F(AVLTest, InsertDeleteAtEveryPosition){
    Container c;
    std::multiset<uint32_t> ref;

    for (uint32_t i = 0; i < 200; ++i) {
        insert(i);
        insert_dup(i);
        remove(i);
    }

    dispose(c);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}