#include "ArgParser.h"
#include "ContinuousBuffer.h"
#include "Cpu.h"
#include "CyclicTask.h"
#include "DataMover.h"
#include "H2f.h"
#include "HwDecoder.h"
#include "RxDriver.h"
#include "RxPipeline.h"
#include "SingleWorker.h"
#include "SwDecoder.h"
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

    const std::string kGenName = "GeneratorThread";
    CpuMonitor kGenCpuMonitor(get_task_id_from_comm(kGenName), kGenName);

    ContinuousBuffer rxBuffer(RX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    ContinuousBuffer txBuffer(TX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    H2f::init();

    DataMover::init(rxBuffer.getPhysicalAddress(), rxBuffer.getSize(), txBuffer.getPhysicalAddress(),
                    txBuffer.getSize(), args.bitrate);

    auto rxRingBuffer = RingBuffer<RingBufferType::RX>(rxBuffer);
    const bool readUntilEmpty = (PACKET_H_DEFAULT_SIZE < 4008U && !args.aperiodicSleep);
    RxDriver addaRx(rxRingBuffer, false, 200, readUntilEmpty);
    HwDecoder decoder{};
    SwDecoder decimationChain{2};
    KernelPatternVerifier patternVerifier;
//    patternVerifier.start();
    RxPipeline rxWorker(addaRx, decimationChain, decoder, patternVerifier);

    auto txRingBuffer = RingBuffer<RingBufferType::TX>(txBuffer);
    TxDriver addaTx(txRingBuffer);
    HwEncoder encoder{4};
    SwEncoder interpolationChain{2};
    KernelPatternGenerator patternGenerator{true};
    TxPipeline txWorker(addaTx, encoder, interpolationChain, patternGenerator);

    SingleWorker singleWorker(txWorker, rxWorker);
    CyclicTask singleTask("ST", args.realtime, args.workerPeriod,
                          std::bind_front(&SingleWorker::update, std::ref(singleWorker)), args.histogramSize,
                          args.dontCalcLatency, args.aperiodicSleep, args.useRunMore);

    if (!args.silent)
    {
        Ui::init();
    }

    if (args.realtime)
    {
        /* Lock memory */
        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            throw std::runtime_error("Main: Failed to lock memory");
        }
    }

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 35U;
        if (!args.silent)
        {
            printRunTime(startTime);
            cpuPrintLoad();
            singleTask.printLatencyData();
            singleTask.printExecutionTimeData();
            singleTask.printResourceData();
            singleTask.printCpuData();
            kGenCpuMonitor.printStats();
            fmt::println("");

            fmt::println("RX Chain - {:14}", patternVerifier.getFormattedBitrate());
            addaRx.printStats();
            decimationChain.printStats();
            decoder.printStats();
            patternVerifier.printStats();
            fmt::println("");

            rxWorker.printQueues();
            fmt::println("\n");

            fmt::println("TX Chain - {:14}", patternGenerator.getFormattedBitrate());
            patternGenerator.printStats();
            encoder.printStats();
            addaTx.printStats();

            fmt::println("");
            txWorker.printQueues();
        }

        auto command = Ui::getCommand();

        static bool first = true;
        if (first)
        {
            if( args.enableRxOnStartup)
            {
                command = Ui::Command::RxEnable;
            }
            first = false;
        }

        switch (command)
        {
        case Ui::Command::Transmit:
            txWorker.clearTransmission();
            static const auto transmitSize = 10_MiB;
            txWorker.send(transmitSize);
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

        singleTask.updateCpuData();
        kGenCpuMonitor.update();
        patternGenerator.updateBitrate();
        patternVerifier.updateBitrate();

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
    if( !args.silent)
    {
        Ui::kill();
    }
    DataMover::rxDisable();
    singleTask.kill();

    rxBuffer.cleanup();
    txBuffer.cleanup();

    singleTask.printFinalStats();
    singleTask.printFinalCpuData();
    kGenCpuMonitor.printFinalStats();

    fmt::println("RX Chain - {:14}", patternVerifier.getFormattedAverageBitrate());
    addaRx.printFinalStats();
    decimationChain.printFinalStats();
    decoder.printFinalStats();
    patternVerifier.printFinalStats();
    rxWorker.printFinalQueues();

    fmt::println("TX Chain - {:14}", patternGenerator.getFormattedAverageBitrate());
    patternGenerator.printFinalStats();
    encoder.printFinalStats();
    interpolationChain.printFinalStats();
    addaTx.printFinalStats();
    txWorker.printFinalQueues();

    fmt::println("Storing Histogram Data...");
    singleTask.saveHistogramData(args.histogramFile);
}
