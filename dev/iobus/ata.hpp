/*
    ata.hpp - IOBus/PCI ATA/ATAPI IDE Controller
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

#include <string>

#include "device.hpp"
#include "pci/device.hpp"
#include "../../hyrisc/state.hpp"

#include "../block.hpp"

#define IOBUS_ATA_PRI_IO   0x1f0
#define IOBUS_ATA_PRI_CTRL 0x3f6
#define IOBUS_ATA_SEC_IO   0x170
#define IOBUS_ATA_SEC_CTRL 0x376

#define IOBUS_ATA_IO_BEGIN   IOBUS_ATA_PRI_IO
#define IOBUS_ATA_IO_SIZE    0xd
#define IOBUS_ATA_CTRL_BEGIN IOBUS_ATA_PRI_CTRL
#define IOBUS_ATA_CTRL_SIZE  0x3

// ATA Registers
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0a
#define ATA_REG_LBA5       0x0b
#define ATA_REG_CONTROL    0x0c
#define ATA_REG_ALTSTATUS  0x0c
#define ATA_REG_DEVADDRESS 0x0d

#define ATA_PRIMARY     0
#define ATA_SECONDARY   1
#define ATA_MASTER      0
#define ATA_SLAVE       1

#define ATA_SECTOR_SIZE 0x200

struct ata_channel_t {
    int drive_number = ATA_MASTER;

    struct drive_t {
        block_dev_t blk;                        // Each drive gets a block device for outputting to a file
        hyu16_t     rw_base_lba[3];             // This is the base LBA for RW ops
        hyu16_t     rw_sectors;                 // Sector count for RW ops
        size_t      rw_pending_bytes;           // Pending RWs from the PIO port
        uint8_t     rw_buf[ATA_SECTOR_SIZE];    // Sector-sized buffer for RW ops
        uint8_t     error;                      // Drive command error
    } drive[2];
};

#define ATA_PRI_MASTER 0
#define ATA_PRI_SLAVE  1
#define ATA_SEC_MASTER 2
#define ATA_SEC_SLAVE  3

class iobus_dev_ata_t : public iobus_device_t {
    iobus_ext_t* iobus;
    pci_device_t pci;
    ata_channel_t primary, secondary;
    hyu16_t iobus_pri_io_base;
    hyu16_t iobus_pri_ctrl_base;
    hyu16_t iobus_sec_io_base;
    hyu16_t iobus_sec_ctrl_base;

public:
    bool attach_drive(const std::string& path, int attachment) {
        switch (attachment) {
            case ATA_PRI_MASTER: return primary.drive[ATA_MASTER].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_PRI_SLAVE : return primary.drive[ATA_SLAVE].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_SEC_MASTER: return secondary.drive[ATA_MASTER].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_SEC_SLAVE : return secondary.drive[ATA_SLAVE].blk.open(path, ATA_SECTOR_SIZE);
            default: return false;
        }

        return false;
    }

    pci_device_t* get_pci_desc() {
        return &pci;
    }

    void init(iobus_ext_t* iobus,
              hyu16_t pri_io_base   = IOBUS_ATA_PRI_IO,
              hyu16_t pri_ctrl_base = IOBUS_ATA_PRI_CTRL,
              hyu16_t sec_io_base   = IOBUS_ATA_SEC_IO,
              hyu16_t sec_ctrl_base = IOBUS_ATA_SEC_CTRL) {
        this->iobus = iobus;

        iobus_pri_io_base   = pri_io_base;
        iobus_pri_ctrl_base = pri_ctrl_base;
        iobus_sec_io_base   = sec_io_base;
        iobus_sec_ctrl_base = sec_ctrl_base;

        // Initialize PCI desc structure
        pci.devid    = 0x01de;  // lol
        pci.vendor   = 0x8086;  // Intel Corp.
        pci.status   = 0x00;
        pci.command  = 0xff;
        pci.devclass = 0x01;    // Mass-storage Device
        pci.subclass = 0x01;    // IDE Controller
        pci.pif      = 0x00;    // PCI Native Interface
        pci.rev      = 0x01;    // Rev. 1
        pci.bist     = 0x00;
        pci.hdr      = 0x00;    // Header Type 0
        pci.lat      = 0x00;
        pci.clsize   = 0x00;
        pci.bar[0]   = PCI_BAR_IO | (iobus_pri_io_base   << 2); // Primary Channel IO
        pci.bar[1]   = PCI_BAR_IO | (iobus_pri_ctrl_base << 2); // Primary Channel CTRL
        pci.bar[2]   = PCI_BAR_IO | (iobus_sec_io_base   << 2); // Secondary Channel IO
        pci.bar[3]   = PCI_BAR_IO | (iobus_sec_ctrl_base << 2); // Secondary Channel CTRL
    }

    // ATA has two buses, a "Primary" bus, and a "Secondary" bus
    // supporting up to two drives each, named "Master" and "Slave"
    void update() {
        int ch = 0;

        // Figure out channel from port
        if ((iobus->port >= iobus_pri_io_base) && (iobus->port <= (iobus_pri_io_base + IOBUS_ATA_IO_SIZE))) {
            ch = ATA_PRIMARY;            
        } else if ((iobus->port >= iobus_sec_io_base) && (iobus->port <= (iobus_sec_io_base + IOBUS_ATA_IO_SIZE))) {
            ch = ATA_SECONDARY;
        }

        ata_channel_t* channel = ch ? &secondary : &primary;
        ata_channel_t::drive_t* drive = channel->drive_number ? &channel->drive[ATA_SLAVE] : &channel->drive[ATA_MASTER];

        switch (iobus->port & 0xf) {
            case ATA_REG_DATA: {

            } break;

            case ATA_REG_ERROR: { // for R, ATA_REG_FEATURES for W

            }

            case ATA_REG_COMMAND: { // for W, ATA_REG_STATUS for R
                switch (iobus->rw) {
                    case RW_READ: { // ATA_REG_STATUS
                    } break;
                }
            } break;
        }
    }
};