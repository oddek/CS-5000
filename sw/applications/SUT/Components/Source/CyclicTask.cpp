
#include "CyclicTask.h"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include "Cpu.h"

static void* worker(void* arg)
{
    auto* rxWorker = static_cast<CyclicTask*>(arg);
    rxWorker->run();
    return nullptr;
}

void CyclicTask::saveHistogramData(const std::string_view& filename)
{
    const std::string fullFilename = taskName + "_" + std::string(filename);
    std::ofstream file;
    file.open(fullFilename.data());

    for (const auto& val : histogram)
    {
        file << std::to_string(val) << "\n";
    }

    file.close();
    fmt::println("CyclicTask: {} histogram saved to {}", taskName, fullFilename);
}

void CyclicTask::printFinalStats()
{
    fmt::println("**************");
    fmt::println("{}Task Stats", taskName);
    fmt::println("**************");

    fmt::println("Latency:");
    fmt::println("{:>6} {} us", "Min:", latencyData.min.load());
    fmt::println("{:>6} {} us", "Max:", latencyData.max.load());
    fmt::println("{:>6} {:.0f} us", "Avg:", latencyData.average / static_cast<double>(latencyData.samples));

    fmt::println("Execution Time:");
    fmt::println("{:>6} {} us", "Min:", executionTimeData.min.load());
    fmt::println("{:>6} {} us", "Max:", executionTimeData.max.load());
    fmt::println("{:>6} {:.0f} us", "Avg:", executionTimeData.average / static_cast<double>(executionTimeData.samples));
    fmt::println("");

    fmt::println("Missed Periods:");
    fmt::println("{:<14} {}", "  Total:", missedPeriods);
    fmt::println("{:<14} {}", "  Max Single:", maxSingleMissedPeriods);
    fmt::println("");

    if (histogramOverflow > 0)
    {
        fmt::println("WARNING: Histogram overflowed {} times\n", histogramOverflow);
    }
}

void CyclicTask::updateTimeData(TimeData& timeData, uint32_t newValue)
{
    timeData.current = newValue;

    if (newValue > timeData.max)
    {
        timeData.max = newValue;
    }

    if (newValue < timeData.min)
    {
        timeData.min = newValue;
    }

    timeData.average += newValue;
    timeData.samples++;
}

void CyclicTask::updateLatencyData()
{
//    const auto now = getTimespecNow();
    const auto diff = timeSpecDiffUs(currWakeUpTime, nextPeriod);

    if (diff < 0)
    {
        throw std::runtime_error("CyclicTask: Negative Latency detected");
    }

    const auto latency = static_cast<uint32_t>(diff);
    updateTimeData(latencyData, latency);
    addToHistogram(latency);
}

void CyclicTask::addToHistogram(uint32_t newValue)
{
    if (newValue < histogram.size())
    {
        histogram[newValue]++;
    }

    else
    {
        histogramOverflow++;
    }
}

void CyclicTask::updateExecutionTimeData(const struct timespec& taskStartTime)
{
    const auto now = getTimespecNow();
    const int64_t diff = timeSpecDiffUs(now, taskStartTime);

    if (diff < 0)
    {
        throw std::runtime_error("CyclicTask: Negative Execution Time Detected");
    }
    const auto executionTime = static_cast<uint32_t>(diff);

    updateTimeData(executionTimeData, executionTime);
}

void CyclicTask::sleep()
{
    if( useRunMore && runMoreActive)
    {
        runMoreActive = false;
        return;
    }

    if (aperiodicSleep)
    {
        nextPeriod = getTimespecNow();
        timeSpecIncrement(nextPeriod, period);

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextPeriod, nullptr);

//        pthread_mutex_lock(&mutex);
//        pthread_cond_timedwait(&cond, &mutex, &nextPeriod);
//        pthread_mutex_unlock(&mutex);
    }
    else
    {
//        nextPeriod = prevWakeUpTime;
        timeSpecIncrement(nextPeriod, period);
        const auto now = getTimespecNow();
        size_t currMissPeriods = 0U;
        while(timeSpecGreater(now, nextPeriod))
        {
            timeSpecIncrement(nextPeriod, period);
            missedPeriods++;
            currMissPeriods++;
        }

        if( currMissPeriods > maxSingleMissedPeriods)
        {
            maxSingleMissedPeriods = currMissPeriods;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextPeriod, nullptr);
    }
    clock_gettime(CLOCK_MONOTONIC, &currWakeUpTime);

    if (!dontCalcLatency)
    {
        updateLatencyData();
    }

    prevWakeUpTime = currWakeUpTime;
//    clock_gettime(CLOCK_MONOTONIC, &prevWakeUpTime);
}

