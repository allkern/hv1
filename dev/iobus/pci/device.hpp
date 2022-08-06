#pragma once

#include "../ext.hpp"

#define PCI_BAR_MEM 0
#define PCI_BAR_IO  1

struct pci_device_t {
    // Don't support multi-function devices yet
    int bus, device;
    bool disabled = false;

    hyu32_t devid,
            vendor,
            status,
            command,
            devclass,
            subclass,
            pif,
            rev,
            bist,
            hdr,
            lat,
            clsize,
            bar[6];

    // virtual void init(iobus_ext_t* iobus) {};
    // virtual void update() {};
};