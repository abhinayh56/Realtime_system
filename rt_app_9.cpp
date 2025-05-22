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
#include <climits>
#include <cmath>

std::atomic<bool> running(true);

// Signal handler to terminate the loop safely
void signal_handler(int) {
    running = false;
}

// Pin thread to a specific CPU core
void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        exit(EXIT_FAILURE);
    }
}

// Set SCHED_FIFO priority
void set_realtime_priority(int priority) {
    sched_param sch_params;
    sch_params.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_params) != 0) {
        std::cerr << "Failed to set SCHED_FIFO. Run with sudo.\n";
        exit(EXIT_FAILURE);
    }
}

// Real-time loop running at fixed frequency
void realtime_loop(int core_id, int priority, int frequency_hz) {
    pin_thread_to_core(core_id);
    set_realtime_priority(priority);

    const int period_us = 1'000'000 / frequency_hz;
    const auto period = std::chrono::microseconds(period_us);

    long min_jitter = LONG_MAX;
    long max_jitter = 0;
    long jitter_sum = 0;
    size_t count = 0;

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    auto last_print = std::chrono::steady_clock::now();

    while (running) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        long dt_us = (now.tv_sec - next.tv_sec) * 1'000'000 +
                     (now.tv_nsec - next.tv_nsec) / 1'000;
        long abs_jitter = std::abs(dt_us);

        jitter_sum += abs_jitter;
        count++;
        min_jitter = std::min(min_jitter, abs_jitter);
        max_jitter = std::max(max_jitter, abs_jitter);

        // Periodic jitter print every second
        auto now_time = std::chrono::steady_clock::now();
        if (now_time - last_print >= std::chrono::seconds(1)) {
            double avg = count > 0 ? static_cast<double>(jitter_sum) / count : 0;
            std::cout << "Jitter (us): min=" << min_jitter
                      << " max=" << max_jitter
                      << " avg=" << avg << std::endl;

            // Reset stats
            min_jitter = LONG_MAX;
            max_jitter = 0;
            jitter_sum = 0;
            count = 0;
            last_print = now_time;
        }

        // RT application logic would go here
        // For example: read sensors, run control algorithm, etc.

        // Calculate next execution time
        next.tv_nsec += period.count() * 1000;
        while (next.tv_nsec >= 1'000'000'000) {
            next.tv_nsec -= 1'000'000'000;
            next.tv_sec += 1;
        }

        // Sleep until next period
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    }

    std::cout << "Real-time loop exited.\n";
}

int main() {
    std::signal(SIGINT, signal_handler);

    int rt_core = 2;       // Change based on your isolated CPU
    int rt_priority = 99;  // SCHED_FIFO priority (1-99)
    int frequency = 500;   // Loop frequency in Hz

    std::cout << "Starting real-time loop on core " << rt_core
              << " with priority " << rt_priority
              << " at " << frequency << " Hz\n";

    realtime_loop(rt_core, rt_priority, frequency);

    return 0;
}
