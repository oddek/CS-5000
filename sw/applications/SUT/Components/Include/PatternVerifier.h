
#ifndef SUT_PATTERNVERIFIER_H
#define SUT_PATTERNVERIFIER_H

#include "PacketQueue.h"
#include "BitrateCalc.h"

class PatternVerifier
{
  public:
    struct Stats
    {
        uint32_t errors{0U};
        uint64_t bytesVerified{0U};
        uint64_t totalBytesVerified{0U};
    };
    virtual ~PatternVerifier() = default;
    void start();
    virtual void reset() = 0;
    virtual void update() = 0;
    virtual void verify(const uint8_t* buffer, size_t nBytes) = 0;
    virtual void verify(const Packet<>& packet) {throw std::runtime_error("Not implemented");}

    virtual Stats getStats();

    void printStats();
    void printFinalStats();
    void put(const Packet<>& packet);
    void updateBitrate();
    std::string getFormattedBitrate();
    std::string getFormattedAverageBitrate();

    Stats stats{0};
    [[nodiscard]] size_t getQueueSize() const;
    [[nodiscard]] size_t getQueueMaxSize() const;
    [[nodiscard]] bool queueIsEmpty() const;

    PacketQueue<> packetQueue{};

  protected:
    pthread_t thread{0};

    BitrateCalc bitrateCalc;
};

#endif // SUT_PATTERNVERIFIER_H
