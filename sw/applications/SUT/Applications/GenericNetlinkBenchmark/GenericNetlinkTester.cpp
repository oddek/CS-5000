
#include "GenericNetlinkTester.h"
#include <fmt/ranges.h>
#include <string_view>
#include <span>

GenericNetlinkTester::GenericNetlinkTester(PatternGenerator& patternGenerator_, PatternVerifier& patternVerifier_)
    : generator(patternGenerator_), verifier(patternVerifier_)
{
}

bool GenericNetlinkTester::update()
{
    askTimer.start();
    generator.generateAndEnqueueMessage(1500U);
    askTimer.stop();
    readTimer.start();
    generator.update();
    readTimer.stop();
    if( generator.packetAvailable())
    {
        const auto newPacket = generator.get();
        writeTimer.start();
        verifier.verify(newPacket);
        writeTimer.stop();

        packetsRelayed++;
    }
    else
    {
        throw std::runtime_error("No packet available from generator after asking");
    }

    return false;
}

void GenericNetlinkTester::printStats()
{
    fmt::println("Netlink - "
                 "Relayed:  {:>11}  ",
                 packetsRelayed
    );
    askTimer.printStats();
    readTimer.printStats();
    writeTimer.printStats();
}

void GenericNetlinkTester::printFinalStats()
{
    fmt::println("Netlink - "
                 "Relayed:  {:>11}  ",
                 packetsRelayed
    );

    askTimer.printFinalStats();
    readTimer.printFinalStats();
    writeTimer.printFinalStats();
}
