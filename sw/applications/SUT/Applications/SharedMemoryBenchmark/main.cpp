#include "ArgParser.h"
#include "ContinuousBuffer.h"
#include "Cpu.h"
#include "CyclicTask.h"
#include "DataMover.h"
#include "H2f.h"
#include "HwDecoder.h"
#include "RxDriver.h"
#include "RxPipeline.h"
#include "TxDriver.h"
#include "TxPipeline.h"
#include "Ui.h"
#include "Utils.h"
#include "fmt/core.h"
#include <stdexcept>
#include <sys/mman.h>
#include <thread>

constexpr size_t RX_BUFFER_SIZE = 128_kiB;
constexpr size_t TX_BUFFER_SIZE = 128_kiB;

int main(int argc, char* argv[]) // NOLINT(*-exception-escape)
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

    ContinuousBuffer rxBuffer(RX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    ContinuousBuffer txBuffer(TX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    H2f::init();

    DataMover::init(rxBuffer.getPhysicalAddress(), rxBuffer.getSize(), txBuffer.getPhysicalAddress(),
                    txBuffer.getSize(), args.bitrate);

    auto rxRingBuffer = RingBuffer<RingBufferType::RX>(rxBuffer);
    const bool readUntilEmpty = (PACKET_H_DEFAULT_SIZE == 512U && !args.aperiodicSleep);
    RxDriver addaRx(rxRingBuffer, true, 50, readUntilEmpty);
    CyclicTask rxTask("RX", args.realtime, args.workerPeriod, std::bind_front(&RxDriver::update, std::ref(addaRx)),
                      args.histogramSize, args.dontCalcLatency, args.aperiodicSleep, args.useRunMore);

    if (!args.silent)
    {
        Ui::init();
    }

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 16;
        if (!args.silent)
        {
            printRunTime(startTime);
            cpuPrintLoad();
            fmt::println("RX Chain");
            rxTask.printLatencyData();
            rxTask.printExecutionTimeData();
            rxTask.printResourceData();
            rxTask.printCpuData();
            fmt::println("");

            addaRx.printStats();
            fmt::println("\n");

            auto command = Ui::getCommand();

            static bool first = true;
            if (first)
            {
                command = Ui::Command::RxEnable;
                first = false;
            }

            switch (command)
            {
            case Ui::Command::Transmit:
                break;

            case Ui::Command::RxDisable:
                DataMover::rxDisable();
                break;

            case Ui::Command::RxEnable:
                DataMover::rxEnable();
                break;

            case Ui::Command::DataMoverSoftReset:
                DataMover::softReset();
                DataMover::init();
                break;

            case Ui::Command::Quit:
                cont = false;
                break;

            case Ui::Command::None:
                break;

            default:
                throw std::runtime_error("Unknown Command");
            }
        }
        else
        {
            static bool first = true;
            if (first)
            {
                DataMover::rxEnable();
                first = false;
            }
        }

        rxTask.updateCpuData();

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
    if (!args.silent)
    {
        Ui::kill();
    }
    DataMover::rxDisable();
    rxTask.kill();

    rxBuffer.cleanup();
    txBuffer.cleanup();

    addaRx.printFinalStats();
    rxTask.printFinalStats();
    rxTask.printFinalCpuData();

    fmt::println("Storing Histogram Data...");
    rxTask.saveHistogramData(args.histogramFile);
}
