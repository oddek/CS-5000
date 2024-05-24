
#include "TxPipeline.h"
#include "DataMover.h"
#include <array>
#include <stdexcept>
#include <thread>
#include <utility>

TxPipeline::TxPipeline(TxDriver& addaTx_, HwEncoder& encoder_, SwEncoder& interpolationChain_,
                   PatternGenerator& patternGenerator_)
    : addaTx(addaTx_), encoder(encoder_), interpolationChain(interpolationChain_), patternGenerator(patternGenerator_)
{
}

bool TxPipeline::clearTransmission()
{
    patternGenerator.reset();
    encoder.reset();
    return true;
}

bool TxPipeline::send(size_t nBytes)
{
    patternGenerator.generateAndEnqueueMessage(nBytes);
    return true;
}

bool TxPipeline::update()
{
    static bool asked = false;
    if (shouldAskMore() && asked == false)
    {
        asked = true;
        patternGenerator.generateAndEnqueueMessage(1500U);
    }

    //    static auto startTime = getTimespecNow();
    //
    //    if(getTimeElapsedMs(startTime) > 14)
    //    {
    //        patternGenerator.generateAndEnqueueMessage(1500U);
    //        startTime = getTimespecNow();
    //    }

    while (patternGenerator.packetAvailable())
    {
        asked = false;
        const auto packet = patternGenerator.get();
        encoder.put(packet);
    }

    encoder.update();

    while (encoder.packetAvailable())
    {
        const auto packet = encoder.get();

        interpolationChain.put(packet);
    }

    interpolationChain.update();

    while (interpolationChain.packetAvailable())
    {
        const auto packet = interpolationChain.get();

        addaTx.put(packet);
    }

    addaTx.update();

    if (allQueuesEmpty())
    {
//        addaTx.setLastFlag();
        return false;
    }

    return true;
}

void TxPipeline::stop()
{
    while (DataMover::txIsEnabled())
    {
        DataMover::txDisable();
    }
    stopAndExit = true;

    while (!aboutToExit)
    {
        static constexpr auto waitForExitSleepTime = std::chrono::microseconds(500);
        std::this_thread::sleep_for(waitForExitSleepTime);
    }
}

void TxPipeline::printFinalStats() const
{
//       fmt::println("**************");
//       fmt::println("TxPipeline Stats");
//       fmt::println("**************");
//       fmt::println("{:30} {}",
//                    "Bytes transmitted:", convertBytesToString(DataMover::txReadsPerformed() * sizeof(uint64_t)));
//       fmt::println("{:30} {}", "Bytes received successfully:", convertBytesToString(currentBytesTransmitted));
//       //    fmt::println("{:30} {}", "Bytes left in buffer:",
//       convertBytesToString(ringBuffer.getActualNumberOfItems())); fmt::println("{:30} {}", "Errors:",
//       DataMover::getTxReadErrors());
//       //    fmt::println("{:30} {}\n", "Min Buffer Size:",
//       convertBytesToString(ringBuffer.getMinMaxRingBufferSize()));
}

void TxPipeline::printStats() const
{
//       fmt::println(
//           "TX - "
//           "{:3} - "
//           "Sent: {:>11}  "
//           "Rcv: {:>11}  "
//           "Buf: {:>11}  "
//           "Err: {:>11} ",
//           stateStrings.at(static_cast<uint8_t>(currentState.load())), convertBytesToString(currentBytesTransmitted),
//           convertBytesToString(DataMover::txReadsPerformed() * sizeof(uint64_t)),
//           //                 convertBytesToString(ringBuffer.getActualNumberOfItems()),
//           DataMover::getTxReadErrors()); 0U, DataMover::getTxReadErrors());
}

void TxPipeline::printFinalQueues() const
{
    fmt::println("QueMaxS  - "
                 "Gen: {:<4} "
                 "EncIn: {:<4} "
                 "EncOut: {:<4} "
                 "IntIn: {:<4} "
                 "IntOut: {:<4} "
                 "AdTx: {:<4}\n",
                 patternGenerator.getQueueMaxSize(), encoder.getInputQueueMaxSize(), encoder.getOutputQueueMaxSize(),
                 interpolationChain.getInputQueueMaxSize(), interpolationChain.getOutputQueueMaxSize(),
                 addaTx.getQueueMaxSize());
}

void TxPipeline::printQueues() const
{
    fmt::println("QueSize  - "
                 "Gen: {:<4} "
                 "EncIn: {:<4} "
                 "EncOut: {:<4} "
                 "IntIn: {:<4} "
                 "IntOut: {:<4} "
                 "AdTx: {:<4}",
                 patternGenerator.getQueueSize(), encoder.getInputQueueSize(), encoder.getOutputQueueSize(),
                 interpolationChain.getInputQueueSize(), interpolationChain.getOutputQueueSize(),
                 addaTx.getQueueSize());

    fmt::println("QueMaxS  - "
                 "Gen: {:<4} "
                 "EncIn: {:<4} "
                 "EncOut: {:<4} "
                 "IntIn: {:<4} "
                 "IntOut: {:<4} "
                 "AdTx: {:<4}",
                 patternGenerator.getQueueMaxSize(), encoder.getInputQueueMaxSize(), encoder.getOutputQueueMaxSize(),
                 interpolationChain.getInputQueueMaxSize(), interpolationChain.getOutputQueueMaxSize(),
                 addaTx.getQueueMaxSize());
}

bool TxPipeline::allQueuesEmpty()
{
    return patternGenerator.queueIsEmpty() && encoder.inputQueueIsEmpty() && encoder.outputQueueIsEmpty() &&
           addaTx.queueIsEmpty();
//    return encoder.inputQueueIsEmpty() && encoder.outputQueueIsEmpty() &&
//           addaTx.queueIsEmpty();
}

bool TxPipeline::shouldAskMore()
{
    return allQueuesEmpty();
//    static size_t queueLimitForAsk = 1U;
//    return ((patternGenerator.getQueueSize() < queueLimitForAsk) && (encoder.getInputQueueSize() < queueLimitForAsk) &&
//            (encoder.getOutputQueueSize() < queueLimitForAsk) &&
//            (interpolationChain.getInputQueueSize() < queueLimitForAsk) &&
//            (interpolationChain.getOutputQueueSize() < queueLimitForAsk) &&
//            (addaTx.getQueueSize() < queueLimitForAsk));
//    return ((addaTx.getQueueSize() < 10U));// && (encoder.getInputQueueSize() < queueLimitForAsk) &&

}