#pragma once

#define container_of(ptr, T, member) \
    ((T*)((char*)(ptr) - offsetof(T, member)))

inline uint32_t str_hash(const uint8_t* data, size_t len){
    // FNV-1a
    uint32_t h = 0x811C9DC5;
    for(size_t i = 0; i < len; i++){
        h = (h ^ data[i]) * 0x01000193;
    }

    return h;
}