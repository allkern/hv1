#pragma once

#include "../hyrisc/state.hpp"

#include "device.hpp"

#include <fstream>
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <conio.h>

int getchar_impl() {
    if (_kbhit()) {
        return _getch();
    } else {
        return 0;
    }
}

#endif

#ifdef __linux__
#include "termios.h"
#include "unistd.h"

int getchar_impl() {
    int c;
 
    static termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO); 

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c;
}
#endif

class dev_terminal_t : public device_t {
    hyrisc_ext_t* proc;

    hyu32_t base;

public:
    void create(hyu32_t base) {
        this->base = base;
    }

    hyu32_t read(hyu32_t addr, hyint_t size) {
        switch (addr) {
            case 0x0: return 0x0;
            case 0x1: return getchar_impl();
        }

        return 0x0;
    }
    
    void write(hyu32_t addr, hyu32_t value, hyint_t size) {
        if (addr == 0x0) {
            std::cout << (char)(value & 0xff);
        }
    }

    void init(hyrisc_ext_t* proc) {
        this->proc = proc;
    }

    void update() override {
        bool address_in_range = (proc->bci.a >= base) && (proc->bci.a <= (base + 2));

        if (!address_in_range) return;
        if (!proc->bci.busreq) return;

        proc->bci.busack = true;
        proc->bci.be = 0x0;
        
        switch (proc->bci.rw) {
            case 0: proc->bci.d = read(proc->bci.a - base, proc->bci.s); break;
            case 1: write(proc->bci.a - base, proc->bci.d, proc->bci.s); break;
        }
    }
};