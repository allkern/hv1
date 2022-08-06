/*
    pci.hpp - Peripheral Component Interconnect Bus Emulator
    Copyright (C) 2022 Lycoder <github.com/lycoder>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "device.hpp"
#include "pci/device.hpp"
#include "../../hyrisc/state.hpp"

#include <vector>

#define IOBUS_PCI_CFG_ADDR 0xcf8
#define IOBUS_PCI_CFG_DATA 0xcfc

class iobus_dev_pci_t : public iobus_device_t {
    iobus_ext_t* iobus;

    std::vector <pci_device_t*> devices;

    pci_device_t* search_device(int bus, int device) {
        for (pci_device_t* dev : devices)
            if ((dev->bus == bus) && (dev->device == device))
                return dev;
    }

public:
    hyu32_t addr;
    hyu8_t bus, device, function, reg;
    bool enable;

    void register_device(pci_device_t* dev) {
        devices.push_back(dev);
    }

    void init(iobus_ext_t* iobus) override {
        this->iobus = iobus;
    }

    void update() override {
        switch (iobus->port) {
            case IOBUS_PCI_CFG_ADDR: {
                switch (iobus->rw) {
                    case RW_READ : { iobus->data = addr; } break;
                    case RW_WRITE: { addr = iobus->data; } break;
                }
            } break;

            case IOBUS_PCI_CFG_DATA: {
                reg      = (addr >> 0 ) & 0xfc;
                function = (addr >> 8 ) & 0x7;
                device   = (addr >> 11) & 0x1f;
                bus      = (addr >> 16) & 0xff;
                enable   =  addr & 0x80000000;

                pci_device_t* dev = search_device(bus, device);

                switch (iobus->rw) {
                    case RW_READ: {
                        if (dev) {
                            switch (reg >> 2) {
                                case 0x0: iobus->data = (dev->devid    << 16) | (dev->vendor   << 0 )                                       ; break;
                                case 0x1: iobus->data = (dev->status   << 16)                                                               ; break;
                                case 0x2: iobus->data = (dev->devclass << 24) | (dev->subclass << 16) | (dev->pif << 8) | (dev->rev    << 0); break;
                                case 0x3: iobus->data = (dev->bist     << 24) | (dev->hdr      << 16) | (dev->lat << 8) | (dev->clsize << 0); break;
                                case 0x4: case 0x5: case 0x6: case 0x7: case 0x8: case 0x9: {
                                    iobus->data = dev->bar[(reg >> 2) - 0x4];
                                } break;
                                default: {
                                    // Unsupported PCI register read
                                } break;
                            }
                        } else {
                            iobus->data = 0xffffffff;
                        }
                    } break;

                    case RW_WRITE: {
                        if (dev) {
                            switch (reg >> 2) {
                                case 0x1: {
                                    if (!iobus->data) dev->disabled = true;

                                    dev->command = iobus->data;

                                    // dev->update();
                                } break;
                                default: {
                                    // Unsupported PCI register write
                                }
                            }
                        }

                        // Writes to non-existent devices are ignored
                    }
                }
            } break;
        }
    }
};