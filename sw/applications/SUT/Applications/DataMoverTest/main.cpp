
#include "ArgParser.h"
#include "ContinuousBuffer.h"
#include "Cpu.h"
#include "CyclicTask.h"
#include "DataMover.h"
#include "H2f.h"
#include "RingBuffer.h"
#include "RxPipeline.h"
#include "TxPipeline.h"
#include "Ui.h"
#include "Utils.h"
#include "fmt/core.h"
#include <chrono>
#include <stdexcept>
#include <thread>

constexpr size_t RX_BUFFER_SIZE = 128_kiB;
constexpr size_t TX_BUFFER_SIZE = 128_kiB;

int main(int argc, char* argv[]) 
{
    fmt::println("##########################################");
    fmt::println("                  Setup                   ");
    fmt::println("##########################################");
    static const auto inputArgs = std::vector<std::string_view>(argv + 1, argv + argc);
    auto args = ArgParser::parse(inputArgs);
    ArgParser::print(args);

    ContinuousBuffer rxBuffer(RX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    ContinuousBuffer txBuffer(TX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    H2f::init();

    DataMover::init(rxBuffer.getPhysicalAddress(), rxBuffer.getSize(), txBuffer.getPhysicalAddress(),
                    txBuffer.getSize(), args.bitrate);

    auto rxRingBuffer = RingBuffer<RingBufferType::RX>(rxBuffer);
    RxDriver rxWorker(rxRingBuffer, nullptr, <#initializer #>);
    CyclicTask rxTask("RX", args.realtime, args.workerPeriod, std::bind_front(&RxDriver::update, std::ref(rxWorker)),
                      args.histogramSize);

    auto txRingBuffer = RingBuffer<RingBufferType::TX>(txBuffer);
    TxDriver txWorker(txRingBuffer );
    CyclicTask txTask("TX", args.realtime, args.workerPeriod, std::bind_front(&TxDriver::update, std::ref(txWorker)),
                      args.histogramSize);

    Ui::init();

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");
    const auto startTime = std::chrono::steady_clock::now();

    bool cont = true;

    while (true)
    {
        static constexpr auto numberOfLinesPrinted = 9U;
        printRunTime(startTime);
        cpuPrintLoad();
        rxWorker.printStats();
        rxTask.printLatencyData();
        rxTask.printExecutionTimeData();
        fmt::println("");

        txWorker.printStats();
        txTask.printLatencyData();
        txTask.printExecutionTimeData();

        // printf( "Tx Read errors:        %3" PRIu32 "\n", DataMover::getTxReadErrors() );
        // printf( "Tx Last data received: 0x%016" PRIx64 "\n", DataMover::getTxLastRead() );
        // printf( "Tx read pointer:       0x%08" PRIx32 "\n", DataMover::getTxReadPointer() );
        // printf( "Tx reads performed:    %3" PRIu64 "\n", DataMover::txReadsPerformed() );
        // printf( "Rx Enable:             0x%08" PRIx32 "\n", DataMover::rxIsEnabled() );
        // printf( "State:                 %3" PRIu32 "\n", DataMover::getState() );
        // printf( "SoftResetCounter:      %3" PRIu32 "\n", DataMover::readRegister(
        // DataMover::Register::SoftResetCounter ) ); printf( "Total Bytes Transmit.: %8s\n", convertBytesToString(
        // txWorker->getTotalBytesTransmitted() ).c_str() );

        auto command = Ui::getCommand();

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

        if (!cont || runTimeExpired(startTime, args.runtime))
        {
            break;
        }

        else
        {
            static constexpr auto sleepTime = std::chrono::milliseconds(500);
            std::this_thread::sleep_for(sleepTime);
            fmt::print("\033[{}A\r", numberOfLinesPrinted);
        }
    }

    fmt::println("\n##########################################");
    fmt::println("                  Done                    ");
    fmt::println("##########################################");
    printRunTime(startTime);
    fmt::print("Stopping all workers...\n\n");
    Ui::kill();
    rxWorker.stop();
    rxTask.kill();
    txWorker.stop();
    txTask.kill();

    rxWorker.printFinalStats();
    rxTask.printFinalStats();
    txWorker.printFinalStats();
    txTask.printFinalStats();

    fmt::println("Storing Histogram Data...");
    rxTask.saveHistogramData(args.histogramFile);
    txTask.saveHistogramData(args.histogramFile);
}
