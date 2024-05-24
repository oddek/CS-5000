#include "PatternVerifier.h"

static void* verifierTxWorker(void* arg)
{
    auto* verifier = static_cast<PatternVerifier*>(arg);
    while(true)
    {
        verifier->update();
    }
}

void PatternVerifier::start()
{
    if (pthread_create(&thread, nullptr, verifierTxWorker, this) != 0)
    {
        throw std::runtime_error("PatternVerifier: Failed to create Thread");
    }

    if (pthread_setname_np(thread, "Verifier") != 0)
    {
        throw std::runtime_error("CyclicTask: Failed to set thread name");
    }
}

void PatternVerifier::put(const Packet<>& packet)
{
    packetQueue.put(packet);
}
PatternVerifier::Stats PatternVerifier::getStats()
{
    return stats;
}
size_t PatternVerifier::getQueueSize() const
{
    return packetQueue.getSize();
}

void PatternVerifier::printFinalStats()
{
    const auto verifierStats = getStats();
    fmt::println("**************");
    fmt::println("Verifier Stats");
    fmt::println("**************");
    fmt::println("{:30} {}", "Verified:", convertBytesToString(verifierStats.totalBytesVerified));
    fmt::println("{:30} {}", "Errors:", verifierStats.errors);
    fmt::println("");
}

void PatternVerifier::printStats()
{
    const auto verifierStats = getStats();

    fmt::println("Verifier - "
                 "                   "
                 "Cur: {:>11}  "
                 "Tot: {:>11}  "
                 "Err: {:>11}",
                 convertBytesToString(verifierStats.bytesVerified),
                 convertBytesToString(verifierStats.totalBytesVerified),
                 verifierStats.errors);
}

size_t PatternVerifier::getQueueMaxSize() const
{
    return packetQueue.getMaxSize();
}
void PatternVerifier::updateBitrate()
{
    const auto verifierStats = getStats();
    bitrateCalc.update(verifierStats.totalBytesVerified);
}
std::string PatternVerifier::getFormattedBitrate()
{
    return bitrateCalc.getFormattedBitrate();
}
bool PatternVerifier::queueIsEmpty() const
{
    return packetQueue.isEmpty();
}
std::string PatternVerifier::getFormattedAverageBitrate()
{
    return bitrateCalc.getFormattedAverageBitrate();
}
