
#include "BitrateCalc.h"
void BitrateCalc::update(size_t bytesTransferred)
{
    if(getTimeElapsedMs(lastbitrateTime) > bitrateUpdateIntervalMs)
    {
        const auto newBitsGenerated = (bytesTransferred - lastBytesGenerated) * 8U;
        const auto now = getTimespecNow();
        const auto diff = timeSpecDiffMs(now, lastbitrateTime);
        double_t diff_seconds = (static_cast<double_t>(diff) / 1000.0);

        currentBitrate = static_cast<double_t>(newBitsGenerated) / diff_seconds;

        lastBytesGenerated = bytesTransferred;
        lastbitrateTime = getTimespecNow();

        {
            static size_t samplesToSkip = 0U;
            if( samplesToSkip > samplesToSkipInAverage)
            {
                averageBitrate += (static_cast<double_t>(currentBitrate) - averageBitrate) / (nSamples + 1);
                nSamples++;
            }
            else
            {
                samplesToSkip++;
            }
        }
    }
}

std::string BitrateCalc::getFormattedBitrate()
{
    return convertBitrateToString(currentBitrate);
}
std::string BitrateCalc::getFormattedAverageBitrate()
{
    return convertBitrateToString(averageBitrate);
}
