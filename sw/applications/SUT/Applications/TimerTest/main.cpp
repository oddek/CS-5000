#include "SCHED_Deadline.h"
#include "Utils.h"
#include <fstream>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

enum class Mode
{
    Timer,
    Nanosleep,
    EDF,
    Timedwait
};

int64_t minLatency{INT64_MAX};
int64_t maxLatency{INT64_MIN};
double_t avgLatency{0.0};

static Mode mode;

static timespec totalStartTime{0};
static timespec totalStopTime{0};
static timespec wakeupTime{0};
static timespec lastTime{0};
struct timespec period;

#define HISTOGRAM_SIZE 5000
#define MAX_SAMPLES 60000

struct itimerval interval;
static unsigned long long histogram[HISTOGRAM_SIZE];
static size_t overflow{0U};

static size_t samples{0U};
bool done = false;

pthread_attr_t attr;
sched_attr sched_attr{0};
static sem_t tick_sem;
static pthread_t tick_thread;
static pthread_cond_t cond;
static pthread_condattr_t cond_attr;
static pthread_mutex_t mutex;

void* tick_thread_func(void* arg)
{
    if (mode == Mode::EDF)
    {
        sched_attr.size = sizeof(struct sched_attr);
        sched_attr.sched_flags = 0U;
        sched_attr.sched_priority = 0U;
        sched_attr.sched_period = 1 * 1000 * 1000;
        sched_attr.sched_deadline = 200 * 1000;
        sched_attr.sched_runtime = 200 * 1000;
        sched_attr.sched_policy = SCHED_DEADLINE;
        pid_t tid = gettid();

        const int ret = sched_setattr(tid, &sched_attr, 0);

        if (ret)
        {
            throw std::runtime_error("Sched_setattr error\n");
        }
    }

    totalStartTime = getTimespecNow();
    lastTime = totalStartTime;
    wakeupTime = totalStartTime;

    while (true)
    {
        if (mode == Mode::Timer)
        {
            sem_wait(&tick_sem);
        }

        auto currentTime = getTimespecNow();
        auto newLastTime = currentTime;
        timeSpecDecrement(currentTime, period);
        const auto diff = timeSpecDiffUs(currentTime, lastTime);

        if (samples > 100)
        {
            if (abs(diff) > HISTOGRAM_SIZE / 2)
            {
                overflow++;
            }
            else
            {
                histogram[diff + HISTOGRAM_SIZE / 2]++;
            }

            if( diff < minLatency) minLatency = diff;
            if( diff > maxLatency) maxLatency = diff;
            avgLatency += ((double_t)diff - avgLatency) / ((samples-100) + 1);
        }

        lastTime = newLastTime;

        if (++samples > MAX_SAMPLES)
        {
            if (mode == Mode::Timer)
            {
                interval.it_value.tv_usec = 0;
                interval.it_value.tv_sec = 0;
                setitimer(ITIMER_REAL, &interval, NULL);
            }
            done = true;
            totalStopTime = getTimespecNow();
            return NULL;
        }

        if (mode == Mode::Nanosleep)
        {
            timeSpecIncrement( wakeupTime, period);
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, nullptr);
        }
        else if (mode == Mode::EDF)
        {
            sched_yield();
        }
        else if(mode == Mode::Timedwait)
        {
            timeSpecIncrement( wakeupTime, period);
            pthread_mutex_lock(&mutex);
            pthread_cond_timedwait(&cond, &mutex, &wakeupTime);
            pthread_mutex_unlock(&mutex);
        }
    }
}

void sig_callback(int signal)
{
    if (signal == SIGALRM)
    {
        sem_post(&tick_sem);
    }
}

int main(int argc, char* argv[])
{
    const std::string modeArg = argv[1];
    bool rtSched{false};

    if (modeArg == "timer")
    {
        mode = Mode::Timer;
    }
    else if (modeArg == "nanosleep")
    {
        mode = Mode::Nanosleep;
    }
    else if (modeArg == "edf")
    {
        mode = Mode::EDF;
    }
    else if (modeArg == "timedwait")
    {
        mode = Mode::Timedwait;
    }

    if (argc > 2 && std::string(argv[2]) == "rt")
    {
        rtSched = true;
    }

    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_usec = 1000;
    interval.it_value.tv_sec = 0;
    interval.it_value.tv_usec = 1000;

    period.tv_nsec = interval.it_interval.tv_usec * 1000U;
    period.tv_sec = interval.it_interval.tv_sec;

    if (mode == Mode::Timer)
    {
        sem_init(&tick_sem, 0, 0);

        sighandler_t sigres = signal(SIGALRM, &sig_callback);
        if (sigres == SIG_ERR)
        {
            fmt::println("Error sigres");
        }
    }

    if( mode == Mode::Timedwait)
    {
        errno = pthread_condattr_init (&cond_attr);
        if (errno) {
            perror ("pthread_condattr_init");
            return -1;
        }

        errno = pthread_condattr_setclock (&cond_attr, CLOCK_MONOTONIC);
        if (errno) {
            perror ("pthread_condattr_setclock");
            return -1;
        }

        errno = pthread_cond_init (&cond, &cond_attr);
        if (errno) {
            perror ("pthread_cond_init");
            return -1;
        }

        if (pthread_mutex_init(&mutex, NULL) != 0) {
            printf("\n mutex init has failed\n");
            return 1;
        }
    }


    if (pthread_attr_init(&attr) != 0)
    {
        throw std::runtime_error("Failed to init pthread attributes");
    }

    if (mode != Mode::EDF && rtSched)
    {
        if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0)
        {
            throw std::runtime_error("Failed to set pthread scheduling policy");
        }

        static constexpr sched_param param{81};

        if (pthread_attr_setschedparam(&attr, &param) != 0)
        {
            throw std::runtime_error("Failed to set pthread scheduling parameters");
        }

        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0)
        {
            throw std::runtime_error("Failed to set pthread inherit sched");
        }
    }

    pthread_create(&tick_thread, &attr, tick_thread_func, NULL);

    if (mode == Mode::Timer)
    {
        if (int res = setitimer(ITIMER_REAL, &interval, NULL))
        {
            fmt::println("Error setittimer: {}", res);
        }
    }

    while (true)
    {
        sleep(5);
        if (done)
        {
            const std::string fullFilename = "timerTestHistogram";
            std::ofstream file;
            file.open(fullFilename.data());

            for (const auto& val : histogram)
            {
                file << std::to_string(val) << "\n";
            }

            file.close();
            fmt::println("Overflowed {}", overflow);

            auto totalTime = timeSpecDiffUs(totalStopTime, totalStartTime);
            auto averagePeriod = static_cast<double_t>(totalTime) / MAX_SAMPLES;
            fmt::println("TotalTime {}", totalTime);
            fmt::println("AveragePeriod {}", averagePeriod);
            fmt::println("MinLatency {}", minLatency);
            fmt::println("MaxLatency {}", maxLatency);
            fmt::println("AvgLatency {}", avgLatency);

            return 0;
        }
    }
}