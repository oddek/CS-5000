

#include "DataMover.h"
#include "H2f.h"
#include <stdexcept>
#include <thread>

uint32_t DataMover::rxBufferAddress{0};
uint32_t DataMover::rxBufferSize{0};
uint32_t DataMover::txBufferAddress{0};
uint32_t DataMover::txBufferSize{0};
uint32_t DataMover::bitrate{0};

void DataMover::init(uint32_t rxBufferAddress_, uint32_t rxBufferSize_, uint32_t txBufferAddress_,
                     uint32_t txBufferSize_, size_t bitrate_)
{
    rxBufferAddress = rxBufferAddress_;
    rxBufferSize = rxBufferSize_;
    txBufferAddress = txBufferAddress_;
    txBufferSize = txBufferSize_;
    bitrate = bitrate_;

    init();
}

void DataMover::init()
{
    readAndAssertRegister(DataMover::Register::MagicNumber, EXP_MAGIC_NUMBER);

    softReset();

    writeAndVerifyRegister(DataMover::Register::WriteBufferStartAddress, rxBufferAddress);
    writeAndVerifyRegister(DataMover::Register::WriteBufferSize, rxBufferSize);
    readAndAssertRegister(DataMover::Register::CurrentWriteAddress, rxBufferAddress);
    readAndAssertRegister(DataMover::Register::WritesPerformedUpper, 0U);
    readAndAssertRegister(DataMover::Register::WritesPerformedLower, 0U);

    writeAndVerifyRegister(DataMover::Register::ReadBufferStartAddress, txBufferAddress);
    writeAndVerifyRegister(DataMover::Register::ReadBufferSize, txBufferSize);
    writeAndVerifyRegister(DataMover::Register::ResetReadBufferOnEnable, 0U);
//    readAndAssertRegister( DataMover::Register::CurrentReadAddress, txBufferAddress );
    readAndAssertRegister(DataMover::Register::ReadsPerformedUpper, 0U);
    readAndAssertRegister(DataMover::Register::ReadsPerformedLower, 0U);
    readAndAssertRegister(DataMover::Register::ReadErrorCount, 0U);
    readAndAssertRegister(DataMover::Register::LastReceivedReadUpper, 0U);
    readAndAssertRegister(DataMover::Register::LastReceivedReadLower, 0U);
    readAndAssertRegister(DataMover::Register::ReadUnderrunFlag, 0U);

    readAndAssertRegister(DataMover::Register::WriteEnable, 0U);
    readAndAssertRegister(DataMover::Register::ReadEnable, READ_NOT_IN_PROGRESS_CODE);

    readAndAssertRegister(DataMover::Register::WriteTickInterval, 0U);

    setWriteInterval(bitrate);
}

void DataMover::setWriteInterval(size_t newBitrate)
{
    static const uint32_t tickInterval = CLOCK_SPEED / (newBitrate / F2H_BUS_WIDTH);

    writeAndVerifyRegister(DataMover::Register::WriteTickInterval, tickInterval - 2U);
}

void DataMover::writeRegister(DataMover::Register reg, uint32_t value)
{
    const uint32_t address = H2f::DATA_MOVER_ADDRESS + static_cast<uint32_t>(reg) * 4U;

    H2f::write(address, value);
}

void DataMover::writeAndVerifyRegister(DataMover::Register reg, uint32_t value)
{
    writeRegister(reg, value);
    readAndAssertRegister(reg, value);
}

