
#include "RxPipeline.h"
#include "DataMover.h"
#include "H2f.h"
#include "SwDecoder.h"
#include <thread>

RxPipeline::RxPipeline(RxDriver& addaRx_, SwDecoder& decimationChain_, HwDecoder& decoder_,
                   PatternVerifier& patternVerifier_)
    : addaRx(addaRx_), decimationChain(decimationChain_), decoder(decoder_), patternVerifier(patternVerifier_)
{
}

bool RxPipeline::update()
{
    const bool runMoreSharedMem = addaRx.update();
//    if( runMoreSharedMem)
//    {
//        fmt::println("runMoreSharedMem {}", runMoreSharedMem);
//    }

    while (addaRx.packetAvailable())
    {
        const auto packet = addaRx.get();
        decimationChain.put(packet);
    }

    decimationChain.update();

    while( decimationChain.packetAvailable())
    {
        const auto packet = decimationChain.get();
        decoder.put(packet);
    }

    decoder.update();

    while (decoder.packetAvailable())
    {
        const auto packet = decoder.get();
        patternVerifier.put(packet);
    }

    patternVerifier.update();

    // If stop is called we return false as soon as ringBuffer is cleared
    if (stopAndExit)
    {
        aboutToExit = true;
        return false;
    }

    return !allQueuesEmpty() || runMoreSharedMem;

//    if (allQueuesEmpty())
//    {
//        return false;
//    }
//    else
//    {
//        return true;
//    }
//
//    return false;
}

bool RxPipeline::allQueuesEmpty()
{
    return patternVerifier.queueIsEmpty() && decoder.outputQueueIsEmpty() && decoder.inputQueueIsEmpty() && decimationChain.outputQueueIsEmpty() &&
           addaRx.queueIsEmpty();
}

void RxPipeline::stop()
{
    DataMover::rxDisable();
    stopAndExit = true;

    while (!aboutToExit)
    {
        static constexpr auto waitForExitSleepTime = std::chrono::microseconds(500);
        std::this_thread::sleep_for(waitForExitSleepTime);
    }
}
void RxPipeline::printFinalStats()
{
}

void RxPipeline::printStats()
{
    fmt::println("RxPipeline");
}

void RxPipeline::printFinalQueues() const
{
    fmt::println("QueMaxS  - "
                 "AdRx: {:<4} "
                 "DChIn: {:<4} "
                 "DChOut: {:<4} "
                 "DecIn: {:<4} "
                 "DecOut: {:<4} "
                 "Verifier: {:<4}\n",
                 addaRx.getQueueMaxSize(), decimationChain.getInputQueueMaxSize(), decimationChain.getOutputQueueMaxSize(), decoder.getInputQueueMaxSize(), decoder.getOutputQueueMaxSize(),
                 patternVerifier.getQueueMaxSize());
}

void RxPipeline::printQueues()
{
    fmt::println("QueSize  - "
                 "AdRx: {:<4} "
                 "DChIn: {:<4} "
                 "DChOut: {:<4} "
                 "DecIn: {:<4} "
                 "DecOut: {:<4} "
                 "Verifier: {:<4}",
                 addaRx.getQueueSize(), decimationChain.getInputQueueSize(), decimationChain.getOutputQueueSize(), decoder.getInputQueueSize(), decoder.getOutputQueueSize(),
                 patternVerifier.getQueueSize());

    fmt::println("QueMaxS  - "
                 "AdRx: {:<4} "
                 "DChIn: {:<4} "
                 "DChOut: {:<4} "
                 "DecIn: {:<4} "
                 "DecOut: {:<4} "
                 "Verifier: {:<4}",
                 addaRx.getQueueMaxSize(), decimationChain.getInputQueueMaxSize(), decimationChain.getOutputQueueMaxSize(), decoder.getInputQueueMaxSize(), decoder.getOutputQueueMaxSize(),
                 patternVerifier.getQueueMaxSize());
}
