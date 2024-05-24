

#ifndef CPU_H
#define CPU_H

/******************************************************************************
 * Get CPU usage for process partially based on:
 * https://stackoverflow.com/questions/50900905/get-cpu-usage-of-current-process
******************************************************************************/

#include <sys/types.h>
#include <tuple>
#include <math.h>
#include "Utils.h"

void cpuPrintLoad();

struct pstat
{
    uint64_t utime_ticks;
    int64_t cutime_ticks;
    uint64_t stime_ticks;
    int64_t cstime_ticks;
    uint64_t vsize; // virtual memory size in bytes
    uint64_t rss;   // Resident  Set  Size in bytes

    long unsigned int cpu_total_time;

    long unsigned int minflt;
    long unsigned int majflt;

    uint32_t invol_context_switches;
    uint32_t vol_context_switches;
};

class CpuMonitor
{
  public:
    CpuMonitor() = delete;
    CpuMonitor(pid_t pid_, std::string name_);
    void setPid(pid_t pid_);
    void update();
    void printStats();
    void printFinalStats();


  private:
    void initialize();
    pstat get_usage();
    std::tuple<double, double> calc_cpu_usage_pct(const struct pstat* cur_usage, const struct pstat* last_usage);


    volatile pid_t pid{0};
    const std::string name{};

    bool initialized{false};

    pstat prevUsage{0};
    pstat firstUsage{0};

    timespec prevTime{0};
    double_t usrCpuLoad{0.0};
    double_t sysCpuLoad{0.0};
    double_t totalCpuLoad{0.0};
    double_t averageUsrCpuLoad{0.0};
    double_t averageSysCpuLoad{0.0};
    double_t averageTotalCpuLoad{0.0};

    uint32_t minorFaults{0};
    uint32_t majorFaults{0};

    uint32_t involContextSwitches{0};
    uint32_t volContextSwitches{0};

    static constexpr int64_t updateInterval{1000000};
    std::tuple<uint32_t, uint32_t> getContextSwitches();
};

#endif