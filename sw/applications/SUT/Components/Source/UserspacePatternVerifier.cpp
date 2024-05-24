
#include "UserspacePatternVerifier.h"

UserspacePatternVerifier::UserspacePatternVerifier(size_t interpolationLevel_): interpolationLevel(interpolationLevel_)
{
}

void UserspacePatternVerifier::update()
{
    if( !packetQueue.isEmpty())
    {
        const auto packet = packetQueue.get();
        verify(packet.getPayload().data(), packet.getSize());
    }
}

void UserspacePatternVerifier::verify(const uint8_t* buffer, size_t nBytes)
{
    for (size_t i = 0U; i < nBytes; i++)
    {
        if (expected != buffer[i])
        {
            stats.errors++;
            expected = buffer[i];
        }

        else
        {
            stats.bytesVerified++;
            stats.totalBytesVerified++;
            prevValCounter++;

            if (prevValCounter == interpolationLevel)
            {
                prevValCounter = 0U;
                expected = (expected == maxPatternValue) ? minPatternValue : expected + 1;
            }
        }
    }
}

[[maybe_unused]] void UserspacePatternVerifier::reset()
{
    prevValCounter = 0U;
    expected = minPatternValue;
    stats.bytesVerified = 0U;
    stats.errors = 0U;
}
void UserspacePatternVerifier::verify(const Packet<>& packet)
{
    verify(packet.getPayload().data(), packet.getSize());
}
