
#ifndef ENCODER_H
#define ENCODER_H

#include "HardwareCodec.h"
#include "KernelPatternGenerator.h"
#include "PacketQueue.h"
#include "UserspacePatternGenerator.h"
#include "UserspacePatternVerifier.h"
#include <array>
#include <cstdint>

class HwEncoder
{
  public:
    HwEncoder();
    explicit HwEncoder(size_t interpolationFactor_);
    void put(const Packet<>& packet);
    Packet<> get();
    bool packetAvailable();
    void update();
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
    typedef enum
    {
        First = 0,
        InBetween,
        Last
    }OutPacketOrder;

    void encode();
    void addPaddingToInputPacket();
    void removePaddingFromOutputPacket();
    void putOutputPacketToQueue();
    bool fullWordLeftInInputBuffer(size_t bytesEncoded);

    PacketQueue<> packetInQueue{};
    PacketQueue<> packetOutQueue{};

    UserspacePatternVerifier patternVerifier{4};
    HardwareCodec<CodecType::Encode> hardwareCodec{4};

    Packet<> inputPacket{};
    Packet<> outputPacket{};
    OutPacketOrder outPacketOrder{OutPacketOrder::First};

    size_t currentPaddingBytes{0};

    uint64_t writesPerformed{0U};
    uint64_t readsPerformed{0U};

    const size_t interpolationFactor{1};
};

#endif // ENCODER_H
