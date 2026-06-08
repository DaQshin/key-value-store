
#include <stdint.h>
#include "thread_pool.h"


static void* worker(void* arg){
    ThreadPool* tp = (ThreadPool*)arg;
    while(true){

        pthread_mutex_lock(&tp->mu);

        while(tp->queue.empty()){
            pthread_cond_wait(&tp->not_empty, &tp->mu);
        }

        Work w = tp->queue.front();
        tp->queue.pop_front();

        pthread_mutex_unlock(&tp->mu);

        w.func(w.arg);

    }

    return NULL;
}

int32_t thread_pool_init(ThreadPool* tp, size_t num_threads){
    if(num_threads <= 0) return -1;

    if(pthread_mutex_init(&tp->mu, NULL) < 0) return -1;

    if(pthread_cond_init(&tp->not_empty, NULL) < 0) return -1;

    tp->threads.resize(num_threads);
    for(size_t i = 0; i < num_threads; i++){
        if(pthread_create(&tp->threads[i], NULL, &worker, tp) < 0) return -1;
    }
    return 0;
}

void thread_pool_queue(ThreadPool* tp, void (*func)(void*), void* arg){
    pthread_mutex_lock(&tp->mu);
    tp->queue.push_back(Work {func, arg});
    pthread_cond_signal(&tp->not_empty);
    pthread_mutex_unlock(&tp->mu);
}