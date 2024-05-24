
#include "ArgParser.h"
#include "fmt/core.h"

namespace ArgParser
{
void print(const Args& args)
{
    fmt::println("{:16}{}", "HistogramFile:", args.histogramFile);
    fmt::println("{:16}{}", "Hist Lat Cap:", convertChronoToMinimalString(args.histogramSize));
    fmt::println("{:16}{}", "Runtime:", convertChronoToMinimalString(args.runtime));
    fmt::println("{:16}{}", "RealTimeTasks:", args.realtime);
    fmt::println("{:16}{}", "Worker Period:", convertTimespecToString(args.workerPeriod));
    fmt::println("{:16}{}", "Bitrate:", convertBitrateToString(args.bitrate));
    fmt::println("{:16}{}", "Dont Calc Latency: ", args.dontCalcLatency);
    fmt::println("{:16}{}", "Aperiodic Sleep: ", args.aperiodicSleep);
    fmt::println("{:16}{}", "Use Run More: ", args.useRunMore);
    fmt::println("{:16}{}", "Enable RX on startup: ", args.enableRxOnStartup);
    fmt::println("{:16}{}", "Dont Memlock On RT: ", args.dontMemlockOnRt);
    fmt::println("");
}

const Args& parse(const std::vector<std::string_view>& inputArgs)
{
    static Args args;

    for (auto it = inputArgs.begin(); it != inputArgs.end(); it++)
    {
        if ((*it == "--histfile") || (*it == "-f"))
        {
            it++;
            args.histogramFile = *it;
        }
        else if ((*it == "--histsize") || (*it == "-s"))
        {
            it++;
            args.histogramSize = timeStringToChrono(*it);
        }
        else if ((*it == "--runtime") || (*it == "-t"))
        {
            it++;
            args.runtime = timeStringToChrono(*it);
        }
        else if ((*it == "--realtime") || (*it == "-r"))
        {
            args.realtime = true;
        }
        else if ((*it == "--bitrate") || (*it == "-b"))
        {
            it++;
            args.bitrate = bitrateStringToBitrate(*it);
        }
        else if ((*it == "--period") || (*it == "-p"))
        {
            it++;
            args.workerPeriod = timeStringToTimespec(*it);
        }
        else if ((*it == "--silent") || (*it == "-S"))
        {
            args.silent = true;
        }
        else if ((*it == "--dont-calc-latency") || (*it == "-d"))
        {
            args.dontCalcLatency = true;
        }
        else if ((*it == "--aperiodic-sleep") || (*it == "-a"))
        {
            args.aperiodicSleep = true;
        }
        else if ((*it == "--use-run-more") || (*it == "-u"))
        {
            args.useRunMore = true;
        }
        else if ((*it == "--rx-start") || (*it == "-R"))
        {
            args.enableRxOnStartup = true;
        }
        else if ((*it == "--dont-memlock-on-rt"))
        {
            args.dontMemlockOnRt = true;
        }
        else
        {
            throw std::runtime_error(" Invalid arg detected: " + std::string(*it));
        }
    }

    return args;
}
} // namespace ArgParser
