

#include "HardwareCodec.h"
#include "H2f.h"
#include "Utils.h"
#include <stdexcept>
#include <thread>
#include <unistd.h>

template <CodecType Type>
HardwareCodec<Type>::HardwareCodec() : HardwareCodec(1U){}

template <CodecType Type>
HardwareCodec<Type>::HardwareCodec(uint8_t codecFactor_) : codecFactor(codecFactor_)
{
    readAndAssertRegister(HardwareCodec::Register::MagicNumber, EXP_MAGIC_NUMBER);

    softReset();

    readAndAssertRegister(HardwareCodec::Register::WritesPerformedUpper, 0U);
    readAndAssertRegister(HardwareCodec::Register::WritesPerformedLower, 0U);
    readAndAssertRegister(HardwareCodec::Register::ReadsPerformedUpper, 0U);
    readAndAssertRegister(HardwareCodec::Register::ReadsPerformedLower, 0U);
    readAndAssertRegister(HardwareCodec::Register::IsFull, 0U);
    readAndAssertRegister(HardwareCodec::Register::IsEmpty, IS_EMPTY_CODE);

    //Write codecSetting to FPGA and set decimation/interpolation
    writeAndVerifyRegister(HardwareCodec::Register::CodecFactor, codecFactor);

    if constexpr (Type == CodecType::Decode)
    {
        writeAndVerifyRegister(HardwareCodec::Register::CodecSetting, CODEC_SETTING_DECIMATE_CODE);
    }
    else if constexpr (Type == CodecType::Encode)
    {
        writeAndVerifyRegister(HardwareCodec::Register::CodecSetting, CODEC_SETTING_INTERPOLATE_CODE);
    }
}

template <CodecType Type>
void HardwareCodec<Type>::writeRegister(HardwareCodec::Register reg, uint32_t value)
{
    const uint32_t baseAddress = getBaseAddress();
    const uint32_t address = baseAddress + static_cast<uint32_t>(reg) * sizeof(uint32_t);
    H2f::write(address, value);
}

template <CodecType Type>
void HardwareCodec<Type>::writeAndVerifyRegister(HardwareCodec::Register reg, uint32_t value)
{
    writeRegister(reg, value);
    readAndAssertRegister(reg, value);
}

template <CodecType Type>
void HardwareCodec<Type>::readAndAssertRegister(HardwareCodec::Register reg, uint32_t expected)
{
    const uint32_t readValue = readRegister(reg);

    if (readValue != expected)
    {
        const std::string error = "HardwareCodec: Asserted read failed in register " +
                                  std::to_string(static_cast<uint8_t>(reg)) +
                                  "\n"
                                  "Expected " +
                                  std::to_string(expected) +
                                  ", "
                                  "Received " +
                                  std::to_string(readValue);
        throw std::runtime_error(error);
    }
}

template <CodecType Type>
uint32_t HardwareCodec<Type>::readRegister(HardwareCodec::Register reg)
{
    const uint32_t baseAddress = getBaseAddress();
    const uint32_t address = baseAddress + static_cast<uint32_t>(reg) * sizeof(uint32_t);
    return H2f::read(address);
}

template <CodecType Type>
void HardwareCodec<Type>::softReset()
{
    const auto softResetCount = readRegister(HardwareCodec::Register::SoftResetCounter);
    static constexpr auto waitForResetTime = std::chrono::milliseconds(1);

    writeRegister(HardwareCodec::Register::SoftReset, SOFT_RESET_CODE);
    std::this_thread::sleep_for(waitForResetTime);
    writeRegister(HardwareCodec::Register::SoftReset, 0x0U);

    const auto newSoftResetCount = readRegister(HardwareCodec::Register::SoftResetCounter);

    if (softResetCount >= newSoftResetCount)
    {
        auto error = fmt::format("SoftResetCount not incremented, was: {}, now: {}", softResetCount, newSoftResetCount);
        throw std::runtime_error(error);
    }
}

template <CodecType Type>
uint64_t HardwareCodec<Type>::getWritesPerformed()
{
    const uint32_t msb = readRegister(HardwareCodec::Register::WritesPerformedUpper);
    const uint32_t lsb = readRegister(HardwareCodec::Register::WritesPerformedLower);
    const uint64_t write = (static_cast<uint64_t>(msb) << (sizeof(uint32_t) * BITS_IN_BYTE)) | lsb;
    return write;
}

template <CodecType Type>
uint64_t HardwareCodec<Type>::getReadsPerformed()
{
    const uint64_t msb = readRegister(HardwareCodec::Register::ReadsPerformedUpper);
    const uint64_t lsb = readRegister(HardwareCodec::Register::ReadsPerformedLower);
    return (msb << (sizeof(uint32_t) * BITS_IN_BYTE)) | lsb;
}

template <CodecType Type>
bool HardwareCodec<Type>::isEmpty()
{
    return (readRegister(HardwareCodec::Register::IsEmpty) == IS_EMPTY_CODE);
}

template <CodecType Type>
bool HardwareCodec<Type>::isFull()
{
    return (readRegister(HardwareCodec::Register::IsFull) == IS_FULL_CODE);
}

template <CodecType Type>
void HardwareCodec<Type>::clear()
{
    while(!isEmpty())
    {
        (void)getWord();
    }
}

template <CodecType Type>
uint32_t HardwareCodec<Type>::getWord()
{
    return readRegister(HardwareCodec::Register::ReadData);
}

template <CodecType Type>
void HardwareCodec<Type>::putWord(uint32_t word)
{
    return writeRegister(HardwareCodec::Register::WriteData, word);
}

template <CodecType Type>
std::string_view HardwareCodec<Type>::getFifoState()
{
    static constexpr auto full = "Full";
    static constexpr auto empty = "Empty";
    static constexpr auto error = "Error";
    static constexpr auto hasElems = "HasElems";

    const bool isFull = HardwareCodec::isFull();
    const bool isEmpty = HardwareCodec::isEmpty();

    if (isFull && isEmpty)
    {
        return error;
    }
    else if (isFull)
    {
        return full;
    }
    else if (isEmpty)
    {
        return empty;
    }
    else
    {
        return hasElems;
    }
}

template <CodecType Type> uint32_t HardwareCodec<Type>::getBaseAddress()
{
    if constexpr (Type == CodecType::Decode)
    {
        return H2f::HARDWARE_DECODER_ADDRESS;
    }
    else if constexpr (Type == CodecType::Encode)
    {
        return H2f::HARDWARE_ENCODER_ADDRESS;
    }
}

template class HardwareCodec<CodecType::Decode>;
template class HardwareCodec<CodecType::Encode>;
