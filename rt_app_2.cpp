#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define NSEC_PER_SEC 1000000000L
#define FREQ 1000

const long period_ns = NSEC_PER_SEC / FREQ;

void timespec_add_ns(timespec &ts, long ns)
{
    ts.tv_nsec += ns;
    while (ts.tv_nsec >= NSEC_PER_SEC)
    {
        ts.tv_nsec -= NSEC_PER_SEC;
        ts.tv_nsec++;
    }
}

void *rt_task(void *arg)
{
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (true)
    {
        std::cout << "Running rt app 2" << std::endl;

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

    param.sched_priority = 90;
    pthread_attr_setschedparam(&attr, &param);

    int ret = pthread_create(&thread, &attr, rt_task, nullptr);
    if(ret != 0)
    {
        std::cerr << "Failed to create thread: " << ret << std::endl;
        return 1;
    }

    pthread_join(thread, nullptr);

    return 0;
}