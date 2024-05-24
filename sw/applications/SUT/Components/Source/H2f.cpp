
#include "H2f.h"
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

int H2f::devMemFd{0};
void* H2f::h2fVirtualBase{nullptr};

H2f::H2f()
{
    devMemFd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (devMemFd == -1)
    {
        throw std::runtime_error("H2f: Could not open \"/dev/mem\"");
    }

    h2fVirtualBase =
        mmap(nullptr, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, devMemFd, static_cast<int32_t>(HW_REGS_BASE));

    if (h2fVirtualBase == MAP_FAILED)
    {
        cleanup();
        throw std::runtime_error("H2f: mmap failed");
    }
}

H2f::~H2f()
{
    cleanup();
}

void H2f::cleanup()
{
    if (h2fVirtualBase != nullptr)
    {
        (void)munmap(h2fVirtualBase, HW_REGS_SPAN);
    }

    if (devMemFd != 0)
    {
        (void)close(devMemFd);
    }
}

uint32_t H2f::read(uint32_t address)
{
    auto regPtr = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<uintptr_t>(h2fVirtualBase) + address);
    const uint32_t value = *regPtr;
    return value;
}

void H2f::write(uint32_t address, uint32_t value)
{
    auto regPtr = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<uintptr_t>(h2fVirtualBase) + address);
    *regPtr = value;
}

void H2f::blinkUpdate()
{
    static bool on = false;
    static auto start = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 500)
    {
        if (on)
        {
            write(BLUE_LIGHT_ADDRESS, 0x01);
        }

        else
        {
            write(BLUE_LIGHT_ADDRESS, 0x00);
        }

        on = !on;
        start = now;
    }
}

void H2f::init()
{
    static H2f h2f;
}
