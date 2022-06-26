#pragma once

#include "../hyrisc/state.hpp"

#include "device.hpp"
#include "iobus/device.hpp"

#include <vector>

class dev_iobus_t : public device_t {
    hyrisc_ext_t* proc;
    iobus_ext_t ext;

    std::vector <iobus_device_t*> devices;

    hyu32_t base;
    hyint_t size = 2;

public:
    void create(hyu32_t base) {
        this->base = base;
    }

    hyu32_t read(hyu32_t addr, hyint_t size) override {}
    
    void write(hyu32_t addr, hyu32_t value, hyint_t size) override {}

    void init(hyrisc_ext_t* proc) {
        this->proc = proc;

        for (iobus_device_t* dev : devices)
            dev->init(&ext);
    }

    void update() override {
        if (!proc->bci.busreq) return;

        bool address_in_range = (proc->bci.a >= base) && (proc->bci.a <= (base + size));

        if (!address_in_range) return;

        proc->bci.busack = true;
        proc->bci.be = 0x0;
        
        switch (proc->bci.rw) {
            case RW_READ: {
                ext.addr = proc->bci.a;
                ext.rw = RW_READ;

                for (iobus_device_t* dev : devices)
                    dev->update();
                
                proc->bci.d = ext.data;
            } break;

            case RW_WRITE: {
                ext.addr = proc->bci.a;
                ext.data = proc->bci.d;
                ext.rw = RW_WRITE;

                for (iobus_device_t* dev : devices)
                    dev->update();
            } break;
        }
    }
};