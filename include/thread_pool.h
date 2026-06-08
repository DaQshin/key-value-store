#pragma once

#include <stddef.h>
#include <pthread.h>
#include <vector>
#include <deque>

struct Work {
    void (*func)(void*) = nullptr;
    void* arg = nullptr;
};

struct ThreadPool{
    std::vector<pthread_t> threads;
    std::deque<Work> queue;
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
};

int32_t thread_pool_init(ThreadPool* thread_pool, size_t num_threads);
void thread_pool_queue(ThreadPool* thread_pool, void (*func)(void*), void* arg);