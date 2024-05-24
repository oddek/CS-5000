

#ifndef TX_WORKER_H
#define TX_WORKER_H

#include "HwEncoder.h"
#include "KernelPatternGenerator.h"
#include "RingBuffer.h"
#include "SwEncoder.h"
#include "TxDriver.h"
#include "UserspacePatternGenerator.h"
#include "Utils.h"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <pthread.h>

class TxPipeline
{
  public:
    explicit TxPipeline(TxDriver& addaTx_, HwEncoder& encoder_, SwEncoder& interpolationChain_,
                      PatternGenerator& patternGenerator_);
    bool update();
    bool send(size_t nBytes);
    void stop();
    bool clearTransmission();
    bool allQueuesEmpty();

    void printStats() const;
    void printFinalStats() const;
    void printFinalQueues() const;
    void printQueues() const;

  private:
    TxDriver& addaTx;
    HwEncoder& encoder;
    SwEncoder& interpolationChain;
    PatternGenerator& patternGenerator;

    std::atomic<bool> stopAndExit{false};
    std::atomic<bool> aboutToExit{false};
    bool shouldAskMore();
};

#endif