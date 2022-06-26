#include "hyrisc/hyrisc.hpp"

#include <iostream>

#include "log.hpp"

#include "dev/flash.hpp"
#include "dev/terminal.hpp"
#include "dev/memory.hpp"
#include "dev/bios.hpp"

int main() {
    _log::init("hyrisc");

    hyrisc_t* cpu = new hyrisc_t;

    dev_flash_t flash;
    dev_terminal_t terminal;
    dev_memory_t memory;
    dev_bios_t bios;

    bios.create(0x1000, 0x80000000);
    bios.init(&cpu->ext);
    bios.load("bios.bin");

    flash.create(0x10000, 0x90000000);
    flash.init(&cpu->ext);
    flash.load("program.bin");

    terminal.create(0xa0000000);
    terminal.init(&cpu->ext);

    memory.create(0x10000, 0x10000000);
    memory.init(&cpu->ext);

    hyrisc_pulse_reset(cpu, 0x80000000);

    cpu->ext.vcc = 1.0f;

    while (true) {
        hyrisc_clock(cpu);

        bios.update();
        flash.update();
        terminal.update();
        memory.update();

        // _log(debug, "il=%08x r0=%08x gp0=%08x gp1=%08x a0=%08x pc=%08x, busreq=%u, busack=%u",
        //     cpu->internal.instruction,
        //     cpu->internal.r[r0],
        //     cpu->internal.r[gp0],
        //     cpu->internal.r[gp1],
        //     cpu->internal.r[a0],
        //     cpu->internal.r[pc],
        //     cpu->ext.bci.busreq,
        //     cpu->ext.bci.busack
        // );
    }
}