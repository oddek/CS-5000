
#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <malloc.h>
#include <sys/mman.h>
#include <any>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <ctime>
#include <fmt/core.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <string_view>
#include <tuple>


static constexpr auto BITS_IN_BYTE = 8U;
static constexpr auto KILO = 1000U;
static constexpr auto KIBI = 1024UL;
static constexpr auto MEGA = KILO * KILO;
static constexpr auto MEBI = KIBI * KIBI;

static constexpr auto NANOS_PER_SECOND = 1000000000;
static constexpr auto NANOS_PER_MILLISECOND = 1000000U;
static constexpr auto NANOS_PER_MICROSECOND = 1000U;
static constexpr auto MICROS_PER_SECOND = 1000000U;
static constexpr auto MILLIS_PER_SECOND = 1000U;

constexpr std::size_t operator""_kiB(unsigned long long val)
{
    return KIBI * val;
}

constexpr std::size_t operator""_MiB(unsigned long long val)
{
    return static_cast<uint64_t>(KIBI) * static_cast<uint64_t>(KIBI) * val;
}

constexpr std::size_t operator""_kibps(unsigned long long val)
{
    return KIBI * val;
}

constexpr std::size_t operator""_Mibps(unsigned long long val)
{
    return MEBI * val;
}

constexpr std::size_t operator""_MHz(unsigned long long val)
{
    return MEGA * val;
}

// From https://xuhuisun.com/post/c++-weekly-2-constexpr-map/
template <typename Key, typename Value, std::size_t Size> struct [[maybe_unused]] Map
{
    std::array<std::pair<Key, Value>, Size> data; // NOLINT(*-non-private-member-variables-in-classes)

    [[nodiscard]] constexpr Value at(const Key& key) const
    {
        const auto itr = std::find_if(begin(data), end(data), [&key](const auto& var) { return var.first == key; });
        if (itr != end(data))
        {
            return itr->second;
        }
        else
        {
            throw std::range_error("Not Found");
        }
    }
} __attribute__((aligned(128)));

//Timespec functions based on:
// https://github.com/jlelli/rt-tests/blob/master/src/cyclictest/cyclictest.c
constexpr int64_t timeSpecDiffNs(const timespec& minuend, const timespec& subtrahend)
{
    int64_t diff = MICROS_PER_SECOND * static_cast<int64_t>((minuend.tv_sec - subtrahend.tv_sec));
    diff += (minuend.tv_nsec - subtrahend.tv_nsec);
    return diff;
}

constexpr int64_t timeSpecDiffUs(const timespec& minuend, const timespec& subtrahend)
{
    int64_t diff = MICROS_PER_SECOND * static_cast<int64_t>((minuend.tv_sec - subtrahend.tv_sec));
    diff += ((minuend.tv_nsec - subtrahend.tv_nsec) / static_cast<int32_t>(NANOS_PER_MICROSECOND));
    return diff;
}

constexpr int64_t timeSpecDiffMs(const timespec& minuend, const timespec& subtrahend)
{
    int64_t diff = MILLIS_PER_SECOND * static_cast<int64_t>((minuend.tv_sec - subtrahend.tv_sec));
    diff += ((minuend.tv_nsec - subtrahend.tv_nsec) / static_cast<int32_t>(NANOS_PER_MILLISECOND));
    return diff;
}

static inline bool timeSpecGreater(const struct timespec& a, const struct timespec& b)
{
    return ((a.tv_sec > b.tv_sec) || (a.tv_sec == b.tv_sec && a.tv_nsec > b.tv_nsec));
}


inline constexpr std::pair<float, size_t> divideToHighestUnit(uint64_t value, const uint32_t unitValue,
                                                              const size_t numberOfUnits)
{
    int64_t div = 0;
    uint64_t rem = 0U;

    while (value >= unitValue && div < numberOfUnits)
    {
        rem = (value % unitValue);
        div++;
        value /= unitValue;
    }

    return {(static_cast<float>(value) + static_cast<float>(rem) / static_cast<float>(unitValue)), div};
}

inline std::string convertBytesToString(uint64_t bytes)
{
    constexpr std::array UNITS = {"B", "KiB", "MiB", "GiB", "TiB"};

    auto [bytesAsFloat, unitIndex] = divideToHighestUnit(bytes, KIBI, UNITS.size());

    return fmt::format("{:.2f} {}", bytesAsFloat, UNITS.at(unitIndex));
}

inline std::string convertBitrateToString(uint64_t bitrateRaw)
{
    static constexpr std::array UNITS = {"bps", "kibps", "Mibps"};
    auto [bitrateFloat, unitIndex] = divideToHighestUnit(bitrateRaw, KIBI, UNITS.size());

    return fmt::format("{:.2f} {}", bitrateFloat, UNITS.at(unitIndex));
}

