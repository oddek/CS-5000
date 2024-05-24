
#include "SwDecoder.h"

SwDecoder::SwDecoder() : SwDecoder(2)
{
}

SwDecoder::SwDecoder(size_t decimationLevel_) : decimationLevel(decimationLevel_)
{
    //    patternVerifier.start();
}

void SwDecoder::update()
{
    static auto [outputPacketIter, outputPacketEnd] = currentOutPacket.getByteIterators();
    static auto currentDecimationCount = 0U;
    while (!inputQueueIsEmpty())
    {
        const auto inputPacket = packetInQueue.get();
        size_t bytesDecimated{0U};
        auto [inputPacketIter, inputPacketEnd] = inputPacket.getByteIterators();

        while (inputPacketIter != inputPacketEnd)
        {
            if (outputPacketIter == outputPacketEnd)
            {
                packetOutQueue.put(currentOutPacket);
                patternVerifier.put(currentOutPacket);
                patternVerifier.update();

                currentOutPacket.clear();
                currentDecimationCount = 0U;
                auto [newOutputPacketIter, newOutputPacketEnd] = currentOutPacket.getByteIterators();
                outputPacketIter = newOutputPacketIter;
            }

            *outputPacketIter = *inputPacketIter;
            outputPacketIter++;
            inputPacketIter++;
            bytesDecimated++;
            currentDecimationCount++;

            if( currentDecimationCount == decimationLevel)
            {
                currentDecimationCount = 0U;
                for(size_t i = 0U; i < decimationLevel; i++)
                {
                    inputPacketIter++;
                    if( inputPacketIter == inputPacketEnd)
                    {
//                        continue;
                        break;
                    }
                }
            }
        }

        currentOutPacket.setSize(currentOutPacket.getSize() + (inputPacket.getSize() / decimationLevel));
        totalBytesDecimated += inputPacket.getSize();
    }
}

void SwDecoder::put(const Packet<>& packet)
{
    packetInQueue.put(packet);
}

Packet<> SwDecoder::get()
{
    return packetOutQueue.get();
}

bool SwDecoder::packetAvailable()
{
    return !packetOutQueue.isEmpty();
}

void SwDecoder::reset()
{
    packetInQueue.clear();
    packetOutQueue.clear();
}

void SwDecoder::printStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("DecChain - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Err: {:>11}",
                 convertBytesToString(totalBytesDecimated),
                 convertBytesToString(verifierStats.totalBytesVerified),
                 verifierStats.errors);
}

void SwDecoder::printFinalStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("**************");
    fmt::println("DeciChain Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Sent:", convertBytesToString(totalBytesDecimated));
    fmt::println("{:30} {}", "Rcv:", convertBytesToString(verifierStats.totalBytesVerified));
    fmt::println("{:30} {}", "Errors:", verifierStats.errors);
    fmt::println("");
}

size_t SwDecoder::getOutputQueueSize() const
{
    return packetOutQueue.getSize();
}

size_t SwDecoder::getInputQueueSize() const
{
    return packetInQueue.getSize();
}

size_t SwDecoder::getOutputQueueMaxSize() const
{
    return packetOutQueue.getMaxSize();
}

size_t SwDecoder::getInputQueueMaxSize() const
{
    return packetInQueue.getMaxSize();
}

bool SwDecoder::inputQueueIsEmpty() const
{
    return packetInQueue.isEmpty();
}

bool SwDecoder::outputQueueIsEmpty() const
{
    return packetOutQueue.isEmpty();
}
