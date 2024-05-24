
#ifndef SUT_TXDRIVER_H
#define SUT_TXDRIVER_H

#include "PacketQueue.h"
#include "RingBuffer.h"
#include "UserspacePatternVerifier.h"

class TxDriver
{
  public:
    enum class State
    {
        Idle,
        Active,
        Done
    };

    explicit TxDriver(RingBuffer<RingBufferType::TX>& ringBuffer_);

    bool update();
    void put(const Packet<>& packet);
    [[nodiscard]] size_t getQueueSize() const;
    [[nodiscard]] size_t getQueueMaxSize() const;
    [[nodiscard]] bool queueIsEmpty() const;


    void printStats();
    void printFinalStats() const;

    void setLastFlag();

  private:
    static constexpr std::array stateStrings = {"Idl", "Act", "Don"};
    size_t currentBytesTransmitted{0};
    RingBuffer<RingBufferType::TX> ringBuffer;
    PacketQueue<> inQueue{};
    State currentState{};
//    UserspacePatternVerifier patternVerifier{8};

    static constexpr size_t minSizeBeforeDmaStart = 64_kiB;

    size_t underruns{0U};

};

#endif // SUT_TXDRIVER_H
