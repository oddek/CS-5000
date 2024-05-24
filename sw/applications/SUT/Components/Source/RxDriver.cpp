
#include "RxDriver.h"
#include "DataMover.h"

RxDriver::RxDriver(RingBuffer<RingBufferType::RX>& ringBuffer_, bool verifyOnTheFly_, uint32_t busyWaitUs_,
               bool readUntilEmpty_)
    : ringBuffer(ringBuffer_), verifyOnTheFly(verifyOnTheFly_), busyWaitUs(busyWaitUs_), readUntilEmpty(readUntilEmpty_)
{
    if (!verifyOnTheFly_)
    {
//        patternVerifier.start();
    }
}

bool RxDriver::update()
{
    ringBuffer.updateMinMaxRingBufferSize();

    if( readUntilEmpty)
    {
        // For share mem 512:
//        while( true)
//        {
//            if (const auto packetRead = getNewPacket(); packetRead == true)
//            {
//                patternVerifier.verify(latestPacket.getPayload().data(), latestPacket.getPayload().size());
//
//                if (busyWaitUs)
//                {
//                    const auto startTime = getTimespecNow();
//                    while (true)
//                    {
//                        const auto now = getTimespecNow();
//                        if (timeSpecDiffUs(now, startTime) > busyWaitUs)
//                        {
//                            break;
//                        }
//                    }
//                }
//
//            }
//            else
//            {
//                return false;
//            }
//        }
        // for full test:
        size_t packetsRead = 0U;
        while( true)
        {
            if(packetsRead++ > 1U)
            {
                return false;
            }

            if (const auto packetRead = getNewPacket(); packetRead == true)
            {
                patternVerifier.put(latestPacket);
                patternVerifier.update();
                outQueue.put(latestPacket);

                if (busyWaitUs)
                {
                    busyWaitMicros(busyWaitUs);
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        const auto packetRead = getNewPacket();
        if (packetRead)
        {
            if( verifyOnTheFly)
            {
                patternVerifier.verify(latestPacket.getPayload().data(), latestPacket.getPayload().size());
            }
            else
            {
                patternVerifier.put(latestPacket);
                patternVerifier.update();
                outQueue.put(latestPacket);
            }

            if (busyWaitUs)
            {
                busyWaitMicros(busyWaitUs);
            }
            return true;
        }

        return false;
    }
}

bool RxDriver::getNewPacket()
{
    if (DataMover::rxIsEnabled())
    {
        if (ringBuffer.getNumberOfItems() > Packet<>::DEFAULT_SIZE)
        {
            latestPacket = ringBuffer.readPacket(RingBuffer<RingBufferType::RX>::ReadSetting::SafetyMargin);
            return true;
        }
    }
    else
    {
        if (ringBuffer.getActualNumberOfItems() > 0U)
        {
            latestPacket = ringBuffer.readPacket(RingBuffer<RingBufferType::RX>::ReadSetting::All);
            return true;
        }
    }

    return false;
}

bool RxDriver::packetAvailable()
{
    return !outQueue.isEmpty();
}

Packet<> RxDriver::get()
{
    return outQueue.get();
}

void RxDriver::printStats()
{
    const auto verifierStats = patternVerifier.getStats();

    fmt::println("AdRx "
                 "{:3} - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Buf: {:>11}  "
                 "Err: {:>11}",
                 (DataMover::rxIsEnabled() ? "On " : "Off"),
                 convertBytesToString(DataMover::rxWritesPerformed() * sizeof(uint64_t)),
                 convertBytesToString(verifierStats.bytesVerified),
                 convertBytesToString(ringBuffer.getActualNumberOfItems()), verifierStats.errors);
}

void RxDriver::printFinalStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("**************");
    fmt::println("RxPipeline Stats");
    fmt::println("**************");
    fmt::println("{:30} {}",
                 "Bytes transmitted:", convertBytesToString(DataMover::rxWritesPerformed() * sizeof(uint64_t)));
    //    fmt::println("{:30} {}", "Bytes transmitted alt:", convertBytesToString(calcBytesTransmittedAlternate()));
    //    fmt::println("{:30} {}", "Bytes received successfully:",
    //    convertBytesToString(patternVerifier.getBytesReceivedSuccessfully()));
    fmt::println("{:30} {}", "Bytes received successfully:", convertBytesToString(verifierStats.bytesVerified));
    fmt::println("{:30} {}", "Bytes left in buffer:", convertBytesToString(ringBuffer.getActualNumberOfItems()));
    //    fmt::println("{:30} {}", "Errors:", patternVerifier.getErrors());
    fmt::println("{:30} {}", "Errors:", verifierStats.errors);
    fmt::println("{:30} {}\n", "Max Buffer Size:", convertBytesToString(ringBuffer.getMinMaxRingBufferSize()));
}
size_t RxDriver::getQueueSize() const
{
    return outQueue.getSize();
}

size_t RxDriver::getQueueMaxSize() const
{
    return outQueue.getMaxSize();
}
bool RxDriver::queueIsEmpty() const
{
    return outQueue.isEmpty();
}