inline std::string convertChronoToMinimalString(const std::chrono::nanoseconds& timestamp)
{
    auto nanoSeconds = timestamp;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(nanoSeconds);
    nanoSeconds -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(nanoSeconds);
    nanoSeconds -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nanoSeconds);
    nanoSeconds -= seconds;
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(nanoSeconds);
    nanoSeconds -= millis;
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(nanoSeconds);
    nanoSeconds -= micros;

    if (nanoSeconds.count() != 0)
    {
        return fmt::format("{} ns", timestamp.count());
    }
    else if (micros.count() != 0)
    {
        return fmt::format("{} us", std::chrono::duration_cast<std::chrono::microseconds>(timestamp).count());
    }
    else if (millis.count() != 0)
    {
        return fmt::format("{} ms", std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
    }
    else if (seconds.count() != 0)
    {
        return fmt::format("{} s", std::chrono::duration_cast<std::chrono::seconds>(timestamp).count());
    }
    else if (minutes.count() != 0)
    {
        return fmt::format("{} m", std::chrono::duration_cast<std::chrono::minutes>(timestamp).count());
    }
    else if (hours.count() != 0)
    {
        return fmt::format("{} h", std::chrono::duration_cast<std::chrono::hours>(timestamp).count());
    }
    else
    {
        return fmt::format("0 nanoSeconds");
    }
}

inline bool runTimeExpired(std::chrono::steady_clock::time_point startTime, std::chrono::nanoseconds maxRuntime)
{
    auto now = std::chrono::steady_clock::now();
    auto timeElapsed = now - startTime;

    return timeElapsed > maxRuntime;
}

inline void printRunTime(const std::chrono::steady_clock::time_point startTime)
{
    const auto now = std::chrono::steady_clock::now();
    const auto time_elapsed = now - startTime;

    auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed);

    auto hours = std::chrono::duration_cast<std::chrono::hours>(milliSeconds);
    milliSeconds -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(milliSeconds);
    milliSeconds -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(milliSeconds);

    fmt::println("{:02}:{:02}:{:02}", hours.count(), minutes.count(), seconds.count());
}

inline std::string convertTimespecToString(const timespec& time)
{
    return convertChronoToMinimalString(std::chrono::seconds(time.tv_sec) + std::chrono::nanoseconds(time.tv_nsec));
}

constexpr void timespecNormalize(timespec& time)
{
    while (time.tv_nsec >= NANOS_PER_SECOND)
    {
        time.tv_sec++;
        time.tv_nsec -= NANOS_PER_SECOND;
    }
}

constexpr void timeSpecDecrement(timespec& time, const timespec& decrement)
{
    time.tv_nsec -= decrement.tv_nsec;
    time.tv_sec -= decrement.tv_sec;

    timespecNormalize(time);
}

constexpr void timeSpecIncrement(timespec& time, const timespec& increment)
{
    time.tv_nsec += increment.tv_nsec;
    time.tv_sec += increment.tv_sec;

    timespecNormalize(time);
}

constexpr timespec durationToTimespec(std::chrono::nanoseconds nanoSeconds)
{
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nanoSeconds);
    nanoSeconds -= seconds;

    return {static_cast<time_t>(seconds.count()), static_cast<int32_t>(nanoSeconds.count())};
}

inline std::tuple<uint32_t, std::string_view> splitUnitString(const std::string_view& str)
{
    auto getValue = [&]() {
        uint32_t val{};
        auto result = std::from_chars(str.data(), str.data() + str.size(), val);

        if (result.ptr == str.data())
        {
            throw std::runtime_error("Could not parse value from unit string");
        }

        return val;
    };

    auto getUnit = [&]() {
        constexpr std::string_view delimiter = "_";
        std::string_view unit = str.substr(str.find(delimiter) + 1);

        if (unit.data() == str.data())
        {
            unit = "";
        }

        return unit;
    };

    return {getValue(), getUnit()};
}

inline uint32_t bitrateStringToBitrate(const std::string_view& bitrateString)
{
    auto [val, unit] = splitUnitString(bitrateString);

    fmt::println("Unit: {}", unit);

    static constexpr auto map = Map<std::string_view, uint32_t, 9>{{{{"kibps", KIBI},
                                                                     {"kbps", KILO},
                                                                     {"Mibps", MEBI},
                                                                     {"Mpbs", MEGA},
                                                                     {"kiBps", KIBI * BITS_IN_BYTE},
                                                                     {"kBps", KILO * BITS_IN_BYTE},
                                                                     {"MiBps", MEBI * BITS_IN_BYTE},
                                                                     {"MBps", MEGA * BITS_IN_BYTE},
                                                                     {"", 1U}}}};

    return val * map.at(unit);
}

