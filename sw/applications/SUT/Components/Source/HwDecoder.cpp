
#include "HwDecoder.h"
#include "Utils.h"
#include <fmt/core.h>

void HwDecoder::decode(const Packet<>& packet)
{
    memcpy((reinterpret_cast<uint8_t*>(inputBuffer)) + inputBufferSize, packet.getPayload().begin(), packet.getSize());
    inputBufferSize += packet.getSize();

    currentInputBufferIndex = 0U;

    while (true)
    {
        readFromHardwareDecoder();

        if ((inputBufferSize - (currentInputBufferIndex * 4U)) < sizeof(uint32_t))
        {
            memmove(&inputBuffer[0], &inputBuffer[currentInputBufferIndex],
                    inputBufferSize - (currentInputBufferIndex * 4U));
            inputBufferSize -= (currentInputBufferIndex * 4U);
            break;
        }

        writeToHardwareDecoder();
    }
}

void HwDecoder::writeToHardwareDecoder()
{
    while (!hardwareDecoder.isFull())
    {
        if ((inputBufferSize - (currentInputBufferIndex * 4U)) < sizeof(uint32_t))
        {
            break;
        }

        hardwareDecoder.putWord(inputBuffer[currentInputBufferIndex]);
        currentInputBufferIndex++;
        writesPerformed++;
    }
}

void HwDecoder::readFromHardwareDecoder()
{
    uint32_t buffer[Packet<>::DEFAULT_SIZE / sizeof(uint32_t)] = {0};
    size_t wordsRead{0U};
    while (!hardwareDecoder.isEmpty())
    {
        buffer[wordsRead++] = hardwareDecoder.getWord();
        readsPerformed++;
    }

    if (wordsRead != 0U)
    {
        const Packet packet(reinterpret_cast<uint8_t*>(buffer), wordsRead * sizeof(uint32_t));
        patternVerifier.put(packet);
        patternVerifier.update();
        packetOutQueue.put(packet);
    }
}
void HwDecoder::put(const Packet<>& packet)
{
    packetInQueue.put(packet);
}

Packet<> HwDecoder::get()
{
    return packetOutQueue.get();
}
bool HwDecoder::packetAvailable()
{
    return !packetOutQueue.isEmpty();
}
void HwDecoder::update()
{
    while (!packetInQueue.isEmpty())
    {
        const auto packet = packetInQueue.get();
        decode(packet);
        patternVerifier.update();
    }
}

void HwDecoder::printStats()
{
    const auto verifierStats = patternVerifier.getStats();
    fmt::println("HwDecoder  - "
                 "Sent: {:>11}  "
                 "Rcv: {:>11}  "
                 "Fif: {:>11}  "
                 "Err: {:>11}",
                 convertBytesToString(writesPerformed * sizeof(uint32_t)),
                 convertBytesToString(verifierStats.bytesVerified), hardwareDecoder.getFifoState(),
                 verifierStats.errors);
}

void HwDecoder::printFinalStats()
{
        const auto verifierStats = patternVerifier.getStats();
        fmt::println("**************");
        fmt::println("HwDecoder Stats");
        fmt::println("**************");
        fmt::println("{:30} {}", "Bytes transmitted:", convertBytesToString(writesPerformed * sizeof(uint32_t)));
        fmt::println("{:30} {}",
                     "Bytes received successfully:", convertBytesToString(verifierStats.bytesVerified));
        fmt::println("{:30} {}", "Fifo:", hardwareDecoder.getFifoState());
        fmt::println("{:30} {}", "Errors:", verifierStats.errors);
        fmt::println("{:30} {}", "Writes:", writesPerformed * 4);
        fmt::println("{:30} {}", "Reads:", readsPerformed * 4);
        fmt::println("{:30} {}", "Success:", verifierStats.bytesVerified);
        fmt::println("");
}
size_t HwDecoder::getOutputQueueSize() const
{
    return packetOutQueue.getSize();
}
size_t HwDecoder::getInputQueueSize() const
{
    return packetInQueue.getSize();
}
size_t HwDecoder::getOutputQueueMaxSize() const
{
    return packetOutQueue.getMaxSize();
}
size_t HwDecoder::getInputQueueMaxSize() const
{
    return packetInQueue.getMaxSize();
}
bool HwDecoder::inputQueueIsEmpty() const
{
    return packetInQueue.isEmpty();
}
bool HwDecoder::outputQueueIsEmpty() const
{
    return packetOutQueue.isEmpty();
}
