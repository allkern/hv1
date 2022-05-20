#include "cpu/cpu.hpp"

#include <iostream>

cpu::u32 program[] = {
    0x000f05ff,
    0xffff05de,
    0x002006ef,
    0x0000007f,
    0x0000007f,
    0x0000007f,
    0x0000007f
};

int main() {
    hysignal_t clock;

    hyrisc_t* cpu = new hyrisc_t;
}