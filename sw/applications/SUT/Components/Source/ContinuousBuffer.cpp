
#include "ContinuousBuffer.h"
#include "cma_api.h"
#include <stdexcept>
#include "fmt/core.h"

std::atomic<size_t> ContinuousBuffer::referenceCounter = 0U;

ContinuousBuffer::ContinuousBuffer(size_t size_, ContinuousBuffer::Cached cacheSetting) : size(size_)
{
    referenceCounter++;
    if (referenceCounter == 1U)
    {
        if (cma_init() != 0)
        {
            throw std::runtime_error("Continuous Buffer: cma_init failed");
        }

    }

    if (cacheSetting == Cached::True)
    {
        bufferPtr = cma_alloc_cached(size);
    }
    else
    {
        bufferPtr = cma_alloc_noncached(size);
    }

    if (bufferPtr == nullptr)
    {
        cleanup();
        throw std::runtime_error("Continuous Buffer: Alloc Failed");
    }

    physicalAddress = cma_get_phy_addr(bufferPtr);

    if (physicalAddress == 0)
    {
        cleanup();
        throw std::runtime_error("Continuous Buffer: Get Physical Address Failed");
    }

    if (cacheSetting == Cached::True)
    {
        physicalAddress |= ACP_MASK;
    }

    auto* buf = static_cast<volatile uint32_t*>(bufferPtr);
    const auto existingValue = *buf;
    *buf = existingValue + 1U;

    if ((*buf) != (existingValue + 1U))
    {
        cleanup();
        throw std::runtime_error("Continuous Buffer: Can't write");
    }
}

ContinuousBuffer::~ContinuousBuffer()
{
    cleanup();
}

void ContinuousBuffer::cleanup()
{
    fmt::println("ContinousBuffer: Cleanup");
    if (bufferPtr != nullptr)
    {
        fmt::println("ContinousBuffer: CMA_FREE");
        (void)cma_free(bufferPtr);
        bufferPtr = nullptr;
    }

    if (referenceCounter > 0)
    {
        referenceCounter--;

        if (referenceCounter == 0)
        {
            fmt::println("ContinousBuffer: CMA_RELEASE");
            (void)cma_release();
        }
    }
}

uintptr_t ContinuousBuffer::getPhysicalAddress() const
{
    return physicalAddress;
}

uintptr_t ContinuousBuffer::getSize() const
{
    return size;
}

volatile uint8_t* ContinuousBuffer::getPointer()
{
    return static_cast<volatile uint8_t*>(bufferPtr);
}
