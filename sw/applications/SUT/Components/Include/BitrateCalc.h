
#ifndef SUT_BITRATECALC_H
#define SUT_BITRATECALC_H

#include "Utils.h"

class BitrateCalc
{
  public:
    void update(size_t bytesTransferred);
    std::string getFormattedBitrate();
    std::string getFormattedAverageBitrate();

  private:
    timespec lastbitrateTime{0};
    size_t lastBytesGenerated{0U};
    double_t currentBitrate{0.0};

    double_t averageBitrate{0.0};
    size_t nSamples{0U};

    static constexpr size_t bitrateUpdateIntervalMs{1000U};
    static constexpr size_t samplesToSkipInAverage{5U};
};

#endif // SUT_BITRATECALC_H
