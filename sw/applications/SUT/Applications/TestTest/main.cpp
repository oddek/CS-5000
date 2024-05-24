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

constexpr size_t TX_BUFFER_SIZE = 128_kiB;

int main(int argc, char* argv[])
{
    fmt::println("##########################################");
    fmt::println("                  Setup                   ");
    fmt::println("##########################################");
    static const auto inputArgs = std::vector<std::string_view>(argv + 1, argv + argc);
    auto args = ArgParser::parse(inputArgs);
    ArgParser::print(args);

    ContinuousBuffer txBuffer(TX_BUFFER_SIZE, ContinuousBuffer::Cached::True);
    H2f::init();

    UserspacePatternGenerator patternGenerator;
    DataMover::init(0, 1, txBuffer.getPhysicalAddress(),
                    txBuffer.getSize(), args.bitrate);

    auto txRingBuffer = RingBuffer<RingBufferType::TX>(txBuffer);

    fmt::println("##########################################");
    fmt::println("                  Begin                   ");
    fmt::println("##########################################");

    patternGenerator.generateAndEnqueueMessage(100_kiB);
    fmt::println("Gen Que Size: {}", patternGenerator.getQueueSize());

    for(size_t i = 0U; i < 20; i++)
    {
        const auto packet = patternGenerator.get();
        txRingBuffer.write(packet);
        DataMover::txSetStopIndex(txRingBuffer.getHead());
        fmt::println("Errors: {}", DataMover::getTxReadErrors());
        fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
        fmt::println("State {}", DataMover::getState());
        fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
        fmt::println("WriteIndex {}", txRingBuffer.getHead());
        fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());
    }

    DataMover::txEnable();

    for(size_t i = 0U; i < 49; i++)
    {
        const auto packet = patternGenerator.get();
        txRingBuffer.write(packet);
        DataMover::txSetStopIndex(txRingBuffer.getHead());
        fmt::println("Errors: {}", DataMover::getTxReadErrors());
        fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
        fmt::println("State {}", DataMover::getState());
        fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
        fmt::println("WriteIndex {}", txRingBuffer.getHead());
        fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());
    }

    sleep(2);
    fmt::println("Errors: {}", DataMover::getTxReadErrors());
    fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
    fmt::println("State {}", DataMover::getState());
    fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
    fmt::println("WriteIndex {}", txRingBuffer.getHead());
    fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());


    patternGenerator.generateAndEnqueueMessage(100_kiB);
    fmt::println("Gen Que Size: {}", patternGenerator.getQueueSize());

    for(size_t i = 0U; i < 20; i++)
    {
        const auto packet = patternGenerator.get();
        txRingBuffer.write(packet);
//        DataMover::txSetStopIndex(txRingBuffer.getHead());
        fmt::println("Errors: {}", DataMover::getTxReadErrors());
        fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
        fmt::println("State {}", DataMover::getState());
        fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
        fmt::println("WriteIndex {}", txRingBuffer.getHead());
        fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());
    }

//    DataMover::txEnable();

    for(size_t i = 0U; i < 49; i++)
    {
        const auto packet = patternGenerator.get();
        txRingBuffer.write(packet);
        DataMover::txSetStopIndex(txRingBuffer.getHead());
        fmt::println("Errors: {}", DataMover::getTxReadErrors());
        fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
        fmt::println("State {}", DataMover::getState());
        fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
        fmt::println("WriteIndex {}", txRingBuffer.getHead());
        fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());
    }

    sleep(2);
    fmt::println("Errors: {}", DataMover::getTxReadErrors());
    fmt::println("Underrun Flag: {}", DataMover::isTxUnderrunFlagSet());
    fmt::println("State {}", DataMover::getState());
    fmt::println("ReadIndex {}", DataMover::getTxReadIndex());
    fmt::println("WriteIndex {}", txRingBuffer.getHead());
    fmt::println("StopIndex {}\n", DataMover::txGetStopIndex());

    txBuffer.cleanup();

}
