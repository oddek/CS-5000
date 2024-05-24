
#include "UserspacePatternGenerator.h"


UserspacePatternGenerator::UserspacePatternGenerator()
{

}

void UserspacePatternGenerator::generateSinglePacket(size_t nBytes, Packet<>::MetaData metaData, size_t totalMessageSize)
{
    auto [packetBegin, packetEnd] = packet.getByteIterators();
    auto bytesLeftToGenerate = nBytes;

    while (bytesLeftToGenerate != 0U)
    {
        if (bytesLeftToGenerate >= sizeof(uint64_t))
        {
            std::fill(packetBegin, packetBegin + sameValueCounter, currentValue);

            bytesLeftToGenerate -= sameValueCounter;
            currentValue = (currentValue == maxPatternValue) ? minPatternValue : currentValue + 1U;
            std::advance(packetBegin, sameValueCounter);
            sameValueCounter = sizeof(uint64_t);
        }

        else
        {
            const size_t bytesToWrite = (bytesLeftToGenerate < sameValueCounter) ? bytesLeftToGenerate : sameValueCounter;
            std::fill(packetBegin, packetBegin + bytesToWrite, currentValue);

            sameValueCounter -= bytesToWrite;
            bytesLeftToGenerate -= bytesToWrite;
            std::advance(packetBegin, bytesToWrite);

            if (sameValueCounter == 0U)
            {
                sameValueCounter = sizeof(uint64_t);
                currentValue = (currentValue == maxPatternValue) ? minPatternValue : currentValue + 1U;
            }
        }
    }

    packet.setSize(nBytes);
    packet.setMetaData(metaData);
    packet.setTotalMessageSize(totalMessageSize);
}

void UserspacePatternGenerator::reset()
{
    currentValue = minPatternValue;
    sameValueCounter = sizeof(uint64_t);
    packetQueue.clear();
}

void UserspacePatternGenerator::generateAndEnqueueMessage(size_t totalSize)
{
    reset();
    size_t remainingBytes = totalSize;
    while( remainingBytes)
    {
        const size_t bytesToGenerate = (remainingBytes > Packet<>::DEFAULT_SIZE) ? Packet<>::DEFAULT_SIZE : remainingBytes;
        remainingBytes -= bytesToGenerate;

        Packet<>::MetaData metaData = Packet<>::MetaData::None;

        if( remainingBytes == totalSize)
        {
            metaData = Packet<>::MetaData::FirstPacket;
        }
        else if(remainingBytes == 0U)
        {
            metaData = Packet<>::MetaData::LastPacket;
        }

        generateSinglePacket(bytesToGenerate, metaData, totalSize);
        packetQueue.put(packet);
    }
}
