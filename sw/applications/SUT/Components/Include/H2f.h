
#ifndef H2F_H
#define H2F_H

#include "Altera_Devkit.h"
#include <cstddef>
#include <cstdint>
#include <mutex>

class H2f
{
  public:
    H2f(const H2f&) = delete;
    H2f(H2f&&) = delete;
    H2f& operator=(const H2f&) = delete;
    H2f& operator=(H2f&&) = delete;

    static void init();
    static uint32_t read(uint32_t address);
    static void write(uint32_t address, uint32_t value);
    static void blinkUpdate();

    static constexpr uintptr_t BLUE_LIGHT_ADDRESS = MASTER_PIO_LED_BLUE_BASE;
    static constexpr uintptr_t DATA_MOVER_ADDRESS = DATAMOVER_0_BASE;
    static constexpr uintptr_t HARDWARE_DECODER_ADDRESS = DECODER_0_BASE;
    static constexpr uintptr_t HARDWARE_ENCODER_ADDRESS = ENCODER_0_BASE;

  private:
    H2f();
    ~H2f();
    static void cleanup();

    static int devMemFd;
    static void* h2fVirtualBase;

    static constexpr uintptr_t HPS_FPGA_BRIDGE_BASE = 0xC0000000;
    static constexpr uintptr_t HW_REGS_BASE = HPS_FPGA_BRIDGE_BASE;
    static constexpr size_t HW_REGS_SPAN = (0x40000000U);
    [[maybe_unused]] static constexpr uint32_t HW_REGS_MASK = (HW_REGS_SPAN - 1U);
};

#endif