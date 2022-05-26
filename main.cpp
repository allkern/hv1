#include "hyrisc/hyrisc.hpp"

#include <iostream>

#include "log.hpp"

#include "dev/flash.hpp"

// hyu32_t program[] = {
//     0x000f05ff,
//     0xffff05de,
//     0x002006ef,
//     0x0000007f,
//     0x0000007f,
//     0x0000007f,
//     0x0000007f
// };

int main() {
    _log::init("hyrisc");

    hysignal_t* clock = new hysignal_t;
    hyrisc_t* cpu = new hyrisc_t;

    dev_flash_t flash;

    flash.create(0x10000, 0x80000000);
    flash.init(&cpu->ext);
    flash.write(0x00000000, 0xfffffdef, 2);
    flash.load("hyrisc-a/test.bin");

    hyrisc_reset(cpu);

    cpu->ext.vcc = 1.0f;
    cpu->internal.r[pc] = 0x80000000;
    //hysignal_set(&cpu->ext.bci.busirq, true);

    for (int i = 0; i < 8; i++) {
        hyrisc_clock(cpu);
        _log(debug, "before flash bci.a=%08x, bci.d=%08x, bci.busreq=%u, bci.busack=%u", cpu->ext.bci.a, cpu->ext.bci.d, cpu->ext.bci.busreq, cpu->ext.bci.busack);
        flash.update();
        _log(debug, "after  flash bci.a=%08x, bci.d=%08x, bci.busreq=%u, bci.busack=%u", cpu->ext.bci.a, cpu->ext.bci.d, cpu->ext.bci.busreq, cpu->ext.bci.busack);

        _log(debug, "pc: %08x", cpu->internal.r[pc]);
    }
}