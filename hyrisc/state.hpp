#pragma once

typedef uint8_t  hyu8_t;
typedef uint16_t hyu16_t;
typedef uint32_t hyu32_t;
typedef uint64_t hyu64_t;
typedef  int8_t  hyi8_t;
typedef  int16_t hyi16_t;
typedef  int32_t hyi32_t;
typedef  int64_t hyi64_t;
typedef float    hyfloat_t;
typedef bool     hybool_t;
typedef int      hyint_t;

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

// Internal data and latches
struct hyrisc_int_t {
    hyint_t     cycle;          // Cycle counter
    hyu32_t     instruction;    // Instruction latch
    hyu32_t     r[32];          // GPRs
    hyint_t     last_cycles;    // Last cycles latch
    hyfloat_t   f[16];          // FPRs
    hyu8_t      opcode;         // Opcode latch
    hyu8_t      st;             // State register
    hybool_t    rw;             // Access type flag
    hyint_t     link_level;     // Link register level
};

struct hyrisc_t {
    hyrisc_int_t internal;
    hyrisc_ext_t ext;
};