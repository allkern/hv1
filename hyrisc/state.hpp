#pragma once

#include "types.hpp"

enum rw_mode_t : bool {
    RW_READ = false,
    RW_WRITE = true
};

// Bus Controller Interface
struct hyrisc_bci_t {
    hyu32_t  a;       // A0-A31 pins (Address bus)
    hyu32_t  d;       // D0-D31 pins (Data bus)
    hybool_t busirq;  // BUSIRQ pin (Bus IRQ on Error)
    hybool_t busack;  // BUSACK pin (Bus Acknowledge)
    hybool_t busreq;  // BUSREQ pin (Bus Request)
    hybool_t rw;      // RW pin (Read/Write)
    hyu8_t   be;      // BE0-BE8 pins (Bus Error)
    hyu8_t   s;       // S0-S1 pins (Data size)
};

// Internal PIC Interface
struct hyrisc_pic_t {
    hyu32_t  v;       // V0-V31 pins (IRQ Vector)
    hybool_t irq;     // IRQ pin (IRQ trigger)
    hybool_t irqack;  // IRQACK pin (IRQ Acknowledge)
};

// External pins and buses
struct hyrisc_ext_t {
    hyrisc_bci_t bci;
    hyrisc_pic_t pic;
    //hysignal_t*     clk;       // CLK pin (Clock Input)
    hybool_t     reset;          // RESET pin
    hybool_t     freeze;         // FREEZE pin
    hyfloat_t    vcc;
};

// Decoder latches
struct hyrisc_decoder_t {
    hyu8_t      opcode;         // Opcode
    hyu8_t      encoding;       // Encoding
    hyu8_t      fieldx;         // Bitfield X
    hyu8_t      fieldy;         // Bitfield Y
    hyu8_t      fieldz;         // Bitfield Z
    hyu8_t      fieldw;         // Bitfield W
    hyu8_t      size;           // Access size
    hyu8_t      imm8;           // 8-bit immediate
    hyu16_t     imm16;          // 16-bit immediate
    // hyu8_t   cond;           // Condition code (Bitfield X)
    // hyu8_t   shift_mul;      // Shift/multiply (Bitfield W)
};

// Internal data and latches
struct hyrisc_int_t {
    hyint_t          cycle;          // Cycle counter
    hyu32_t          instruction;    // Instruction latch
    hyu32_t          r[32];          // GPRs
    hyint_t          last_cycles;    // Last cycles latch
    hyfloat_t        f[32];          // FPRs and FPCSR
    hyu8_t           st;             // State register
    hybool_t         rw;             // Access type flag
    hyrisc_decoder_t decoder;
};

struct hyrisc_t {
    // For debugging purposes
    const char* id;
    hyint_t core;

    hyrisc_int_t internal;
    hyrisc_ext_t ext;
};