void DataMover::readAndAssertRegister(DataMover::Register reg, uint32_t expected)
{
    const uint32_t readValue = readRegister(reg);

    if (readValue != expected)
    {
        const std::string error = "DataMover: Asserted read failed in register " +
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

uint32_t DataMover::readRegister(DataMover::Register reg)
{
    const uint32_t address = H2f::DATA_MOVER_ADDRESS + static_cast<uint32_t>(reg) * 4U;
    return H2f::read(address);
}

uint32_t DataMover::getState()
{
    return readRegister(DataMover::Register::CurrentState);
}

void DataMover::softReset()
{
    const auto softResetCount = readRegister(DataMover::Register::SoftResetCounter);
    static constexpr auto waitForResetTime = std::chrono::milliseconds(1);

    writeRegister(DataMover::Register::SoftReset, SOFT_RESET_CODE);
    std::this_thread::sleep_for(waitForResetTime);
    writeRegister(DataMover::Register::SoftReset, 0x0U);

    const auto newSoftResetCount = readRegister(DataMover::Register::SoftResetCounter);

    if (softResetCount >= newSoftResetCount)
    {
        auto error = fmt::format("SoftResetCount not incremented, was: {}, now: {}", softResetCount, newSoftResetCount);
        throw std::runtime_error(error);
    }
}

uint32_t DataMover::getRxWritePointer()
{
    return readRegister(DataMover::Register::CurrentWriteAddress);
}

void DataMover::rxEnable()
{
    writeAndVerifyRegister(DataMover::Register::WriteEnable, UINT32_MAX);
}

void DataMover::rxDisable()
{
    writeAndVerifyRegister(DataMover::Register::WriteEnable, 0x0U);
}

bool DataMover::rxIsEnabled()
{
    return (readRegister(DataMover::Register::WriteEnable) == UINT32_MAX);
}

uint64_t DataMover::rxWritesPerformed()
{
    const uint64_t msb = readRegister(DataMover::Register::WritesPerformedUpper);
    const uint64_t lsb = readRegister(DataMover::Register::WritesPerformedLower);
    return (msb << (sizeof(uint32_t) * BITS_IN_BYTE)) | lsb;
}

uint32_t DataMover::getRxBufferStartAddress()
{
    return readRegister(DataMover::Register::WriteBufferStartAddress);
}

uint32_t DataMover::getRxBufferSize()
{
    return readRegister(DataMover::Register::WriteBufferSize);
}

uint32_t DataMover::getTxReadIndex()
{
    return getTxReadPointer() - DataMover::txBufferAddress;
}

uint32_t DataMover::getTxReadPointer()
{
    return readRegister(DataMover::Register::CurrentReadAddress);
}

void DataMover::txEnable()
{
    writeRegister(DataMover::Register::ReadEnable, READ_ENABLE_CODE);
}

void DataMover::txDisable()
{
    writeRegister(DataMover::Register::ReadEnable, READ_NOT_IN_PROGRESS_CODE);
}

bool DataMover::txIsEnabled()
{
    return (readRegister(DataMover::Register::ReadEnable) == READ_IN_PROGRESS_CODE);
}

uint64_t DataMover::txReadsPerformed()
{
    const uint64_t msb = readRegister(DataMover::Register::ReadsPerformedUpper);
    const uint64_t lsb = readRegister(DataMover::Register::ReadsPerformedLower);
    return (msb << (sizeof(uint32_t) * BITS_IN_BYTE)) | lsb;
}

uint32_t DataMover::getTxBufferStartAddress()
{
    return readRegister(DataMover::Register::ReadBufferStartAddress);
}

uint32_t DataMover::getTxBufferSize()
{
    return readRegister(DataMover::Register::ReadBufferSize);
}

uint32_t DataMover::getTxReadErrors()
{
    return readRegister(DataMover::Register::ReadErrorCount);
}

uint64_t DataMover::getTxLastRead()
{
    const uint64_t msb = readRegister(DataMover::Register::LastReceivedReadUpper);
    const uint64_t lsb = readRegister(DataMover::Register::LastReceivedReadLower);
    return (msb << (sizeof(uint32_t) * BITS_IN_BYTE)) | lsb;
}

uint32_t DataMover::txGetStopIndex()
{
    return readRegister(DataMover::Register::ReadStopPointAddress) - DataMover::txBufferAddress;
}

void DataMover::txSetStopIndex(uint32_t index)
{
    writeAndVerifyRegister(DataMover::Register::ReadStopPointAddress, DataMover::txBufferAddress + index);
}

void DataMover::txSetStopFlag()
{
    writeRegister(DataMover::Register::SetReadStopFlag, READ_STOP_FLAG_CODE);
}

void DataMover::txResetReadAddress()
{
    writeRegister(DataMover::Register::ResetReadBufferOnEnable, RESET_READ_ADDRESS_CODE);
}

void DataMover::txClearUnderrunFlag()
{
    writeRegister(DataMover::Register::ClearUnderrunFlag, CLEAR_UNDERRUN_FLAG_CODE);
}

bool DataMover::isTxUnderrunFlagSet()
{
    return (readRegister(DataMover::Register::ReadUnderrunFlag) == READ_UNDERRUN_FLAG_IS_SET_CODE);
}

//void DataMover::txSetReadAmount(uint32_t amount)
//{
//    writeAndVerifyRegister(DataMover::Register::ReadAmount, amount);
//}
