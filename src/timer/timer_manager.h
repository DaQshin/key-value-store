#pragma once

#include <heap.h>
#include <list.h>

namespace timer_manager
{
    const uint64_t k_idle_timeout_ms = 30 * 1000;
    const uint64_t k_io_timeout_ms = 10 * 1000;

    uint64_t get_monotonic_msec();
    int64_t next_timer_ms();
    void process_timers();

} // namespace timer_manager