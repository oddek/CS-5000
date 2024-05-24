#include "Utils.h"

static struct rusage usage;
size_t samples{0};
constexpr size_t maxSamples{10*60};

void* worker(void* arg)
{
    getrusage(RUSAGE_THREAD, &usage);
    fmt::println("Initial Values:");
    fmt::println("Voluntary preempt {}, Preempted {}", usage.ru_nvcsw, usage.ru_nivcsw);

    const auto totalStartTime = std::chrono::steady_clock::now();

    while(true)
    {
        const auto startTime = getTimespecNow();

        while (true)
        {
            const auto now = getTimespecNow();

            if(timeSpecDiffUs(now, startTime) > (500U * 1000U))
            {
                break;
            }
        }


        if (samples++ == maxSamples)
        {
            break;
        }

        usleep(500U*1000U);
    }

    printRunTime(totalStartTime);

    const auto now = std::chrono::steady_clock::now();
    const auto time_elapsed = now - totalStartTime;

    auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed);

    fmt::println("Total millis {}", milliSeconds.count());

    getrusage(RUSAGE_THREAD, &usage);
    fmt::println("Final Values:");
    fmt::println("Voluntary preempt {}, Preempted {}", usage.ru_nvcsw, usage.ru_nivcsw);

    return NULL;
}

int main()
{
    pthread_t thread;
    pthread_attr_t thread_attr;

    if (pthread_attr_init(&thread_attr) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to init pthread attributes");
    }

    if (pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set pthread stack size");
    }

    if (pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set pthread scheduling policy");
    }

    static constexpr sched_param realTimePriority{99};

    if (pthread_attr_setschedparam(&thread_attr, &realTimePriority) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set pthread scheduling parameters");
    }

    if (pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set pthread inherit sched");
    }

    if (pthread_create(&thread, &thread_attr, worker, NULL) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to create Thread");
    }

    pthread_join(thread, NULL);

    return 0;
}