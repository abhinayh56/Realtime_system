#include "Rt_scheduler.h"

Rt_scheduler::Rt_scheduler()
{
}

Rt_scheduler::~Rt_scheduler()
{
}

void *Rt_scheduler::rt_task(void *arg)
{
    pin_thread_to_cores({5});

    struct timespec next, now;
    clock_gettime(CLOCK_MONOTONIC, &next);

    long max_jitter = 0;
    long min_jitter = nsec_per_sec;
    long total_jitter = 0;
    long loop_count = 0;

    while (true)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        long latency = timespec_to_ns(now) - timespec_to_ns(next);

        long jitter = std::abs(latency);
        if (jitter > max_jitter)
            max_jitter = jitter;
        if (jitter < min_jitter)
            min_jitter = jitter;
        total_jitter += jitter;

        // === Your real-time task ===
        // Do nothing or simulate work
        // ===========================

        loop_count++;
        if (loop_count % report_interval == 0)
        {
            double avg_jitter = total_jitter / (double)report_interval;
            std::cout << "[RT] Loop " << loop_count
                      << " | Max Jitter: " << max_jitter / 1e3 << " us"
                      << " | Min: " << min_jitter / 1e3 << " us"
                      << " | Avg: " << avg_jitter / 1e3 << " us"
                      << std::endl;

            // Reset stats
            max_jitter = 0;
            min_jitter = nsec_per_sec;
            total_jitter = 0;
        }

        timespec_add_ns(next, period_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    }

    return nullptr;
}

void Rt_scheduler::main_fun()
{
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    param.sched_priority = 99;
    pthread_attr_setschedparam(&attr, &param);

    int ret = pthread_create(&thread, &attr, rt_task, nullptr);
    if (ret != 0)
    {
        std::cerr << "Failed to create thread: " << ret << std::endl;
    }
    pthread_join(thread, nullptr);
}

void Rt_scheduler::pin_thread_to_cores(const std::vector<int> &core_ids)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    for (int core_id : core_ids)
    {
        CPU_SET(core_id, &cpuset);
    }

    pthread_t current_thread = pthread_self();
    int ret = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (ret != 0)
    {
        std::cerr << "Error setting CPU affinity: " << strerror(ret) << std::endl;
    }
    else
    {
        std::cout << "Thread pinned to cores: ";
        for (int core_id : core_ids)
        {
            std::cout << core_id << " ";
        }
        std::cout << std::endl;
    }
}

inline long Rt_scheduler::timespec_to_ns(const timespec &ts)
{
    return ts.tv_sec * nsec_per_sec + ts.tv_nsec;
}

void Rt_scheduler::timespec_add_ns(timespec &ts, long ns)
{
    ts.tv_nsec += ns;
    while (ts.tv_nsec >= nsec_per_sec)
    {
        ts.tv_nsec -= nsec_per_sec;
        ts.tv_sec++;
    }
}
