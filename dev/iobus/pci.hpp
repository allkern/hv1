#pragma once

#include "device.hpp"
#include "../../hyrisc/state.hpp"

class iobus_dev_pci_t : public iobus_device_t {
    hyu16_t device,
            vendor,
            status;
    hyu8_t  devclass,
            subclass,
            pif,
            rev,
            bist,
            hdr,
            lat,
            clsize;
    hyu32_t bar[6];
    hyu32_t cardbus_cis_ptr;
    



};