
#include "HwEncoder.h"
#include "Utils.h"
#include <fmt/core.h>

HwEncoder::HwEncoder() : HwEncoder(1)
{
}

HwEncoder::HwEncoder(size_t interpolationFactor_) : hardwareCodec(interpolationFactor_), interpolationFactor(interpolationFactor_)
{
//    patternVerifier.start();
}

void HwEncoder::update()
{
    while (!packetInQueue.isEmpty())
    {
        inputPacket = packetInQueue.get();
        writesPerformed += inputPacket.getSize() / sizeof(uint32_t);


        encode();
        patternVerifier.update();
    }
}

void HwEncoder::encode()
{
    outPacketOrder = OutPacketOrder::First;

    size_t bytesEncoded{0U};
    auto [inputPacketIter, inputPacketEnd] = inputPacket.getWordIterators();
    auto [outputPacketIter, outputPacketEnd] = outputPacket.getWordIterators();

    addPaddingToInputPacket();

    while (true)
    {
        while (!hardwareCodec.isFull() && fullWordLeftInInputBuffer(bytesEncoded))
        {
            hardwareCodec.putWord(*inputPacketIter);
            inputPacketIter++;
            bytesEncoded += sizeof(uint32_t);
        }

        while (!hardwareCodec.isEmpty())
        {
            if (outputPacketIter == outputPacketEnd)
            {
                putOutputPacketToQueue();
                auto [newOutputPacketIter, newOutputPacketEnd] = outputPacket.getWordIterators();
                outputPacketIter = newOutputPacketIter;
                outputPacketEnd = newOutputPacketEnd;
            }

            *outputPacketIter = hardwareCodec.getWord();
            outputPacketIter++;
            outputPacket.setSize(outputPacket.getSize() + sizeof(uint32_t));
        }

        if (!fullWordLeftInInputBuffer(bytesEncoded))
        {
            outPacketOrder = OutPacketOrder::Last;


            putOutputPacketToQueue();
            return;
        }
    }
}

void HwEncoder::putOutputPacketToQueue()
{
    if (outPacketOrder == OutPacketOrder::First && inputPacket.isFirstPacket())
    {
        outputPacket.setMetaData(Packet<>::MetaData::FirstPacket);
        outPacketOrder = InBetween;
    }
    else if (outPacketOrder == OutPacketOrder::Last && inputPacket.isLastPacket())
    {
        outputPacket.setMetaData(Packet<>::MetaData::LastPacket);
        removePaddingFromOutputPacket();
    }
    else
    {
        outputPacket.setMetaData(Packet<>::MetaData::None);
    }

    outputPacket.setTotalMessageSize(inputPacket.getTotalMessageSize() * interpolationFactor);

    packetOutQueue.put(outputPacket);
    readsPerformed += outputPacket.getSize();
    patternVerifier.put(outputPacket);
    patternVerifier.update();

    outputPacket.clear();
}

void HwEncoder::reset()
{
    inputPacket.clear();
    patternVerifier.reset();
    packetInQueue.clear();
    hardwareCodec.clear();
}

bool HwEncoder::fullWordLeftInInputBuffer(size_t bytesEncoded)
{
    return ((inputPacket.getSize() - bytesEncoded) >= sizeof(uint32_t));
}

Packet<> HwEncoder::get()
{
    return packetOutQueue.get();
}

bool HwEncoder::packetAvailable()
{
    return !packetOutQueue.isEmpty();
}

void HwEncoder::put(const Packet<>& packet)
{
    packetInQueue.put(packet);
}

size_t HwEncoder::getOutputQueueSize() const
{
    return packetOutQueue.getSize();
}

size_t HwEncoder::getInputQueueSize() const
{
    return packetInQueue.getSize();
}

void HwEncoder::printStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("HwEncoder  - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Fif: {:>11}  "
                 "Err: {:>11}",
                 convertBytesToString(writesPerformed * sizeof(uint32_t)),
                 convertBytesToString(verifierStats.totalBytesVerified), hardwareCodec.getFifoState(),
                 verifierStats.errors);
}

void HwEncoder::printFinalStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("**************");
    fmt::println("HwEncoder Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Bytes transmitted:", convertBytesToString(writesPerformed * sizeof(uint32_t)));
    fmt::println("{:30} {}", "Bytes received successfully:", convertBytesToString(verifierStats.bytesVerified));
    fmt::println("{:30} {}", "Fifo:", hardwareCodec.getFifoState());
    fmt::println("{:30} {}", "Errors:", verifierStats.errors);
    fmt::println("{:30} {}", "Writes:", writesPerformed * 4);
    fmt::println("{:30} {}", "Reads:", readsPerformed * 4);
    fmt::println("{:30} {}", "Success:", verifierStats.totalBytesVerified);
    fmt::println("");
}

void HwEncoder::addPaddingToInputPacket()
{
    if (inputPacket.isLastPacket())
    {
        currentPaddingBytes = std::abs(0 - static_cast<int>((inputPacket.getSize() % sizeof(uint32_t))));

        if (currentPaddingBytes != 0)
        {
            auto [inputPacketIter, inputPacketEnd] = inputPacket.getWordIterators();
            std::memset(inputPacketIter + inputPacket.getSize(), 0, currentPaddingBytes);
            inputPacket.setSize(inputPacket.getSize() + currentPaddingBytes);
        }
    }
}

void HwEncoder::removePaddingFromOutputPacket()
{
    if (currentPaddingBytes != 0)
    {
        outputPacket.setSize(outputPacket.getSize() - currentPaddingBytes);
    }
}

bool HwEncoder::outputQueueIsEmpty() const
{
    return packetOutQueue.isEmpty();
}

bool HwEncoder::inputQueueIsEmpty() const
{
    return packetInQueue.isEmpty();
}

size_t HwEncoder::getOutputQueueMaxSize() const
{
    return packetOutQueue.getMaxSize();
}

size_t HwEncoder::getInputQueueMaxSize() const
{
    return packetInQueue.getMaxSize();
}
