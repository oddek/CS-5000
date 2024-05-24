

/******************************************************************************
 * Get CPU usage for process partially based on:
 * https://stackoverflow.com/questions/50900905/get-cpu-usage-of-current-process
******************************************************************************/


#include "Cpu.h"
#include <fmt/core.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>




void cpuPrintLoad()
{
    std::string loadAverage;
    std::ifstream file("/proc/loadavg");
    std::getline(file, loadAverage);
    fmt::println("Load Avg: {}", loadAverage);
}

CpuMonitor::CpuMonitor(pid_t pid_, std::string name_) : pid(pid_), name(name_)
{
    if (pid_)
    {
        initialize();
    }
}

void CpuMonitor::setPid(pid_t pid_)
{
    pid = pid_;

    if (pid)
    {
        initialize();
    }
}

void CpuMonitor::initialize()
{
    if (!initialized)
    {
        firstUsage = get_usage();
        prevUsage = firstUsage;
        prevTime = getTimespecNow();
        initialized = true;
    }
    else
    {
        std::string error = fmt::format("CpuMonitor for {} already initialized", name);
        throw std::runtime_error(error);
    }
}

void CpuMonitor::update()
{
    if (!pid)
    {
        return;
//        std::string error = fmt::format("CpuMonitor for {}: update called on pid 0", name);
//        throw std::runtime_error(error);
    }

    const auto now = getTimespecNow();
    const auto diff = timeSpecDiffUs(now, prevTime);

    if (diff > updateInterval)
    {
        const auto currUsage = get_usage();

        auto [usrCpu, sysCpu] = calc_cpu_usage_pct(&currUsage, &prevUsage);

        if (std::isnan(usrCpu) || std::isnan(sysCpu) || std::isinf(usrCpu) || std::isinf(sysCpu) || usrCpu > 100.0 ||
            sysCpu > 100.0)
        {
            return;
        }

        usrCpuLoad = usrCpu;
        sysCpuLoad = sysCpu;
        totalCpuLoad = usrCpuLoad + sysCpuLoad;

        auto [avrUsrCpu, avrSysCpu] = calc_cpu_usage_pct(&currUsage, &firstUsage);

        averageUsrCpuLoad = avrUsrCpu;
        averageSysCpuLoad = avrSysCpu;
        averageTotalCpuLoad = averageUsrCpuLoad + averageSysCpuLoad;

        prevUsage = currUsage;
        prevTime = now;

        minorFaults = currUsage.minflt - firstUsage.minflt;
        majorFaults = currUsage.majflt - firstUsage.majflt;

        involContextSwitches = currUsage.invol_context_switches;
        volContextSwitches = currUsage.vol_context_switches;
    }
}
void CpuMonitor::printStats()
{
    fmt::println("CPU - {} - {}:", pid, name);
    fmt::println("\tCurr - Usr {: 3.2f}, Sys {: 3.2f}, Tot {: 3.2f}", usrCpuLoad, sysCpuLoad, totalCpuLoad);
    fmt::println("\tAvg. - Usr {: 3.2f}, Sys {: 3.2f}, Tot {: 3.2f}", averageUsrCpuLoad, averageSysCpuLoad,
                 averageTotalCpuLoad);
    fmt::println("\tFlts - Maj {:8}, Min {:8}", majorFaults, minorFaults);
    fmt::println("\tCtxS - Vol {:8}, Non {:8}", volContextSwitches, involContextSwitches);
}
void CpuMonitor::printFinalStats()
{
    fmt::println("CPU - {} - {}:", pid, name);
    fmt::println("\tAvg. Load - Usr {:.2f}, Sys {:.2f}, Tot {:.2f}", averageUsrCpuLoad, averageSysCpuLoad,
                 averageTotalCpuLoad);
    fmt::println("\tFlts - Maj {:8}, Min {:8}", majorFaults, minorFaults);
    fmt::println("\tCtxS - Vol {:8}, Non {:8}", volContextSwitches, involContextSwitches);
    fmt::println("");
}

std::tuple<uint32_t, uint32_t> CpuMonitor::getContextSwitches()
{
    uint32_t invol{UINT32_MAX};
    uint32_t vol{UINT32_MAX};
    const std::string filename = "/proc/" + std::to_string(pid) + "/task/" + std::to_string(pid) + "/status";

    std::ifstream input(filename);
    for (std::string eachLine; getline(input, eachLine);)
    {
        std::istringstream strm(eachLine);
        std::string key;
        uint32_t value = UINT32_MAX;
        strm >> key >> value;

//        fmt::println("Key: {}, Val: {}", key, value);
        if (key == "voluntary_ctxt_switches:")
        {
            vol = value;
        }
        else if (key == "nonvoluntary_ctxt_switches:")
        {
            invol = value;
        }
    }

    if (invol == UINT32_MAX || vol == UINT32_MAX)
    {
        throw std::runtime_error("Did not find context switches");
    }
    return {vol, invol};
}

