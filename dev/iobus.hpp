#pragma once

#include "../hyrisc/state.hpp"

#include "device.hpp"
#include "iobus/device.hpp"

#include <vector>

#define IN_RANGE(base, size) ((proc->bci.a >= base) && (proc->bci.a < (base + size)))
#define IOBUS_PORT 0xfffffffe
#define IOBUS_DATA 0xffffffff

class dev_iobus_t : public device_t {
    hyrisc_ext_t* proc;
    iobus_ext_t ext;

    std::vector <iobus_device_t*> devices;

    hyu32_t base = 0xfffffffe;
    hyint_t size = 2;

public:
    void create(hyu32_t base) {
        this->base = base;
    }

    void init(hyrisc_ext_t* proc) {
        this->proc = proc;

        for (iobus_device_t* dev : devices)
            dev->init(&ext);
    }

    void update() override {
        if (!(proc->bci.busreq && IN_RANGE(base, size))) return;

        proc->bci.busack = true;
        proc->bci.be = 0x0;

        switch (proc->bci.a) {
            case IOBUS_PORT: {
                switch (proc->bci.rw) {
                    case RW_WRITE: ext.port = proc->bci.d; break;
                    case RW_READ : proc->bci.d = ext.port; break;
                }
            } break;

            case IOBUS_DATA: {
                switch (proc->bci.rw) {
                    case RW_WRITE: {
                        ext.data = proc->bci.d;
                        ext.rw   = RW_WRITE;

                        for (iobus_device_t* dev : devices)
                            dev->update();
                    } break;

                    case RW_READ: {
                        ext.rw = RW_READ;

                        for (iobus_device_t* dev : devices)
                            dev->update();
                        
                        proc->bci.d = ext.data;
                    } break;
                }
            } break;
        }
    }
};