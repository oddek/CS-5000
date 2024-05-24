
#include "DurationTimer.h"
#include "Utils.h"

template <Resolution resolution>
DurationTimer<resolution>::DurationTimer(std::string name_): name(name_)
{
}

template <Resolution resolution>
void DurationTimer<resolution>::start()
{
    startTime = getTimespecNow();
}

template <Resolution resolution>
void DurationTimer<resolution>::stop()
{
    if constexpr (resolution == Resolution::ms)
    {
        current = getTimeElapsedMs(startTime);
    }
    else if constexpr (resolution == Resolution::us)
    {
        current = getTimeElapsedUs(startTime);
    }
    else if constexpr (resolution == Resolution::ns)
    {
        current = getTimeElapsedNs(startTime);
    }
    else
    {
        throw std::runtime_error("No valid resolution for DurationTimer");
    }

    if( current > max) {max = current;}
    if( current < min) {min = current;}

    average += (static_cast<double_t>(current) - average) / (samples + 1);
    samples++;
}

template <Resolution resolution>
void DurationTimer<resolution>::printStats()
{
    if constexpr (resolution == Resolution::ms)
    {
        fmt::println("{:<8} - Curr: {:8} ms  "
                     "Min: {:8} ms  "
                     "Max: {:8} ms  "
                     "Avg: {:8.0f} ms",
                     name, current, min, max,
                     average);
    }
    else if constexpr (resolution == Resolution::us)
    {
        fmt::println("{:<8} - Curr: {:8} us  "
                     "Min: {:8} us  "
                     "Max: {:8} us  "
                     "Avg: {:8.0f} us",
                     name, current, min, max,
                     average);
    }
    else if constexpr (resolution == Resolution::ns)
    {
        fmt::println("{:<8} - Curr: {:8} ns  "
                     "Min: {:8} ns  "
                     "Max: {:8} ns  "
                     "Avg: {:8.0f} ns",
                     name, current, min, max,
                     average);
    }
    else
    {
        throw std::runtime_error("No valid resolution for DurationTimer");
    }
}

template <Resolution resolution>
void DurationTimer<resolution>::printFinalStats()
{
    fmt::println("Duration: {}:", name);
    fmt::println("{:>6} {} us", "Min:", min);
    fmt::println("{:>6} {} us", "Max:", max);
    fmt::println("{:>6} {:.0f} us", "Avg:", average);
}

template class DurationTimer<Resolution::ms>;
template class DurationTimer<Resolution::us>;
template class DurationTimer<Resolution::ns>;
