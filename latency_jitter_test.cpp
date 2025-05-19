#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <sched.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <ctime>
#include <sys/mman.h>
#include <pthread.h>

constexpr int RT_PRIORITY   = 80;       // SCHED_FIFO real-time priority
constexpr int NUM_ITERATIONS = 5000;
constexpr int INTERVAL_US    = 1000;    // 1ms (1000 microseconds)

inline long timespec_to_us(const timespec& ts) {
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

inline long diff_us(const timespec& start, const timespec& end) {
    return timespec_to_us(end) - timespec_to_us(start);
}

bool lock_memory() {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        perror("mlockall");
        return false;
    }
    return true;
}

bool set_thread_affinity_and_priority(pthread_t thread, int cpu, int priority) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return false;
    }

    sched_param sch_params{};
    sch_params.sched_priority = priority;
    if (pthread_setschedparam(thread, SCHED_FIFO, &sch_params) != 0) {
        perror("pthread_setschedparam");
        return false;
    }

    return true;
}

void realtime_loop(int cpu) {
    std::cout << "Thread running on CPU " << cpu << " with RT priority " << RT_PRIORITY << "\n";

    if (!lock_memory()) {
        std::cerr << "Failed to lock memory.\n";
        return;
    }

    timespec t_next, t_start;
    clock_gettime(CLOCK_MONOTONIC, &t_next);

    long min_latency = 1e6, max_latency = 0, total_latency = 0;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        // Calculate next wake-up time
        t_next.tv_nsec += INTERVAL_US * 1000;
        while (t_next.tv_nsec >= 1000000000) {
            t_next.tv_sec += 1;
            t_next.tv_nsec -= 1000000000;
        }

        int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next, nullptr);
        if (ret != 0) {
            std::cerr << "clock_nanosleep failed at iteration " << i << ": " << strerror(ret) << "\n";
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &t_start);
        long latency = diff_us(t_next, t_start);

        min_latency = std::min(min_latency, latency);
        max_latency = std::max(max_latency, latency);
        total_latency += latency;
    }

    std::cout << "Iterations     : " << NUM_ITERATIONS << "\n";
    std::cout << "Interval (us)  : " << INTERVAL_US << "\n";
    std::cout << "Min Latency    : " << min_latency << " us\n";
    std::cout << "Max Latency    : " << max_latency << " us\n";
    std::cout << "Avg Latency    : " << total_latency / NUM_ITERATIONS << " us\n";
    std::cout << "Jitter (max-min): " << max_latency - min_latency << " us\n";
}

int main(int argc, char* argv[]) {
    int cpu = 0;
    if (argc > 1) {
        cpu = std::stoi(argv[1]);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Set stack size (optional, but can help with predictability)
    pthread_attr_setstacksize(&attr, 1024 * 1024);

    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    sched_param sch_params{};
    sch_params.sched_priority = RT_PRIORITY;
    pthread_attr_setschedparam(&attr, &sch_params);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    pthread_t thread;
    auto thread_func = [](void* arg) -> void* {
        int cpu_id = *reinterpret_cast<int*>(arg);
        set_thread_affinity_and_priority(pthread_self(), cpu_id, RT_PRIORITY);
        realtime_loop(cpu_id);
        return nullptr;
    };

    if (pthread_create(&thread, &attr, thread_func, &cpu) != 0) {
        perror("pthread_create");
        return 1;
    }

    pthread_join(thread, nullptr);
    return 0;
}
