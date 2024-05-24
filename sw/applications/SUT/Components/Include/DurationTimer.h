
#ifndef SUT_DURATIONTIMER_H
#define SUT_DURATIONTIMER_H

#include <stdint.h>
#include <time.h>
#include <string>
#include <math.h>

enum class Resolution
{
    ms,
    us,
    ns
};

template <Resolution resolution = Resolution::us>
class DurationTimer
{
  public:
    explicit DurationTimer(std::string name_);
    void start();
    void stop();
    void printStats();
    void printFinalStats();

  private:
    const std::string name;
    uint32_t current{0};
    uint32_t min{UINT32_MAX};
    uint32_t max{0U};
    uint32_t samples{0};
    double_t average{0.0};

    timespec startTime{0};
};

#endif // SUT_DURATIONTIMER_H
