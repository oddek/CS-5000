
#ifndef DECODER_H
#define DECODER_H

#include "HardwareCodec.h"
#include "KernelPatternVerifier.h"
#include "Packet.h"
#include "UserspacePatternVerifier.h"
#include <array>
#include <cinttypes>
#include <vector>

class HwDecoder
{
  public:
    HwDecoder()
    {
//        patternVerifier.start();
    }
    static constexpr auto DECODE_BUFFER_SIZE = 4012U;
    void put(const Packet<>& packet);
    Packet<> get();
    bool packetAvailable();
    void update();
    void decode(const Packet<>& packet);

    void printStats();
    void printFinalStats();

    [[nodiscard]] size_t getOutputQueueSize() const;
    [[nodiscard]] size_t getInputQueueSize() const;
    [[nodiscard]] size_t getOutputQueueMaxSize() const;
    [[nodiscard]] size_t getInputQueueMaxSize() const;
    [[nodiscard]] bool inputQueueIsEmpty() const;
    [[nodiscard]] bool outputQueueIsEmpty() const;


  private:
    PacketQueue<> packetInQueue{};
    PacketQueue<> packetOutQueue{};

    UserspacePatternVerifier patternVerifier{1};

    HardwareCodec<CodecType::Decode> hardwareDecoder{4};
    uint32_t inputBuffer[(DECODE_BUFFER_SIZE / sizeof(uint32_t)) + 1U]{0};
    size_t inputBufferSize{0U};
    size_t currentInputBufferIndex{0};

    uint64_t writesPerformed{0U};
    uint64_t readsPerformed{0U};

    void readFromHardwareDecoder();
    void writeToHardwareDecoder();
};

#endif // DECODER_H
