#pragma once

#include "../hyrisc/state.hpp"

#include "device.hpp"

#include <fstream>
#include <vector>

class dev_flash_t : public device_t {
    hyrisc_ext_t* proc;

    std::vector <hyu8_t> buf;

    hyu32_t base;

    hyu8_t read8(hyu32_t addr) {
        return buf[addr];
    }

    hyu16_t read16(hyu32_t addr) {
        return (read8(addr + 1) << 8) | read8(addr);
    }

    hyu32_t read32(hyu32_t addr) {
        return (read16(addr + 2) << 16) | read16(addr);
    }

    void write8(hyu32_t addr, hyu32_t value) {
        _log(debug, "addr=%08x, value=%08x", addr, value);
        buf[addr] = value & 0xff;
    }

    void write16(hyu32_t addr, hyu32_t value) {
        write8(addr    , (value     ) & 0xff);
        write8(addr + 1, (value >> 8) & 0xff);
    }

    void write32(hyu32_t addr, hyu32_t value) {
        write16(addr    , (value      ) & 0xffff);
        write16(addr + 2, (value >> 16) & 0xffff);
    }

public:
    void create(size_t size, hyu32_t base) {
        buf.resize(size);

        this->base = base;
    }

    hyu32_t read(hyu32_t addr, hyint_t size) override {
        switch (size) {
            case 0: return read8(addr);
            case 1: return read16(addr);
            case 2: return read32(addr);
        }

        return 0x0;
    }
    
    void write(hyu32_t addr, hyu32_t value, hyint_t size) override {
        switch (size) {
            case 0: { write8(addr, value); return; }
            case 1: { write16(addr, value); return; }
            case 2: { write32(addr, value); return; }
        }
    }

    void init(hyrisc_ext_t* proc) {
        this->proc = proc;
    }

    void load(std::string fn) {
        std::ifstream file(fn, std::ios::binary);

        file.read((char*)buf.data(), buf.size());
    }

    void update() override {
        bool address_in_range = (proc->bci.a >= base) && (proc->bci.a <= (base + buf.size()));

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