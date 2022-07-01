#pragma once

#include "types.hpp"
#include "state.hpp"

#define Z 0b00000001
#define N 0b00000010
#define V 0b00000100
#define C 0b00001000

inline static void hyrisc_set_flags(hyrisc_t* proc, hyu8_t mask, bool cond, bool reset = true) {
    if (cond) {
        proc->internal.st |= mask;
        return;
    }

    if (reset) proc->internal.st &= ~mask;
}