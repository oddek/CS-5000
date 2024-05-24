

#ifndef DATA_MOVER_H
#define DATA_MOVER_H

#include "Utils.h"
#include <cstddef>
#include <cstdint>

class DataMover
{
  public:
    enum class Register
    {
        MagicNumber = 0U,
        CurrentWriteAddress,
        CurrentReadAddress,
        WritesPerformedUpper,
        WritesPerformedLower,
        ReadsPerformedUpper,
        ReadsPerformedLower,
        ReadErrorCount,
        ReadUnderrunFlag,
        LastReceivedReadUpper,
        LastReceivedReadLower,
        CurrentState,

        WriteBufferStartAddress,
        WriteBufferSize,
        ReadBufferStartAddress,
        ReadBufferSize,
        ReadStopPointAddress,
        SetReadStopFlag,
        ClearUnderrunFlag,
        ResetReadBufferOnEnable,
        WriteEnable,
        ReadEnable,
        WriteTickInterval,
        ReadTickInterval,
        SoftReset,
        ResetCounter,
        SoftResetCounter,
        RegisterLast
    };

    DataMover() = delete;
    static void init();
    static void init(uint32_t rxBufferAddress_, uint32_t rxBufferSize_, uint32_t txBufferAddress_,
                     uint32_t txBufferSize_, size_t bitrate_);
    static void writeRegister(Register, uint32_t);
    static void writeAndVerifyRegister(Register, uint32_t);
    static void readAndAssertRegister(Register, uint32_t);
    static uint32_t readRegister(Register);

    static void softReset();

    static void rxEnable();
    static void rxDisable();
    static uint32_t getRxWritePointer();
    static bool rxIsEnabled();
    static uint64_t rxWritesPerformed();
    static uint32_t getRxBufferStartAddress();
    static uint32_t getRxBufferSize();

    static void txEnable();
    static void txDisable();
    static uint32_t getTxReadPointer();
    static bool txIsEnabled();
    static uint64_t txReadsPerformed();
    static uint32_t getTxBufferStartAddress();
    static uint32_t getTxBufferSize();
    static uint32_t getTxReadErrors();
    static uint64_t getTxLastRead();

    static void txSetStopIndex(uint32_t index);
    static void txSetStopFlag();
    static void txResetReadAddress();
    static void txClearUnderrunFlag();
    static bool isTxUnderrunFlagSet();
    static uint32_t getTxReadIndex();
    static uint32_t txGetStopIndex();

    static uint32_t getState();

  private:
    static void setWriteInterval(size_t bitrate);
    static constexpr uint32_t EXP_MAGIC_NUMBER = 0x2224ABCDU;
    static constexpr uint32_t SOFT_RESET_CODE = 0xFF00FF00U;
    static constexpr uint32_t WRITE_ENABLE_CODE = 0xFFFFFFFFU;
    static constexpr uint32_t READ_ENABLE_CODE = 0xFFFFFFFFU;
    static constexpr uint32_t READ_STOP_FLAG_CODE = 0x12345678U;
    static constexpr uint32_t READ_UNDERRUN_FLAG_IS_SET_CODE = 0x89271203U;
    static constexpr uint32_t CLEAR_UNDERRUN_FLAG_CODE = 0x87654321U;
    static constexpr uint32_t RESET_READ_ADDRESS_CODE = 0x83920394U;
    static constexpr uint32_t READ_IN_PROGRESS_CODE = 0xB0B0A0A0U;
    static constexpr uint32_t READ_NOT_IN_PROGRESS_CODE = 0xA0A0B0B0;

    static constexpr size_t CLOCK_SPEED = 100_MHz;
    static constexpr size_t F2H_BUS_WIDTH = 64;

    static uint32_t rxBufferAddress;
    static uint32_t rxBufferSize;
    static uint32_t txBufferAddress;
    static uint32_t txBufferSize;
    static uint32_t bitrate;
};

#endif