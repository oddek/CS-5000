
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "ContinuousBuffer.h"
#include "Packet.h"
#include "UserspacePatternVerifier.h"
#include "Utils.h"

enum class RingBufferType
{
    RX,
    TX
};

template <RingBufferType Type> class RingBuffer
{
  public:
    enum class ReadSetting
    {
        All,
        SafetyMargin
    };

    explicit RingBuffer( ContinuousBuffer& buffer );

    size_t write( const std::vector<uint8_t>& vec );
    size_t write( const Packet<>& packet );
    size_t write( const std::array<uint8_t, Packet<>::DEFAULT_SIZE>& buffer, size_t nBytes );

    Packet<> readPacket( ReadSetting readSetting );
    std::tuple<std::array<uint8_t, Packet<>::DEFAULT_SIZE>, size_t> read( ReadSetting readSetting );
    void clear();

    [[nodiscard]] size_t getCapacity() const;
    [[maybe_unused]] [[nodiscard]] bool isFull() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] size_t getNumberOfItems() const;
    [[nodiscard]] size_t getActualNumberOfItems() const;
    [[nodiscard]] size_t getFreeBytes() const;
    [[nodiscard]] size_t getActualFreeBytes() const;
    [[nodiscard]] size_t getMinMaxRingBufferSize() const;
    void updateMinMaxRingBufferSize();
    [[nodiscard]] size_t getTail() const;
    [[nodiscard]] size_t getHead() const;

  private:
    void incrementOperatePointer( size_t increment );

    volatile uint8_t* const startPointer;
    const size_t bufferSize;
    const uint32_t physicalStartPointer;
    volatile uint8_t* currentOperatePointer;

    size_t margin{};
    size_t minMaxRingBufferSize{};
};

#endif // RINGBUFFER_H
