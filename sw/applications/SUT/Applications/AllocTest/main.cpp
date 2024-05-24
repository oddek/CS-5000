#include "ArgParser.h"
#include "Cpu.h"
#include "CyclicTask.h"
#include "Utils.h"
#include "fmt/core.h"
#include <stdexcept>
#include <thread>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <PatternGenerator/Netlink.h>
#include <DurationTimer.h>

struct nl_sock* nl_socket;
int genl_family;
static constexpr size_t socketBufferSize = 256U * 8192U;
DurationTimer<Resolution::us> allocTimer{"alloc"};
DurationTimer<Resolution::us> putTimer{"put"};
DurationTimer<Resolution::us> freeTimer{"free"};

bool alloc_task(void* arg)
{
    allocTimer.start();
    auto* msg = nlmsg_alloc();
    allocTimer.stop();

    putTimer.start();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, genl_family, 0, 0, GENERATE, NLM_F_REQUEST);
    nla_put_u32(msg, SAI_LENGTH, 150);
    putTimer.stop();


    freeTimer.start();
    nlmsg_free(msg);
    freeTimer.stop();

    return false;
}

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
        if(!args.dontMemlockOnRt)
        {
            doMemoryLock();
            prefaultHeap(100_MiB);
        }
    }

    nl_socket = nl_socket_alloc();
    genl_connect(nl_socket);
    genl_family = genl_ctrl_resolve(nl_socket, GENERATOR_NETLINK_FAMILY);

    if (genl_family < 0)
    {
        throw std::runtime_error(fmt::format("Kernel Generator: Genl resolve failed: {}", genl_family));
    }

    nl_socket_disable_seq_check(nl_socket);
    nl_socket_set_buffer_size(nl_socket, socketBufferSize, socketBufferSize);
    nl_socket_disable_auto_ack(nl_socket);

    CyclicTask task("NL", args.realtime, args.workerPeriod, std::bind_front(&alloc_task, nullptr), args.histogramSize, args.dontCalcLatency, args.aperiodicSleep, args.useRunMore);

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 11;
        if (!args.silent)
        {
            printRunTime(startTime);
            cpuPrintLoad();
            fmt::println("");
            task.printLatencyData();
            task.printExecutionTimeData();
            task.printResourceData();
            allocTimer.printStats();
            putTimer.printStats();
            freeTimer.printStats();
            fmt::println("");
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
    printRunTime(startTime);
    cpuPrintLoad();
    fmt::print("Stopping all workers...\n\n");
    task.kill();

    task.printFinalStats();
    task.printFinalCpuData();
    fmt::println("");
    allocTimer.printFinalStats();
    putTimer.printFinalStats();
    freeTimer.printFinalStats();

    fmt::println("Storing Histogram Data...");
    task.saveHistogramData(args.histogramFile);
}
