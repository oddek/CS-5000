
#ifndef SUT_DECIMATIONCHAIN_H
#define SUT_DECIMATIONCHAIN_H

#include <cstddef>
#include <cstdint>
#include "PacketQueue.h"
#include "UserspacePatternVerifier.h"

class SwDecoder
{
  public:
    SwDecoder();
    explicit SwDecoder(size_t decimationLevel_);
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
    const size_t decimationLevel;

    UserspacePatternVerifier patternVerifier{4};
    PacketQueue<> packetInQueue{};
    PacketQueue<> packetOutQueue{};
    Packet<> currentOutPacket{};

    size_t totalBytesDecimated{0};
};

#endif // SUT_DECIMATIONCHAIN_H
