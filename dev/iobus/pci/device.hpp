#pragma once

#include "../ext.hpp"

class pci_device_t {
public:
    virtual void init(iobus_ext_t* iobus) {};
    virtual void update() {};
};