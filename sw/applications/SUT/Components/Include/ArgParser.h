
#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "Utils.h"
#include <string>

namespace ArgParser
{
static constexpr size_t DEFAULT_BITRATE = 10_Mibps;
static constexpr auto DEFAULT_HISTOGRAM_SIZE = std::chrono::microseconds(1000U);
static constexpr auto DEFAULT_RUNTIME = std::chrono::minutes(30);
static constexpr auto DEFAULT_WORKER_PERIOD = durationToTimespec(std::chrono::milliseconds(1));

struct Args
{
    std::string_view histogramFile{"hist.txt"};
    std::chrono::nanoseconds histogramSize{DEFAULT_HISTOGRAM_SIZE};
    bool realtime{false};
    size_t bitrate{DEFAULT_BITRATE};
    std::chrono::nanoseconds runtime = {DEFAULT_RUNTIME};
    struct timespec workerPeriod = {DEFAULT_WORKER_PERIOD};
    bool silent{false};
    bool dontCalcLatency{false};
    bool aperiodicSleep{false};
    bool useRunMore{false};
    bool enableRxOnStartup{false};
    bool dontMemlockOnRt{false};
};

void print(const Args& args);
const Args& parse(const std::vector<std::string_view>& inputArgs);

} // namespace ArgParser

#endif
