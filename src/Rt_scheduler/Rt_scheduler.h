#ifndef RT_SCHEDULER_H
#define RT_SCHEDULER_H

#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <cmath>
#include <vector>
#include <stdint.h>
#include <string.h>

class Rt_scheduler
{
public:
    Rt_scheduler();
    ~Rt_scheduler();
    void *rt_task(void *arg);
    void main_fun();

private:
    const unsigned long nsec_per_sec = 1000000000L;
    uint64_t loop_freq = 1000;
    uint64_t period_ns = nsec_per_sec / loop_freq;
    uint64_t report_interval = 1000;

    void pin_thread_to_cores(const std::vector<int> &core_ids);
    inline long timespec_to_ns(const timespec &ts);
    void timespec_add_ns(timespec &ts, long ns);
};

#endif // RT_SCHEDULER_H
