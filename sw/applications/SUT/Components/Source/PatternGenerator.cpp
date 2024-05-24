#include "PatternGenerator.h"


Packet<> PatternGenerator::get()
{
    auto packet = packetQueue.get();
    return packet;
}

bool PatternGenerator::packetAvailable() const
{
    return !packetQueue.isEmpty();
}

size_t PatternGenerator::getQueueSize() const
{
    return packetQueue.getSize();
}

void PatternGenerator::printFinalStats() const
{
    fmt::println("**************");
    fmt::println("Generator Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Generated:", convertBytesToString(totalBytesGenerated));
    fmt::println("{:30} {}", "Packets:", packetsGenerated);
    fmt::println("{:30} {}", "Messages:", messagesGenerated);
    fmt::println("");
}

void PatternGenerator::printStats() const
{
    fmt::println("Generator- "
                 "Gen:  {:>11}  "
                 "Tot: {:>11}  "
                 "Pac: {:>11}  "
                 "Msg: {:>11}",
                 convertBytesToString(bytesGenerated),
                 convertBytesToString(totalBytesGenerated),
                 packetsGenerated,
                 messagesGenerated
                 );
}

bool PatternGenerator::queueIsEmpty() const
{
    return packetQueue.isEmpty();
}

size_t PatternGenerator::getQueueMaxSize() const
{
    return packetQueue.getMaxSize();
}

void PatternGenerator::updateBitrate()
{
    bitrateCalc.update(totalBytesGenerated);
}

std::string PatternGenerator::getFormattedBitrate()
{
    return bitrateCalc.getFormattedBitrate();
}
std::string PatternGenerator::getFormattedAverageBitrate()
{
    return bitrateCalc.getFormattedAverageBitrate();
}
