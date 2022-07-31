#pragma once

#include "../../hyrisc/state.hpp"

struct dev_uart_state_t {
    hyu32_t rbr;         // Receiver Buffer Register (read only)
    hyu32_t thr;         // Transmitter Holding Register (write only)
    hyu32_t ier;         // Interrupt Enable Register
    hyu32_t iir;         // Interrupt Identification Register (read only)
    hyu32_t fcr;         // FIFO Control Register (write only)
    hyu32_t lcr;         // Line Control Register
    hyu32_t mcr;         // Modem Control Register (Unimplemented)
    hyu32_t lsr;         // Line Status Register
    hyu32_t msr;         // Modem Status Register (Unimplemented)
    hyu32_t scr;         // Scratch Pad Register
    hyu32_t dll;         // Divisor LSB Latch
    hyu32_t dlh;         // Divisor MSB Latch
    hyu32_t revid1;      // Revision Identification Register 1
    hyu32_t revid2;      // Revision Identification Register 2
    hyu32_t pwremu_mgmt; // Power and Emulation Management Register
    hyu32_t mdr;         // Mode Definition Register
};