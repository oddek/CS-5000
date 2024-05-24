
#ifndef PACKET_H
#define PACKET_H

#include "Utils.h"
#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t PACKET_H_DEFAULT_SIZE = 1504U;

template <size_t T = PACKET_H_DEFAULT_SIZE> class Packet
{
  public:
    enum class MetaData
    {
        None,
        OnlyPacket,
        FirstPacket,
        LastPacket
    };
  private:
    std::array<uint8_t, T> payload{0};
    size_t size{0U};
    size_t totalMessageSize{0U};
    MetaData metaData{};


  public:
    static constexpr size_t DEFAULT_SIZE = PACKET_H_DEFAULT_SIZE;
    Packet() = default;
    Packet(std::array<uint8_t, T> payload_, size_t size_) : payload(payload_), size(size_){};
    Packet(const uint8_t* buffer, size_t size_) : size(size_)
    {
        std::copy(buffer, buffer+size_, payload.begin());
    }

    [[nodiscard]] const std::array<uint8_t, T>& getPayload() const
    {
        return payload;
    }

    [[nodiscard]] bool isFirstPacket() const
    {
        return ( (metaData == Packet<>::MetaData::FirstPacket) || ( metaData == Packet<>::MetaData::OnlyPacket ));
    }

    [[nodiscard]] bool isLastPacket() const
    {
        return ( (metaData == Packet<>::MetaData::LastPacket) || ( metaData == Packet<>::MetaData::OnlyPacket ));
    }

    [[nodiscard]] size_t getSize() const
    {
        return size;
    }

    [[nodiscard]] size_t getTotalMessageSize() const
    {
        return totalMessageSize;
    }

    [[nodiscard]] MetaData getMetaData() const
    {
        return metaData;
    }

    void setMetaData(MetaData metaData_)
    {
        metaData = metaData_;
    }

    void setTotalMessageSize(size_t size_)
    {
        totalMessageSize = size_;
    }

    void setSize(size_t size_)
    {
        size = size_;
    }

    [[nodiscard]] std::pair<typename std::array<uint8_t, T>::const_iterator,
                            typename std::array<uint8_t, T>::const_iterator>
    getByteIterators() const
    {
        return {payload.cbegin(), payload.cend()};
    }

    [[nodiscard]] std::pair<typename std::array<uint32_t, T / sizeof(uint32_t)>::const_iterator,
                            typename std::array<uint32_t, T / sizeof(uint32_t)>::const_iterator>
    getWordIterators() const
    {
        return {reinterpret_cast<const uint32_t*>(payload.cbegin()), reinterpret_cast<const uint32_t*>(payload.cend())};
    }

    [[nodiscard]] std::pair<typename std::array<uint32_t, T / sizeof(uint32_t)>::iterator,
                            typename std::array<uint32_t, T / sizeof(uint32_t)>::iterator>
    getWordIterators()
    {
        return {reinterpret_cast<uint32_t*>(payload.begin()), reinterpret_cast<uint32_t*>(payload.end())};
    }

    [[nodiscard]] std::pair<typename std::array<uint8_t, T>::iterator, typename std::array<uint8_t, T>::iterator>
    getByteIterators()
    {
        return {payload.begin(), payload.end()};
    }

    void clear()
    {
        setSize(0U);
    }

    void print()
    {
        std::ostringstream convert;
        for (size_t a = 0; a < getSize(); a++) {
            convert << (int)getPayload()[a];
            convert << ", ";
        }

        std::string str = convert.str();
        fmt::println("{}", str);
    }
};

#endif // PACKET_H
