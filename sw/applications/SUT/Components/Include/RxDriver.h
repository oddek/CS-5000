
#ifndef SUT_RXDRIVER_H
#define SUT_RXDRIVER_H

#include "PacketQueue.h"
#include "Packet.h"
#include "RingBuffer.h"

class RxDriver
{
  public:
    explicit RxDriver(RingBuffer<RingBufferType::RX>& ringBuffer_, bool verifyOnTheFly_, uint32_t busyWaitUs_,
                    bool readUntilEmpty_ = false);
    bool update();
    bool packetAvailable();
    Packet<> get();
    size_t getQueueSize() const;
    size_t getQueueMaxSize() const;
    bool queueIsEmpty() const;

    void printStats();
    void printFinalStats();


  private:
    Packet<> latestPacket{};
    UserspacePatternVerifier patternVerifier{8};
    bool getNewPacket();
    RingBuffer<RingBufferType::RX>& ringBuffer;
    PacketQueue<> outQueue{};
    const bool verifyOnTheFly{};
    const uint32_t busyWaitUs{};
    const bool readUntilEmpty{};
};


#endif // SUT_RXDRIVER_H
