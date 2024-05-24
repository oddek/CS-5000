
#ifndef CONTINUOUS_BUFFER_H
#define CONTINUOUS_BUFFER_H

#include <atomic>
#include <cstddef>
#include <cstdint>

class ContinuousBuffer
{
  public:
    enum class Cached
    {
        False [[maybe_unused]],
        True
    };

    ContinuousBuffer(size_t size_, Cached cacheSetting);
    ContinuousBuffer(const ContinuousBuffer&) = delete;
    ContinuousBuffer(ContinuousBuffer&&) = delete;
    ContinuousBuffer& operator=(const ContinuousBuffer&) = delete;
    ContinuousBuffer& operator=(ContinuousBuffer&&) = delete;
    ~ContinuousBuffer();

    [[nodiscard]] size_t getSize() const;
    [[nodiscard]] uintptr_t getPhysicalAddress() const;
    volatile uint8_t* getPointer();
    void cleanup();

  private:
    uintptr_t physicalAddress{0};
    void* bufferPtr = nullptr;
    const size_t size{0};
    static std::atomic<size_t> referenceCounter;

    static constexpr auto ACP_MASK = 0x080000000U;

};

#endif
