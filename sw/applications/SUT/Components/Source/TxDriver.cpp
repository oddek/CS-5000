
#include "TxDriver.h"
#include "DataMover.h"

TxDriver::TxDriver(RingBuffer<RingBufferType::TX>& ringBuffer_) : ringBuffer(ringBuffer_)
{
}

bool TxDriver::update()
{
    bool returnValue = false;

    if(DataMover::isTxUnderrunFlagSet())
    {
        DataMover::txClearUnderrunFlag();
        underruns++;
    }



    switch (currentState)
    {
    case State::Idle: {
        while (!inQueue.isEmpty())
        {
            const auto packet = inQueue.get();
            ringBuffer.write(packet);
            DataMover::txSetStopIndex(ringBuffer.getHead());
            currentBytesTransmitted += packet.getSize();

            const bool startDma = ringBuffer.getActualNumberOfItems() >= minSizeBeforeDmaStart;

            if(startDma)
            {
                currentState = State::Active;
                DataMover::txEnable();
                break;
            }
        }

        break;
    }

    case State::Active: {
        if (!DataMover::txIsEnabled())
        {
            currentState = State::Done;
            break;
        }

        ringBuffer.updateMinMaxRingBufferSize();

        while (!inQueue.isEmpty())
        {
            if (ringBuffer.getFreeBytes() >= Packet<>::DEFAULT_SIZE)
            {
                const auto packet = inQueue.get();
                currentBytesTransmitted += packet.getSize();
                ringBuffer.write(packet);
                DataMover::txSetStopIndex(ringBuffer.getHead());

//                if (packet.isLastPacket())
//                {
//                }
            }
            else
            {
                break;
            }
        }

        break;
    }

    case State::Done: {
        if (!DataMover::txIsEnabled())
        {
            currentState = State::Idle;
        }
        break;
    }
    }

    return returnValue;
}

void TxDriver::put(const Packet<>& packet)
{
    inQueue.put(packet);
}

void TxDriver::printFinalStats() const
{
    fmt::println("**************");
    fmt::println("TxDriver Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Bytes transmitted:", convertBytesToString(currentBytesTransmitted));
    fmt::println("{:30} {}", "Bytes received successfully:", convertBytesToString(DataMover::txReadsPerformed() * sizeof(uint64_t)));
    fmt::println("{:30} {}", "Bytes left in buffer:", convertBytesToString(ringBuffer.getActualNumberOfItems()));
    fmt::println("{:30} {}", "Errors:", DataMover::getTxReadErrors());
    fmt::println("{:30} {}", "Min Buffer Size:", convertBytesToString(ringBuffer.getMinMaxRingBufferSize()));
    fmt::println("{:30} {}, Underruns: {}\n", "Underrun Flag:", DataMover::isTxUnderrunFlagSet(), underruns);
}

void TxDriver::printStats()
{
//    const auto verifierStats = patternVerifier.getStats();
//    fmt::println("DaTx "
//                 "Ver: {:>11}  "
//                 "Err: {:>11} ",
//                 convertBytesToString(verifierStats.bytesVerified),
//                 verifierStats.errors);
//
//    if(currentBytesTransmitted % 8 != 0U)
//    {
//        throw std::runtime_error("Bytes transmitted not divisible by 8");
//    }
    fmt::println("DaTx "
                 "{:3} - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Buf: {:>11}  "
                 "Err: {:>11} ",
                 stateStrings.at(static_cast<uint8_t>(currentState)), convertBytesToString(currentBytesTransmitted),
                 convertBytesToString(DataMover::txReadsPerformed() * sizeof(uint64_t)),
                 convertBytesToString(ringBuffer.getActualNumberOfItems()), DataMover::getTxReadErrors());
    //        0U, DataMover::getTxReadErrors());
    fmt::println("Underrun Flag: {}, Underruns: {:>11}", DataMover::isTxUnderrunFlagSet(), underruns);
}
size_t TxDriver::getQueueSize() const
{
    return inQueue.getSize();
}

void TxDriver::setLastFlag()
{
    DataMover::txSetStopFlag();
    currentState = State::Done;

//    if( !ringBuffer.isEmpty() && !DataMover::txIsEnabled())
//    {
//        currentState = State::Done;
//        DataMover::txEnable();
//    }
}
bool TxDriver::queueIsEmpty() const
{
    return inQueue.isEmpty();
}

size_t TxDriver::getQueueMaxSize() const
{
    return inQueue.getMaxSize();
}
