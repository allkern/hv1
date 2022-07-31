#pragma once

#include "../../hyrisc/state.hpp"

#include "../device.hpp"

#include <fstream>
#include <vector>

class dev_uart_t : public device_t {
    hyrisc_ext_t* proc;

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

    hyu32_t base;

public:
    void reset() {
        revid1 = 0x11020002;
        revid2 = 0x00000000;
    }

    void create(hyu32_t base) {
        this->base = base;
    }

    hyu32_t read(hyu32_t addr, hyint_t size) {
        return 0x0;
    }
    
    void write(hyu32_t addr, hyu32_t value, hyint_t size) {
    }

    void init(hyrisc_ext_t* proc) {
        this->proc = proc;
    }

    void update() override {
        bool address_in_range = (proc->bci.a >= base) && (proc->bci.a <= (base + phys.size()));

        if (!address_in_range) return;
        if (!proc->bci.busreq) return;

        proc->bci.busack = true;
        proc->bci.be = 0x0;
        
        switch (proc->bci.rw) {
            case 0: proc->bci.d = read(proc->bci.a - base, proc->bci.s); break;
            case 1: write(proc->bci.a - base, proc->bci.d, proc->bci.s); break;
        }
    }
};