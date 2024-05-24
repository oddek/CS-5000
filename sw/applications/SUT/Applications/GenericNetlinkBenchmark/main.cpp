#include "ArgParser.h"
#include "Cpu.h"
#include "CyclicTask.h"
#include "Utils.h"
#include "fmt/core.h"
#include <stdexcept>
#include <thread>

#include "KernelPatternVerifier.h"
#include "KernelPatternGenerator.h"
#include "UserspacePatternVerifier.h"
#include "UserspacePatternGenerator.h"
#include "GenericNetlinkTester.h"

int main(int argc, char* argv[]) 
{
    fmt::println("##########################################");
    fmt::println("                  Setup                   ");
    fmt::println("##########################################");
    static const auto inputArgs = std::vector<std::string_view>(argv + 1, argv + argc);
    auto args = ArgParser::parse(inputArgs);
    ArgParser::print(args);

    if (args.realtime)
    {
        doMemoryLock();
        prefaultHeap(100_MiB);
    }

    fmt::println("Pid of Generator: {}", get_task_id_from_comm("GeneratorThread"));
    std::string kGenName = "GeneratorThread";
    CpuMonitor kGenCpuMonitor(get_task_id_from_comm(kGenName), kGenName);

    KernelPatternGenerator kernelPatternGenerator{false};
    KernelPatternVerifier kernelPatternVerifier;
    GenericNetlinkTester netlink(kernelPatternGenerator, kernelPatternVerifier);

    CyclicTask task("NL", args.realtime, args.workerPeriod, std::bind_front(&GenericNetlinkTester::update, std::ref(netlink)), args.histogramSize, args.dontCalcLatency, args.aperiodicSleep, args.useRunMore);

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 26;
        if (!args.silent)
        {
            printRunTime(startTime);
            cpuPrintLoad();
            fmt::println("");
            task.printLatencyData();
            task.printExecutionTimeData();
            task.printResourceData();
            fmt::println("");
            kernelPatternGenerator.printStats();
            kernelPatternVerifier.printStats();

            task.updateCpuData();
            task.printCpuData();
            kGenCpuMonitor.update();
            kGenCpuMonitor.printStats();

            netlink.printStats();
            fmt::println("\n");
        }

        if (!cont || runTimeExpired(startTime, args.runtime))
        {
            break;
        }

        else
        {
            static constexpr auto sleepTime = std::chrono::milliseconds(500);
            std::this_thread::sleep_for(sleepTime);
            if (!args.silent)
            {
                fmt::print("\033[{}A\r", numberOfLinesPrinted - 1);
            }
        }
    }

    fmt::println("\n##########################################");
    fmt::println("                  Done                    ");
    fmt::println("##########################################");
    kGenCpuMonitor.update();
    printRunTime(startTime);
    cpuPrintLoad();
    fmt::print("Stopping all workers...\n\n");
    task.kill();

    task.printFinalStats();
    netlink.printFinalStats();
    kernelPatternGenerator.printStats();
    kernelPatternVerifier.printStats();
    task.printFinalCpuData();
    kGenCpuMonitor.printFinalStats();

    fmt::println("Storing Histogram Data...");
    task.saveHistogramData(args.histogramFile);
}