inline std::chrono::nanoseconds timeStringToChrono(const std::string_view& timeString)
{
    auto [timeValue, unit] = splitUnitString(timeString);

    static auto map = Map<std::string_view, std::function<std::chrono::nanoseconds(uint32_t)>, 6>{
        {{{"ns", [](uint32_t val) { return std::chrono::nanoseconds(val); }},
          {"us", [](uint32_t val) { return std::chrono::microseconds(val); }},
          {"ms", [](uint32_t val) { return std::chrono::milliseconds(val); }},
          {"s", [](uint32_t val) { return std::chrono::seconds(val); }},
          {"m", [](uint32_t val) { return std::chrono::minutes(val); }},
          {"h", [](uint32_t val) { return std::chrono::hours(val); }}}}};

    return map.at(unit)(timeValue);
}

inline struct timespec timeStringToTimespec(const std::string_view& timeString)
{
    return durationToTimespec(timeStringToChrono(timeString));
}

[[maybe_unused]] inline void printTimespec(const struct timespec& time)
{
    fmt::println("Ts: {} s, {} ns", time.tv_sec, time.tv_nsec);
}

inline timespec getTimespecNow()
{
    timespec now{};
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now;
}

inline size_t getTimeElapsedUs(const timespec startTime)
{
    const auto now = getTimespecNow();
    return timeSpecDiffUs(now, startTime);
}

inline size_t getTimeElapsedMs(const timespec startTime)
{
    const auto now = getTimespecNow();
    return timeSpecDiffMs(now, startTime);
}


inline size_t getTimeElapsedNs(timespec startTime)
{
    const auto now = getTimespecNow();
    return timeSpecDiffUs(now, startTime);
}

inline void printTimeElapsedNs(timespec startTime)
{
    fmt::println("Took {} ns", getTimeElapsedNs(startTime));
}

inline void printTimeElapsedUs(timespec startTime)
{
    fmt::println("Took {} us", getTimeElapsedUs(startTime));
}

inline void busyWaitMicros(uint32_t us)
{
    const auto startTime = getTimespecNow();
    while (true)
    {
        const auto now = getTimespecNow();
        if (timeSpecDiffUs(now, startTime) > static_cast<int64_t>(us))
        {
            break;
        }
    }
}

inline static void doMemoryLock()
{
    if (mlockall(MCL_CURRENT | MCL_FUTURE))
    {
        throw std::runtime_error("Could not lock");
    }

    mallopt(M_TRIM_THRESHOLD, -1);
    mallopt(M_MMAP_MAX, 0);
}

inline static void prefaultHeap(size_t size)
{
    uint8_t* buffer = static_cast<uint8_t*>(malloc(size));

    for(size_t i = 0; i < size; i += sysconf(_SC_PAGESIZE))
    {
        buffer[i] = 0;
    }
}

inline rusage getRusage()
{
    struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);
    return usage;
}

inline size_t getNewSoftFaults(rusage prevUsage)
{
    static struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);
    return (usage.ru_minflt - prevUsage.ru_minflt);
}

inline void printNewFaults(rusage prevUsage)
{
    static struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);
    fmt::println("Soft Page Faults: {}\nHard Page Faults: {}", (usage.ru_minflt - prevUsage.ru_minflt),
                 (usage.ru_majflt - prevUsage.ru_majflt));
}

inline void printPageFaults()
{
    static struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);
    fmt::println("Soft Page Faults: {}\nHard Page Faults: {}", usage.ru_minflt, usage.ru_majflt);
}

inline void printPreemption()
{
    struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);

    fmt::println("Voluntary preempt {}, Preempted {}", usage.ru_nvcsw, usage.ru_nivcsw);
}


static constexpr size_t defaultStackSize = 0x10000U;

inline static void setStackSize(size_t size)
{
    static pthread_attr_t attr{0};
    if (pthread_attr_init(&attr) != 0)
    {
        throw std::runtime_error("Utils: Failed to init pthread attributes");
    }

    if (pthread_attr_setstacksize(&attr, size) != 0)
    {
        throw std::runtime_error("Utils: Failed to set pthread stack size");
    }
}

inline pid_t get_task_id(void)
{
    pid_t tid = syscall(SYS_gettid);
    return tid;
}

inline pid_t get_task_id_from_comm(std::string_view comm)
{
    const std::string s = fmt::format("ps aux -o pid,comm | grep {}", comm);
    FILE* cmd_pipe = popen(s.c_str(), "r");

    char buf[512];
    fgets(buf, 512, cmd_pipe);
    fmt::println("Req: {}", s);
    fmt::println("Buff: {}", buf);
    pid_t pid = strtoul(buf, NULL, 10);

    pclose(cmd_pipe);

    return pid;
}

#endif
