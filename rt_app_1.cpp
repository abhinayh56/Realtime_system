#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

void *realtime_task(void *arg)
{
    while (true)
    {
        std::cout << "Running rt app 1" << std::endl;
        usleep(1000000);
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

    int ret = pthread_create(&thread, &attr, realtime_task, nullptr);
    if(ret != 0)
    {
        perror("pthread_create");
        return 1;
    }

    pthread_join(thread, nullptr);

    return 0;
}
