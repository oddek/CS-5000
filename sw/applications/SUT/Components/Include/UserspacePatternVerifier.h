
#ifndef PATTERN_VERIFIER_H
#define PATTERN_VERIFIER_H

#include "Packet.h"
#include "PatternVerifier.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ranges>

class UserspacePatternVerifier : public PatternVerifier
{
  public:
    UserspacePatternVerifier(size_t interpolationLevel_ = 1U);
    void update() override;
    void reset() override;
    void verify(const uint8_t* buffer, size_t nBytes) override;
    void verify(const Packet<>& packet) override;

  private:
    uint8_t prevValCounter{0U};
    uint8_t expected{0x01U};
    size_t interpolationLevel{1};

    static constexpr auto minPatternValue = 1U;
    static constexpr auto maxPatternValue = 250U;
};

#endif // PATTERN_VERIFIER_H
