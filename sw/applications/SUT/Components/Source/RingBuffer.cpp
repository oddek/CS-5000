
#include "RingBuffer.h"
#include "DataMover.h"
#include <algorithm>

template <RingBufferType Type>
RingBuffer<Type>::RingBuffer(ContinuousBuffer& buffer)
    : startPointer(buffer.getPointer()), bufferSize(buffer.getSize()),
      physicalStartPointer(buffer.getPhysicalAddress()), currentOperatePointer(buffer.getPointer())
{
    (void)startPointer;
    if constexpr (Type == RingBufferType::RX)
    {
        minMaxRingBufferSize = 0U;
        margin = 4_kiB;
    }
    else if constexpr (Type == RingBufferType::TX)
    {
        minMaxRingBufferSize = bufferSize;
        margin = 64_kiB;
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::getCapacity() const
{
    return bufferSize;
}

template <RingBufferType Type>
size_t RingBuffer<Type>::getMinMaxRingBufferSize() const
{
    return minMaxRingBufferSize;
}

template <RingBufferType Type> size_t RingBuffer<Type>::getHead() const
{
    if constexpr (Type == RingBufferType::RX)
    {
        const size_t currentWritePointer = DataMover::getRxWritePointer();
        return currentWritePointer - physicalStartPointer;
    }
    else if constexpr (Type == RingBufferType::TX)
    {
        return static_cast<size_t>(currentOperatePointer - startPointer);
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::getTail() const
{
    if constexpr (Type == RingBufferType::RX)
    {
        return static_cast<size_t>(currentOperatePointer - startPointer);
    }
    else if constexpr (Type == RingBufferType::TX)
    {
        const uint32_t currentReadPointer = DataMover::getTxReadPointer();
        return currentReadPointer - physicalStartPointer;
    }
}

template <RingBufferType Type> [[maybe_unused]] bool RingBuffer<Type>::isFull() const
{
    return (getActualFreeBytes() < sizeof(uint32_t));
}

template <RingBufferType Type> bool RingBuffer<Type>::isEmpty() const
{
    return (getActualNumberOfItems() == 0U);
}

template <RingBufferType Type> void RingBuffer<Type>::clear()
{
    currentOperatePointer = startPointer;
}

template <RingBufferType Type> size_t RingBuffer<Type>::getActualNumberOfItems() const
{
    const auto head = getHead();
    const auto tail = getTail();

    if (head == tail)
    {
        return 0;
    }

    else if (head > tail)
    {
        return head - tail;
    }

    else
    {
        return bufferSize + head - tail - 1U;
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::getNumberOfItems() const
{
    const size_t size = getActualNumberOfItems();

    if (size < margin)
    {
        return 0;
    }

    else
    {
        return size - margin;
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::getFreeBytes() const
{
    if constexpr (Type == RingBufferType::RX)
    {
        throw std::runtime_error("getFreeBytes Called on RX RingBuffer");
    }
    else
    {
        const auto actualFreeBytes = getActualFreeBytes();

        return (actualFreeBytes > margin) ? actualFreeBytes : 0U;
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::getActualFreeBytes() const
{
    if constexpr (Type == RingBufferType::RX)
    {
        throw std::runtime_error("getFreeBytes Called on RX RingBuffer");
    }
    else
    {
        return (getCapacity() - getActualNumberOfItems() - 1U);
    }
}

template <RingBufferType Type> void RingBuffer<Type>::incrementOperatePointer(size_t increment)
{
    if constexpr (Type == RingBufferType::RX)
    {
        const size_t newIndex = (getTail() + increment) % bufferSize;
        currentOperatePointer = startPointer + newIndex;
    }
    else if constexpr (Type == RingBufferType::TX)
    {
        const size_t newIndex = (getHead() + increment) % getCapacity();
        currentOperatePointer = startPointer + newIndex;
    }
}

template <RingBufferType Type> size_t RingBuffer<Type>::write(const std::vector<uint8_t>& vec)
{
    if constexpr (Type == RingBufferType::RX)
    {
        throw std::runtime_error("Write Called on RX RingBuffer");
    }
    const size_t sizeLeft = getCapacity() - getHead();

    if (sizeLeft < vec.size())
    {
        std::copy(vec.begin(), vec.begin() + sizeLeft, currentOperatePointer);
        incrementOperatePointer(sizeLeft);

        std::copy(vec.begin() + sizeLeft, vec.end(), currentOperatePointer);
        incrementOperatePointer(vec.size() - sizeLeft);
    }

    else
    {
        std::copy(vec.begin(), vec.end(), currentOperatePointer);
        incrementOperatePointer(vec.size());
    }

    return vec.size();
}

template <RingBufferType Type> size_t RingBuffer<Type>::write(const Packet<>& packet)
{
    return write(packet.getPayload(), packet.getSize());
}

template <RingBufferType Type>
size_t RingBuffer<Type>::write(const std::array<uint8_t, Packet<>::DEFAULT_SIZE>& buffer, size_t nBytes)
{
    if constexpr (Type == RingBufferType::RX)
    {
        throw std::runtime_error("Write Called on RX RingBuffer");
    }
    const size_t sizeLeft = getCapacity() - getHead();

    if (sizeLeft < nBytes)
    {
        std::copy(buffer.begin(), buffer.begin() + sizeLeft, currentOperatePointer);
        incrementOperatePointer(sizeLeft);

        std::copy(buffer.begin() + sizeLeft, buffer.begin() + nBytes, currentOperatePointer);
        incrementOperatePointer(nBytes - sizeLeft);
    }

    else
    {
        std::copy(buffer.begin(), buffer.begin() + nBytes, currentOperatePointer);
        incrementOperatePointer(nBytes);
    }

    return nBytes;
}

template <RingBufferType Type>
Packet<> RingBuffer<Type>::readPacket(RingBuffer::ReadSetting readSetting)
{
    auto [buffer, size] = read(readSetting);
    return Packet<>(buffer, size);
}

template <RingBufferType Type>
std::tuple<std::array<uint8_t, Packet<>::DEFAULT_SIZE>, size_t> RingBuffer<Type>::read(
    ReadSetting readSetting)
{
    std::array<uint8_t, Packet<>::DEFAULT_SIZE> readBuffer{0U};
    if constexpr (Type == RingBufferType::TX)
    {
        throw std::runtime_error("Read Called on TX RingBuffer");
    }

    const size_t items = (readSetting == ReadSetting::SafetyMargin) ? getNumberOfItems() : getActualNumberOfItems();
    const size_t toRead = (items > Packet<>::DEFAULT_SIZE) ? Packet<>::DEFAULT_SIZE : items;

    if ((getTail() + toRead) >= bufferSize)
    {
        const size_t sizeLeft = bufferSize - getTail();
        std::copy(currentOperatePointer, currentOperatePointer + sizeLeft, std::begin(readBuffer));
        incrementOperatePointer(sizeLeft);
        std::copy(currentOperatePointer, currentOperatePointer + (toRead - sizeLeft),
                  std::begin(readBuffer) + sizeLeft);
        incrementOperatePointer(toRead - sizeLeft);
    }

    else
    {
        std::copy(currentOperatePointer, currentOperatePointer + toRead, std::begin(readBuffer));
        incrementOperatePointer(toRead);
    }

    return {readBuffer, toRead};
}

template <RingBufferType Type>
void RingBuffer<Type>::updateMinMaxRingBufferSize()
{
    if constexpr (Type == RingBufferType::RX)
    {
        if (getActualNumberOfItems() > minMaxRingBufferSize)
        {
            minMaxRingBufferSize = getActualNumberOfItems();
        }
    }
    else if constexpr (Type == RingBufferType::TX)
    {
        if (getActualNumberOfItems() < minMaxRingBufferSize)
        {
            minMaxRingBufferSize = getActualNumberOfItems();
        }
    }
}

template class RingBuffer<RingBufferType::RX>;
template class RingBuffer<RingBufferType::TX>;
