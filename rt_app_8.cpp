#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <atomic>
#include <vector>
#include <numeric>

std::atomic<bool> running(true);

void signal_handler(int) {
    running = false;
}

// Pin thread to specific core
void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

// Set real-time priority
void set_realtime_priority(int priority) {
    sched_param sch_params;
    sch_params.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_params) != 0) {
        std::cerr << "Failed to set SCHED_FIFO. Run with sudo.\n";
        exit(EXIT_FAILURE);
    }
}

void realtime_loop(int core_id, int priority, int frequency_hz) {
    pin_thread_to_core(core_id);
    set_realtime_priority(priority);

    const int period_us = 1'000'000 / frequency_hz;
    const auto period = std::chrono::microseconds(period_us);

    std::vector<long> jitters;
    jitters.reserve(100000);

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (running) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        long dt_us = (now.tv_sec - next.tv_sec) * 1'000'000 +
                     (now.tv_nsec - next.tv_nsec) / 1'000;
        jitters.push_back(std::abs(dt_us));

        // Do RT work here
        // Example: read sensors, run control, log data, etc.

        // Schedule next execution time
        next.tv_nsec += period.count() * 1000;
        while (next.tv_nsec >= 1'000'000'000) {
            next.tv_nsec -= 1'000'000'000;
            next.tv_sec += 1;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    }

    // Compute jitter statistics
    if (!jitters.empty()) {
        auto [min_it, max_it] = std::minmax_element(jitters.begin(), jitters.end());
        double avg = std::accumulate(jitters.begin(), jitters.end(), 0.0) / jitters.size();

        std::cout << "\nJitter statistics (in microseconds):\n";
        std::cout << "  Min: " << *min_it << "\n";
        std::cout << "  Max: " << *max_it << "\n";
        std::cout << "  Avg: " << avg << "\n";
    }
}

int main() {
    std::signal(SIGINT, signal_handler);

    int rt_core = 4;       // Isolated core
    int rt_priority = 80;  // SCHED_FIFO priority (1-99)
    int frequency = 500;   // Hz

    std::cout << "Starting real-time loop on core " << rt_core
              << " with priority " << rt_priority
              << " at " << frequency << " Hz\n";

    realtime_loop(rt_core, rt_priority, frequency);

    std::cout << "Real-time loop terminated.\n";
    return 0;
}
