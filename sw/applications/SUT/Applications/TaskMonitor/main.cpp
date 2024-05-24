#include "Cpu.h"
#include "Utils.h"
#include "fmt/core.h"
#include <thread>
#include "ArgParser.h"
#include <stdexcept>

int main(int argc, char* argv[])
{
    fmt::println("##########################################");
    fmt::println("                  Setup                   ");
    fmt::println("##########################################");
    static const auto inputArgs = std::vector<std::string_view>(argv + 1, argv + argc);
    auto args = ArgParser::parse(inputArgs);

    std::string taskcomm = "GeneratorThread";

    CpuMonitor kGenCpuMonitor(get_task_id_from_comm(taskcomm), taskcomm);

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 6;
        if (!args.silent)
        {
            printRunTime(startTime);
            cpuPrintLoad();
            kGenCpuMonitor.update();
            kGenCpuMonitor.printStats();
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
    kGenCpuMonitor.printFinalStats();
}
