
#ifndef HARDWARE_CODEC
#define HARDWARE_CODEC

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>

enum class CodecType
{
    Decode,
    Encode
};

template <CodecType Type>
class HardwareCodec
{
  public:
    enum class Register
    {
        MagicNumber = 0U,
        ResetCounter,
        SoftResetCounter,
        WritesPerformedUpper,
        WritesPerformedLower,
        ReadsPerformedUpper,
        ReadsPerformedLower,
        IsFull,
        IsEmpty,
        ReadData,
        WriteData,
        SoftReset,
        CodecSetting,
        CodecFactor,
        RegisterLast [[maybe_unused]]
    };

    HardwareCodec();
    explicit HardwareCodec(uint8_t codecFactor_);
    uint64_t getWritesPerformed();
    uint64_t getReadsPerformed();
    bool isEmpty();
    bool isFull();
    void clear();

    std::string_view getFifoState();

    void putWord(uint32_t word);
    uint32_t getWord();
    static constexpr uint32_t CAPACITY = 16U;

  private:
    uint32_t getBaseAddress();
    void softReset();
    void writeRegister(Register, uint32_t);
    void writeAndVerifyRegister(Register, uint32_t);
    void readAndAssertRegister(Register, uint32_t);
    uint32_t readRegister(Register);

    const uint8_t codecFactor{1U};

    static constexpr uint32_t SOFT_RESET_CODE = 0xFF00FF00U;
    static constexpr uint32_t EXP_MAGIC_NUMBER = 0x2224ABCDU;
    static constexpr uint32_t IS_EMPTY_CODE = 0x0F0F0F0FU;
    static constexpr uint32_t IS_FULL_CODE = 0xF0F0F0F0U;
    static constexpr uint32_t CODEC_SETTING_INTERPOLATE_CODE  = 0x0000FFFFU;
    static constexpr uint32_t CODEC_SETTING_DECIMATE_CODE = 0xFFFF0000U;
};

#endif