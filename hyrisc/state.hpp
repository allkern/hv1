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

struct hysignal_t {
    hybool_t prev;
    hybool_t current;

    enum trigger_t {
        LEVEL_LOW,
        RISING,
        FALLING,
        LEVEL_HIGH,
        EDGE,
        LEVEL_ANY,
        HIGH,
        LOW
    } trigger;
};

void hysignal_set(hysignal_t* sig, bool value) {
    sig->prev = sig->current;
    sig->current = value;
}

bool hysignal_get(hysignal_t* sig) {
    switch (sig->trigger) {
        case hysignal_t::LEVEL_LOW : return  !sig->prev && !sig->current;
        case hysignal_t::RISING    : return  !sig->prev &&  sig->current;
        case hysignal_t::FALLING   : return   sig->prev && !sig->current;
        case hysignal_t::LEVEL_HIGH: return   sig->prev &&  sig->current;
        case hysignal_t::EDGE      : return (!sig->prev &&  sig->current) || ( sig->prev && !sig->current);
        case hysignal_t::LEVEL_ANY : return (!sig->prev && !sig->current) || ( sig->prev &&  sig->current);
        case hysignal_t::HIGH      : return   sig->current;
        case hysignal_t::LOW       : return  !sig->current;
    }

    return false;
}

bool hysignal_get(hysignal_t* sig, hysignal_t::trigger_t trigger) {
    switch (trigger) {
        case hysignal_t::LEVEL_LOW : return  !sig->prev && !sig->current;
        case hysignal_t::RISING    : return  !sig->prev &&  sig->current;
        case hysignal_t::FALLING   : return   sig->prev && !sig->current;
        case hysignal_t::LEVEL_HIGH: return   sig->prev &&  sig->current;
        case hysignal_t::EDGE      : return (!sig->prev &&  sig->current) || ( sig->prev && !sig->current);
        case hysignal_t::LEVEL_ANY : return (!sig->prev && !sig->current) || ( sig->prev &&  sig->current);
        case hysignal_t::HIGH      : return   sig->current;
        case hysignal_t::LOW       : return  !sig->current;
    }

    return false;
}

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
    // Internal state machine
    enum state_t : int {
        ST_FETCHING,
        ST_EXECUTING
    } state;

    hyint_t     cycle;          // Cycle counter
    hyu32_t     instruction;    // Instruction latch
    hyu32_t     r[32];          // GPRs
    hyint_t     last_cycles;    // Last cycles latch
    hyfloat_t   f[16];          // FPRs
    hyu8_t      opcode;         // Opcode latch
    hyu8_t      st;             // State register
    hybool_t    memory_access;  // Memory access flag
    hybool_t    rw;             // Access type flag
    hyu32_t     shadow_address; // Temporary address latch
    hyu32_t     shadow_data;    // Temporary data latch
};

struct hyrisc_t {
    hyrisc_int_t internal;
    hyrisc_ext_t ext;
};