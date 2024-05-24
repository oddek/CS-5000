
#ifndef SUT_INTERPOLATIONCHAIN_H
#define SUT_INTERPOLATIONCHAIN_H

#include <cstddef>
#include <cstdint>
#include "PacketQueue.h"
#include "UserspacePatternVerifier.h"

class SwEncoder
{
  public:
    SwEncoder();
    explicit SwEncoder(size_t interpolationLevel_);
    void update();
    void put(const Packet<>& packet);
    Packet<> get();
    bool packetAvailable();
    void reset();
    void printStats();
    void printFinalStats();
    [[nodiscard]] size_t getOutputQueueSize() const;
    [[nodiscard]] size_t getInputQueueSize() const;
    [[nodiscard]] size_t getOutputQueueMaxSize() const;
    [[nodiscard]] size_t getInputQueueMaxSize() const;
    [[nodiscard]] bool inputQueueIsEmpty() const;
    [[nodiscard]] bool outputQueueIsEmpty() const;

  private:
    const size_t interpolationLevel;

    UserspacePatternVerifier patternVerifier{8};
    PacketQueue<> packetInQueue{};
    PacketQueue<> packetOutQueue{};

    size_t totalBytesInterpolated{0};
};

#endif // SUT_INTERPOLATIONCHAIN_H
