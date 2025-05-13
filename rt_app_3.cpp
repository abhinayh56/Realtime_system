#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <cstring>
#include <cmath>

#define NSEC_PER_SEC 1000000000L

const int frequency = 1000; // Target frequency in Hz
const long period_ns = NSEC_PER_SEC / frequency;
const int report_interval = 1000; // Report after this many iterations

inline long timespec_to_ns(const timespec &ts)
{
    return ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec;
}

void timespec_add_ns(timespec &ts, long ns)
{
    ts.tv_nsec += ns;
    while (ts.tv_nsec >= NSEC_PER_SEC)
    {
        ts.tv_nsec -= NSEC_PER_SEC;
        ts.tv_sec++;
    }
}

void *rt_task(void *arg)
{
    struct timespec next, now;
    clock_gettime(CLOCK_MONOTONIC, &next);

    long max_jitter = 0;
    long min_jitter = NSEC_PER_SEC;
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
            min_jitter = NSEC_PER_SEC;
            total_jitter = 0;
        }

        timespec_add_ns(next, period_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    }

    return nullptr;
}

int main()
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
        return 1;
    }

    pthread_join(thread, nullptr);

    return 0;
}