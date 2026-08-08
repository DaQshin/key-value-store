#pragma once

#include <cstddef>
#include <cstdint>

namespace hashtable
{
    struct HNode
    {
        HNode *next = nullptr;
        std::uint64_t hash = 0;
    };

    struct HTable
    {
        HNode **table = nullptr;
        std::size_t mask = 0;
        std::size_t size = 0;
    };

    struct HMap
    {
        HTable older;
        HTable newer;
        std::size_t migrate_pos = 0;
    };

    HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
    void hm_insert(HMap *hmap, HNode *node);
    HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
    void hm_clear(HMap *hmap);
    std::size_t hm_size(HMap *hmap);

}
