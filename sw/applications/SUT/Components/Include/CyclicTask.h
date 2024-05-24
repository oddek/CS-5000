
#ifndef CYCLIC_TASK_H
#define CYCLIC_TASK_H

#include "Utils.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <pthread.h>
#include <vector>
#include "Cpu.h"

struct TimeData
{
    std::atomic<uint32_t> current{0U};
    std::atomic<uint32_t> min{UINT32_MAX};
    std::atomic<uint32_t> max{0U};
    std::atomic<double> average{0.0};
    std::atomic<uint64_t> samples{1U};
};


class CyclicTask
{
  public:
    CyclicTask(const std::string& taskName_, bool rtEnabled_, const timespec& period_, std::function<bool()> task_,
               std::chrono::nanoseconds histogramSize, bool dontCalcLatency_, bool aperiodicSleep_, bool useRunMore_);
    CyclicTask(const CyclicTask&) = delete;
    CyclicTask(CyclicTask&&) = delete;
    CyclicTask& operator=(const CyclicTask&) = delete;
    CyclicTask& operator=(CyclicTask&&) = delete;
    ~CyclicTask();
    void kill();
    void run();
    void printCpuData();
    void printFinalCpuData();
    void printLatencyData();
    void printResourceData();
    void printExecutionTimeData();
    void printFinalStats();
    void saveHistogramData(const std::string_view& filename);

    void updateCpuData();

  private:
    const std::string taskName{};
    const struct timespec period
    {
    };
    struct timespec nextPeriod{};
    struct timespec prevWakeUpTime{};
    struct timespec currWakeUpTime{};
    std::atomic<bool> stopRunning{false};
    const bool rtEnabled{};
    std::function<bool(void)> task;
    pthread_t thread{};
    pthread_attr_t thread_attr{};
    pthread_cond_t cond;
    pthread_condattr_t cond_attr;
    pthread_mutex_t mutex;

    static constexpr sched_param realTimePriority{80};

    void initThreadAttributes();
    void initCondVar();
    void sleep();
    void printTimeData(TimeData& timeData, const char* prefix);
    void updateLatencyData();
    void updateResourceData();
    void updateExecutionTimeData(const struct timespec& taskStartTime);
    static void updateTimeData(TimeData& timeData, uint32_t newValue);

    void addToHistogram(uint32_t newValue);

    rusage resourceData;
    TimeData latencyData;
    TimeData executionTimeData;

    std::vector<size_t> histogram;
    size_t histogramOverflow{0};

    const bool dontCalcLatency{};
    const bool aperiodicSleep{};
    const bool useRunMore{};
    bool runMoreActive{false};
    size_t missedPeriods{0U};
    size_t maxSingleMissedPeriods{0U};

    CpuMonitor cpuMonitor;
};

#endif