void CyclicTask::printExecutionTimeData()
{
    printTimeData(executionTimeData, "Exe");
}

void CyclicTask::printLatencyData()
{
    printTimeData(latencyData, "Lat");
}

void CyclicTask::printResourceData()
{
    fmt::println("Missed Single Per {:4}, Missed Total Per {:4}", maxSingleMissedPeriods, missedPeriods);
}

void CyclicTask::printCpuData()
{
    cpuMonitor.printStats();
}

void CyclicTask::printFinalCpuData()
{
    cpuMonitor.printFinalStats();
}

void CyclicTask::printTimeData(TimeData& timeData, const char* prefix)
{
    fmt::println("{} - {} - Curr: {:8} us  "
                 "Min: {:8} us  "
                 "Max: {:8} us  "
                 "Avg: {:8.0f} us",
                 taskName, prefix, timeData.current.load(), timeData.min.load(), timeData.max.load(),
                 (timeData.average / static_cast<double>(timeData.samples)));
}

void CyclicTask::kill()
{
    stopRunning = true;
    int ret = pthread_join(thread, nullptr);

    if( ret)
    {
        fmt::println("Pthread join failed on {}, with {}", taskName, ret);
    }
}

void CyclicTask::run()
{
    cpuMonitor.setPid(get_task_id());
    clock_gettime(CLOCK_MONOTONIC, &prevWakeUpTime);
    nextPeriod = prevWakeUpTime;

    while (true)
    {
        const bool status = task();

        if( useRunMore)
        {
            runMoreActive = status;
        }

        if( stopRunning)
        {
            break;
        }

        if (!dontCalcLatency)
        {
            updateExecutionTimeData(prevWakeUpTime);
        }

        sleep();
    }
}

void CyclicTask::initThreadAttributes()
{
    if (pthread_attr_init(&thread_attr) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to init pthread attributes");
    }

    if (pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN + defaultStackSize) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set pthread stack size");
    }

    size_t stackSet = 0;
    if (pthread_attr_getstacksize(&thread_attr, &stackSet) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to get pthread stack size");
    }

    if (stackSet != defaultStackSize + PTHREAD_STACK_MIN)
    {
        throw std::runtime_error("CyclicTask: StackSet != stackSize");
    }

    if (rtEnabled)
    {
        if (pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO) != 0)
        {
            throw std::runtime_error("CyclicTask: Failed to set pthread scheduling policy");
        }

        if (pthread_attr_setschedparam(&thread_attr, &realTimePriority) != 0)
        {
            throw std::runtime_error("CyclicTask: Failed to set pthread scheduling parameters");
        }

        if (pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED) != 0)
        {
            throw std::runtime_error("CyclicTask: Failed to set pthread inherit sched");
        }
    }
}

CyclicTask::CyclicTask(const std::string& taskName_, bool rtEnabled_, const timespec& period_,
                       std::function<bool()> task_, std::chrono::nanoseconds histogramSize, bool dontCalcLatency_,
                       bool aperiodicSleep_, bool useRunMore_)
    : taskName(taskName_), period(period_), rtEnabled(rtEnabled_), task(std::move(task_)),
      histogram(std::chrono::duration_cast<std::chrono::microseconds>(histogramSize).count(), 0),
      dontCalcLatency(dontCalcLatency_), aperiodicSleep(aperiodicSleep_), useRunMore(useRunMore_), cpuMonitor(0, taskName_)
{
    const auto threadName = "RtTest_" + taskName_ + "Worker";

    initThreadAttributes();

    initCondVar();

    if (pthread_create(&thread, &thread_attr, worker, this) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to create Thread");
    }

    if (pthread_setname_np(thread, threadName.c_str()) != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set thread name");
    }
}

CyclicTask::~CyclicTask()
{
//    stopRunning = true;

//    if (int ret = pthread_join(thread, nullptr); ret != 0)
//    {
//        fmt::println("join pthread failed: {} with {}", taskName, ret);
//    }
}
void CyclicTask::updateResourceData()
{
    getrusage(RUSAGE_THREAD, &resourceData);
}
void CyclicTask::initCondVar()
{
    errno = pthread_condattr_init (&cond_attr);
    if (errno) {
        throw std::runtime_error ("pthread_condattr_init");
    }

    errno = pthread_condattr_setclock (&cond_attr, CLOCK_MONOTONIC);
    if (errno) {
        throw std::runtime_error ("pthread_condattr_setclock");
    }

    errno = pthread_cond_init (&cond, &cond_attr);
    if (errno) {
        throw std::runtime_error ("pthread_cond_init");
    }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        throw std::runtime_error("\n mutex init has failed\n");
    }
}
void CyclicTask::updateCpuData()
{
    cpuMonitor.update();
}
