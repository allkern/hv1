#pragma once

#include "../../hyrisc/state.hpp"

struct iobus_ext_t {
    hyu32_t addr;
    hyu32_t data;
    hybool_t rw;
};