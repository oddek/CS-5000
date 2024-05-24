
#include "Utils.h"

constexpr size_t stackMemSize = 16 * 4096;
constexpr size_t staticMemSize = 16*4096;
static uint8_t staticMemory[staticMemSize];

enum class Mode
{
    Static,
    Stack,
    Heap
};

bool lock{false};
bool prefault{false};
Mode mode{Mode::Static};

pthread_attr_t attr{0};

void touchAllPages(uint8_t* memory, size_t size)
{
    fmt::println("Touching all pages ...");
    const auto usage = getRusage();
    const auto startTime = getTimespecNow();
    for (size_t i = 0U; i < size; i += sysconf(_SC_PAGESIZE))
    {
        memory[i] = 0xABU;
    }
    printTimeElapsedNs(startTime);
    printNewFaults(usage);

    fmt::print("\n\n");
}

std::tuple<size_t, size_t> accessIndex(uint8_t* memory, size_t index)
{
    const auto usage = getRusage();
    const auto startTime = getTimespecNow();
    memory[index] = 0xABU;
    const auto elapsed = getTimeElapsedNs(startTime);
    const auto newFaults = getNewSoftFaults(usage);

    return {elapsed, newFaults};
}

std::tuple<size_t, size_t> accessPage(uint8_t* memory, size_t page)
{
    fmt::println("Access Page {} ...", page);
    const auto usage = getRusage();
    const auto startTime = getTimespecNow();
    memory[page * _SC_PAGESIZE] = 5;
    const auto elapsed = getTimeElapsedNs(startTime);
    const auto newFaults = getNewSoftFaults(usage);
    fmt::println("Elapsed: {}, newfaults {}", elapsed, newFaults);

    return {elapsed, newFaults};
}


void* thread_func(void* arg)
{
    uint8_t stackMemory[stackMemSize];
    uint8_t* memory{nullptr};
    size_t memSize = staticMemSize;

    size_t highestSingleFaultCount{0};
    size_t highestSingleFaultLatency{0};
    double_t averageSingleFaultLatency{0};
    size_t samples{0};

    switch(mode)
    {
    case Mode::Static:
        memory = staticMemory;
        break;
    case Mode::Stack:
        fmt::println("Stack enabled");
        memory = stackMemory;
        memSize = stackMemSize;
        break;
    case Mode::Heap:
        fmt::println("Heap enabled");
        memory = static_cast<uint8_t*>(malloc(staticMemSize));
        if( memory == nullptr)
        {
            throw std::runtime_error("Could not malloc");
        }
    }

    if (lock)
    {
        fmt::println("Lock enabled");
        mlockall(MCL_CURRENT | MCL_FUTURE);
    }

    if (prefault)
    {
        fmt::println("Prefault enabled");
        touchAllPages(memory, memSize);
    }

    printPageFaults();

    fmt::println("**********************");
    fmt::println("STARTING");
    fmt::println("**********************");


//    auto count = 0;
//    while(true)
//    {
        const auto initialUsage = getRusage();
        const auto startTime = getTimespecNow();
        for (size_t i = 0U; i < memSize; i += sysconf(_SC_PAGESIZE))
        {
//            fmt::println("I: {}", i);
            const auto [elapsed, faults] = accessIndex(memory, i);

            averageSingleFaultLatency += elapsed;
            samples++;

            if (elapsed > highestSingleFaultLatency)
            {
                highestSingleFaultLatency = elapsed;
            }

            if (faults > highestSingleFaultCount)
            {
                highestSingleFaultCount = faults;
            }
        }

        averageSingleFaultLatency /= static_cast<double_t>(samples);

        //    printPageFaults();
        fmt::println("Total New Faults:");
        printNewFaults(initialUsage);
        printTimeElapsedUs(startTime);
        fmt::println("Highest Single Fault Count: {}", highestSingleFaultCount);
        fmt::println("Highest Single Fault Latency: {} ns", highestSingleFaultLatency);
        fmt::println("Average Single Fault Latency: {} ns", averageSingleFaultLatency);

//        fmt::println("sleep for 10");
//        sleep(10);
//
//        count++;
//
//        if( count == 2)
//        {
//            break;
//        }
//    }

    return nullptr;
}


int main(int argc, char* argv[])
{
    for (size_t i = 1; i < static_cast<size_t>(argc); i++)
    {
        std::string arg = argv[i];
        if (arg == "memlock")
        {
            lock = true;
        }
        else if (arg == "memlock-main")
        {
            fmt::println("Lock enabled in main");
            mlockall(MCL_CURRENT | MCL_FUTURE);
        }
        else if (arg == "stack")
        {
            mode = Mode::Stack;
        }
        else if( arg == "heap")
        {
            mode = Mode::Heap;
        }
        else if( arg == "prefault")
        {
            prefault = true;
        }
    }

    if (pthread_attr_init(&attr) != 0)
    {
        throw std::runtime_error("Failed to init pthread attributes");
    }

    if (pthread_attr_setstacksize(&attr, 500_kiB) != 0)
    {
        throw std::runtime_error("Failed to set pthread stack size");
    }

    pthread_t thread;

    if (pthread_create(&thread, &attr, thread_func, nullptr) != 0)
    {
        throw std::runtime_error("Failed to create Thread");
    }

    pthread_join(thread, nullptr);


    return 0;
}