#include <timer_manager.h>

namespace timer_manager
{
    uint64_t get_monotonic_msec()
    {
        struct timespec ts = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return uint64_t(ts.tv_sec) * 1000 + ts.tv_nsec / 1000 / 1000;
    }

    int64_t next_timer_ms()
    {
        int64_t now_ms = get_monotonic_msec();

        int64_t next_ms = INT64_MAX;
        int64_t next_idle_ms = INT64_MAX;
        int64_t next_io_ms = INT64_MAX;
        int64_t next_ttl_ms = INT64_MAX;

        if (!dlist_empty(&g_data.idle_list))
        {
            Conn *conn = container_of(g_data.idle_list.next, Conn, idle_node);
            next_idle_ms = conn->last_active_ms + k_idle_timeout_ms;
        }

        if (!dlist_empty(&g_data.io_list))
        {
            Conn *conn = container_of(g_data.io_list.next, Conn, io_node);
            next_io_ms = conn->io_start_ms + k_io_timeout_ms;
        }

        if (!g_data.heap.empty() && g_data.heap[0].val < next_idle_ms)
        {
            next_ttl_ms = g_data.heap[0].val;
        }

        next_ms = std::min(next_idle_ms, std::min(next_io_ms, next_ttl_ms));
        if (next_ms == INT64_MAX)
            return -1;
        if (next_ms <= now_ms)
            return 0;
        return (next_ms - now_ms);
    }

    void process_timers()
    {
        int64_t now_ms = get_monotonic_msec();
        while (!dlist_empty(&g_data.idle_list))
        {
            Conn *conn = container_of(g_data.idle_list.next, Conn, idle_node);
            int64_t next_ms = conn->last_active_ms + k_idle_timeout_ms;
            if (next_ms >= now_ms)
                break;

            spdlog::warn("removing idle connection[connection timeout]: {}\n", conn->fd);
            destroy(conn);
        }

        while (!dlist_empty(&g_data.io_list))
        {
            Conn *conn = container_of(g_data.io_list.next, Conn, io_node);
            int64_t end_ms = conn->io_start_ms + k_io_timeout_ms;
            if (end_ms >= now_ms)
                break;

            spdlog::warn("removing idle connection[io timeout]: {}\n", conn->fd);
            destroy(conn);
        }

        size_t k_max_works = 2000;
        size_t n_works = 0;

        while (!g_data.heap.empty() && g_data.heap[0].val < now_ms && n_works < k_max_works)
        {
            Entry *ent = container_of(g_data.heap[0].ref, Entry, heap_idx);
            HNode *node = hm_delete(&g_data.db, &ent->node, &entry_eq);
            if (node != &ent->node)
            {
                spdlog::warn("key expired: {}\n", ent->key.c_str());
            }
            entry_del(ent);
            n_works++;
        }
    }

} // namespace timer_manager