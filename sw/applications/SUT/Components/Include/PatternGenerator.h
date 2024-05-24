
#ifndef SUT_PATTERNGENERATOR_H
#define SUT_PATTERNGENERATOR_H

#include "PacketQueue.h"
#include "BitrateCalc.h"

class PatternGenerator
{
  public:
    virtual ~PatternGenerator()= default;
    virtual void generateAndEnqueueMessage(size_t messageSize) = 0;
    virtual void reset() = 0;
    virtual void update() {}

    Packet<> get();
    [[nodiscard]] bool packetAvailable() const;
    [[nodiscard]] size_t getQueueSize() const;
    [[nodiscard]] size_t getQueueMaxSize() const;
    [[nodiscard]] bool queueIsEmpty() const;
    void printStats() const;
    void printFinalStats() const;
    void updateBitrate();
    std::string getFormattedBitrate();
    std::string getFormattedAverageBitrate();


  protected:
    PacketQueue<QueueType::Blocking> packetQueue{};
//    PacketQueue<QueueType::NonBlocking> packetQueue{};
    size_t bytesGenerated = 0U;
    size_t totalBytesGenerated = 0U;
    size_t packetsGenerated = 0U;
    size_t messagesGenerated = 0U;

    BitrateCalc bitrateCalc;
};

#endif // SUT_PATTERNGENERATOR_H
