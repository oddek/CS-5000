
#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include "PatternGenerator.h"

class UserspacePatternGenerator : public PatternGenerator
{
  public:
    UserspacePatternGenerator();
    void reset() override;
    void generateAndEnqueueMessage(size_t messageSize) override;

  private:
    Packet<> packet;
    void generateSinglePacket(size_t nBytes, Packet<>::MetaData metaData, size_t totalMessageSize);

    uint8_t currentValue{minPatternValue};
    uint8_t sameValueCounter{sizeof(uint64_t)};
    static constexpr auto minPatternValue = 1U;
    static constexpr auto maxPatternValue = 250U;
};

#endif // PATTERN_GENERATOR_H
