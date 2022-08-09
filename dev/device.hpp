#pragma once

#include "../hyrisc/state.hpp"

class device_t {
public:
    //virtual hyu32_t read(hyu32_t addr, hyint_t size) { return 0x0; };
    //virtual void write(hyu32_t addr, hyu32_t value, hyint_t size) {};
    virtual void init(hyrisc_ext_t* proc) {};
    virtual void update() {};
};