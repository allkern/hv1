#pragma once

#include "ext.hpp"

class iobus_device_t {
public:
    virtual void init(iobus_ext_t* iobus) {};
    virtual void update() {};
};