pstat CpuMonitor::get_usage()
{
    pstat result{0};
    const auto [vol, invol] = getContextSwitches();
    result.vol_context_switches = vol;
    result.invol_context_switches = invol;
    // convert  pid to string
    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);
    char stat_filepath[30] = "/proc/";
    strncat(stat_filepath, pid_s, sizeof(stat_filepath) - strlen(stat_filepath) - 1);
    strncat(stat_filepath, "/task/", sizeof(stat_filepath) - strlen(stat_filepath) - 1);
    strncat(stat_filepath, pid_s, sizeof(stat_filepath) - strlen(stat_filepath) - 1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) - strlen(stat_filepath) - 1);

    FILE* fpstat = fopen(stat_filepath, "r");
    if (fpstat == NULL)
    {
        throw std::runtime_error("FOPEN ERROR ");
    }

    FILE* fstat = fopen("/proc/stat", "r");
    if (fstat == NULL)
    {
        throw std::runtime_error("FOPEN ERROR ");
    }

    // read values from /proc/pid/stat
    uint64_t rss{0};

    //               " %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u"
    //               " %*u %*u %*u %*u %*llu %*llu %*lld\n",
    //               "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
    //               "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",


//    3858 (RtTest_RXWorker) S 3855 3855 129 1088 3855 4194624 33
//    0 0 0 269 0 0 0 -81 0 3 0 427210 1194844160 29024 4294967295 65536 249416 3201969648 0 0 0 0 0 2 1 0 0 -1 1 80 1 0 0 0 253052 254528 258048 3201969924 3201970007 3201970007 3201970148 0

    if (fscanf(fpstat,
               "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u"
               " %*u %*u %llu %llu %lld %lld %*d %*d %*u %*u %*d %llu %llu",
               &result.utime_ticks, &result.stime_ticks, &result.cutime_ticks, &result.cstime_ticks, &result.vsize,
               &rss) == EOF)
    {
        fclose(fpstat);
        throw std::runtime_error("Could not read proc pid stat");
    }

//    if (fscanf(fpstat,
//               "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
//               "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
//               &result.utime_ticks, &result.stime_ticks, &result.cutime_ticks, &result.cstime_ticks, &result.vsize,
//               &rss) == EOF)
//    {
//        fclose(fpstat);
//        throw std::runtime_error("Could not read proc pid stat");
//    }
    fclose(fpstat);
    result.rss = rss * getpagesize();

    // read+calc cpu total time from /proc/stat
    long unsigned int cpu_time[10];
    bzero(cpu_time, sizeof(cpu_time));
    if (fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &cpu_time[0], &cpu_time[1], &cpu_time[2],
               &cpu_time[3], &cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7], &cpu_time[8], &cpu_time[9]) == EOF)
    {
        fclose(fstat);
        throw std::runtime_error("Could not read proc stat");
    }

    fclose(fstat);

    for (int i = 0; i < 10; i++)
        result.cpu_total_time += cpu_time[i];

    return result;
}
std::tuple<double, double> CpuMonitor::calc_cpu_usage_pct(const struct pstat* cur_usage, const struct pstat* last_usage)
{
    const long unsigned int total_time_diff = cur_usage->cpu_total_time - last_usage->cpu_total_time;

    if (total_time_diff == 0)
    {
        return {0,0};
        throw std::runtime_error("Calc cpu usage divide by zero");
    }

    double ucpu_usage =
        100 *
        (((cur_usage->utime_ticks + cur_usage->cutime_ticks) - (last_usage->utime_ticks + last_usage->cutime_ticks)) /
         (double)total_time_diff);

//    fmt::println("ucpu: {}, {} {} {} {} {}", ucpu_usage, cur_usage->utime_ticks, cur_usage->cutime_ticks, last_usage->utime_ticks, last_usage->cutime_ticks,
//                 (double)total_time_diff);

    double scpu_usage =
        100 *
        ((((cur_usage->stime_ticks + cur_usage->cstime_ticks) - (last_usage->stime_ticks + last_usage->cstime_ticks))) /
         (double)total_time_diff);

//    fmt::println("scpu: {}, {} {} {} {} {}", scpu_usage, cur_usage->stime_ticks, cur_usage->cstime_ticks, last_usage->stime_ticks, last_usage->cstime_ticks,
//         (double)total_time_diff);

    return {ucpu_usage, scpu_usage};
}
