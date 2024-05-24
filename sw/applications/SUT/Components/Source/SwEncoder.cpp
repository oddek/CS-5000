

#include "SwEncoder.h"

SwEncoder::SwEncoder() : SwEncoder(2)
{
}

SwEncoder::SwEncoder(size_t interpolationLevel_) : interpolationLevel(interpolationLevel_)
{
//    patternVerifier.start();
}

void SwEncoder::update()
{
    while (!inputQueueIsEmpty())
    {
        const auto inputPacket = packetInQueue.get();
        Packet<> outputPacket;

        size_t bytesInterpolated{0U};
        auto [inputPacketIter, inputPacketEnd] = inputPacket.getByteIterators();
        auto [outputPacketIter, outputPacketEnd] = outputPacket.getByteIterators();

        while (bytesInterpolated != inputPacket.getSize())
        {
            if (outputPacketIter == outputPacketEnd)
            {
                packetOutQueue.put(outputPacket);
                patternVerifier.put(outputPacket);
                patternVerifier.update();
                outputPacket.clear();
                auto [newOutputPacketIter, newOutputPacketEnd] = outputPacket.getByteIterators();
                outputPacketIter = newOutputPacketIter;
            }

            for (size_t i = 0U; i < interpolationLevel; i++)
            {
                *outputPacketIter = *inputPacketIter;
                outputPacketIter++;
            }

            inputPacketIter++;
            outputPacket.setSize(outputPacket.getSize() + interpolationLevel);
            bytesInterpolated++;
        }

        if (outputPacket.getSize() != 0U)
        {
            packetOutQueue.put(outputPacket);
            patternVerifier.put(outputPacket);
            patternVerifier.update();
            outputPacket.clear();
        }

        totalBytesInterpolated += inputPacket.getSize();
    }
}

void SwEncoder::put(const Packet<>& packet)
{
    packetInQueue.put(packet);
}

Packet<> SwEncoder::get()
{
    return packetOutQueue.get();
}

bool SwEncoder::packetAvailable()
{
    return !packetOutQueue.isEmpty();
}

void SwEncoder::reset()
{
    packetInQueue.clear();
    packetOutQueue.clear();
}

void SwEncoder::printStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("IntChain - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Err: {:>11}",
                 convertBytesToString(totalBytesInterpolated),
                 convertBytesToString(verifierStats.totalBytesVerified),
                 verifierStats.errors);
}

void SwEncoder::printFinalStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("**************");
    fmt::println("IntChain Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Sent:", convertBytesToString(totalBytesInterpolated));
    fmt::println("{:30} {}", "Rcv:", convertBytesToString(verifierStats.bytesVerified));
    fmt::println("{:30} {}", "Errors:", verifierStats.errors);
    fmt::println("");
}

size_t SwEncoder::getOutputQueueSize() const
{
    return packetOutQueue.getSize();
}

size_t SwEncoder::getInputQueueSize() const
{
    return packetInQueue.getSize();
}

size_t SwEncoder::getOutputQueueMaxSize() const
{
    return packetOutQueue.getMaxSize();
}

size_t SwEncoder::getInputQueueMaxSize() const
{
    return packetInQueue.getMaxSize();
}

bool SwEncoder::inputQueueIsEmpty() const
{
    return packetInQueue.isEmpty();
}

bool SwEncoder::outputQueueIsEmpty() const
{
    return packetOutQueue.isEmpty();
}
