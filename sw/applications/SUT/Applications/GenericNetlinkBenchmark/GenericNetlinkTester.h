
#ifndef SUT_GENERICNETLINKTESTER_H
#define SUT_GENERICNETLINKTESTER_H

#include "PatternVerifier.h"
#include "PatternGenerator.h"
#include "DurationTimer.h"

class GenericNetlinkTester
{
  public:
    GenericNetlinkTester(PatternGenerator& patternGenerator_, PatternVerifier& patternVerifier_);
    bool update();
    void printStats();
    void printFinalStats();
  private:
    PatternGenerator& generator;
    PatternVerifier& verifier;

    size_t packetsRelayed{0U};
    DurationTimer<Resolution::us> askTimer{"Ask"};
    DurationTimer<Resolution::us> readTimer{"Read"};
    DurationTimer<Resolution::us> writeTimer{"Write"};
};

#endif // SUT_GENERICNETLINKTESTER_H
