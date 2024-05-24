
#ifndef RX_WORKER_H
#define RX_WORKER_H

#include "HwDecoder.h"
#include "Packet.h"
#include "RingBuffer.h"
#include "RxDriver.h"
#include "SwDecoder.h"
#include "UserspacePatternVerifier.h"
#include "Utils.h"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <pthread.h>

class RxPipeline
{
  public:
    explicit RxPipeline(RxDriver& addaRx_, SwDecoder& decimationChain_, HwDecoder& decoder_,
                      PatternVerifier& patternVerifier_);

    void stop();
    bool update();
    void printStats();
    void printQueues();
    void printFinalQueues() const;
    void printFinalStats();

  private:
    RxDriver& addaRx;
    SwDecoder& decimationChain;
    HwDecoder& decoder;
    PatternVerifier& patternVerifier;

    std::atomic<bool> stopAndExit{false};
    std::atomic<bool> aboutToExit{false};
    bool allQueuesEmpty();
};

#endif