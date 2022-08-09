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
#include <iostream>

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

// Status Register
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Error Register
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// ATA Commands
#define ATA_CMD_READ_PIO          0x20 // READ SECTORS
#define ATA_CMD_READ_PIO_EXT      0x24 // READ SECTORS EXT
#define ATA_CMD_READ_DMA          0xC8 // READ DMA
#define ATA_CMD_READ_DMA_EXT      0x25 // READ DMA EXT
#define ATA_CMD_WRITE_PIO         0x30 // WRITE SECTORS
#define ATA_CMD_WRITE_PIO_EXT     0x34 // WRITE SECTORS EXT
#define ATA_CMD_WRITE_DMA         0xCA // WRITE DMA
#define ATA_CMD_WRITE_DMA_EXT     0x35 // WRITE DMA EXT
#define ATA_CMD_CACHE_FLUSH       0xE7 // FLUSH CACHE
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA // FLUSH CACHE EXT
#define ATA_CMD_PACKET            0xA0 // PACKET (ATAPI?)
#define ATA_CMD_IDENTIFY_PACKET   0xA1 // IDENTIFY PACKET DEVICE (ATAPI?)
#define ATA_CMD_IDENTIFY          0xEC // IDENTIFY DEVICE

// #IDENTITY fields
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define ATA_PRIMARY     0
#define ATA_SECONDARY   1
#define ATA_MASTER      0
#define ATA_SLAVE       1

#define ATA_SECTOR_SIZE 0x200

struct ata_channel_t {
    int drive_number = ATA_MASTER;

    struct drive_t {
        block_dev_t blk;                        // Each drive gets a block device for outputting to a file
        hyu64_t     rw_base_lba;                // This is the base LBA for RW ops
        hyu16_t     rw_sectors;                 // Sector count for RW ops
        size_t      rw_pending_bytes;           // Pending RWs from the PIO port
        uint8_t     rw_buf[ATA_SECTOR_SIZE];    // Sector-sized buffer for RW ops
        bool        rw_direction;               // RW op direction (read, write)
        uint8_t     error;                      // Drive command error
        uint8_t     status;                     // Drive status
    } drive[2];
};

#define ATA_PRI_MASTER 0
#define ATA_PRI_SLAVE  1
#define ATA_SEC_MASTER 2
#define ATA_SEC_SLAVE  3

class iobus_dev_ata_t : public iobus_device_t {
    iobus_ext_t* iobus;
    pci_device_t dev;

    // Channel index
    int index = ATA_PRIMARY;
    ata_channel_t channel[2];

    hyu16_t iobus_pri_io_base;
    hyu16_t iobus_pri_ctrl_base;
    hyu16_t iobus_sec_io_base;
    hyu16_t iobus_sec_ctrl_base;

#define ATA_ID_CFG_RESERVED1  0b0000000000000001
#define ATA_ID_CFG_UNUSED3    0b0000000000000010
#define ATA_ID_CFG_INCOMPLETE 0b0000000000000100 // 0 - Complete response, 1 - Incomplete response
#define ATA_ID_CFG_UNUSED2    0b0000000000111000
#define ATA_ID_CFG_FIXED      0b0000000001000000 // 0 - Not fixed device, 1 - Fixed device
#define ATA_ID_CFG_REMOVABLE  0b0000000010000000 // 0 - Non-removable device, 1 Removable device
#define ATA_ID_CFG_UNUSED1    0b0111111100000000
#define ATA_ID_CFG_DEVICETYPE 0b1000000000000000 // 0 - ATA device?, 1 - ???

#define CURRENT_CHANNEL channel[index]
#define CURRENT_DRIVE channel[index].drive[channel[index].drive_number]

    void store_identify_buffer() {
        // struct ata_identify_t {
        //     struct config_t {
        //         uint16_t device_type            : 1;
        //         uint16_t unused1                : 7;
        //         uint16_t removable              : 1;
        //         uint16_t fixed                  : 1;
        //         uint16_t unused2                : 3;
        //         uint16_t response_incomplete    : 1;
        //         uint16_t unused3                : 1;
        //         uint16_t reserved1              : 1;
        //     } config;

        //     uint16_t cylinders;
        //     uint16_t spec;
        //     uint16_t heads;
        //     uint16_t unused1[2];
        //     uint16_t sectors_per_track;
        //     uint16_t vendor_info1[3];
        //     char     serial[20];
        //     uint16_t unused2[2];
        //     uint16_t obsolete1;
        //     char     firmware[8];
        //     char     model[40];
        //     uint8_t  max_blk_xfer;
        //     uint8_t  vendor_info2;

        //     struct trusted_computing_t {
        //         uint16_t supported  : 1;
        //         uint16_t reserved1  : 15;
        //     } trusted_computing;

        //     struct capabilities_t {
        //         uint8_t physical_sector_alignment : 2;
        //         uint8_t reserved1                 : 6;
        //         uint8_t dma_supported             : 1;
        //         uint8_t lba_supported             : 1;
        //         uint8_t iordy_disable             : 1;
        //         uint8_t iordy_supported           : 1;
        //         uint8_t reserved2                 : 1;
        //         uint8_t standby_timer_supported   : 1;
        //         uint8_t reserved3                 : 2;
        //         uint16_t reserved4;
        //     } capabilities;

        //     uint16_t obsolete2[2];
        //     uint16_t translation_fields_valid : 3;
        //     uint16_t reserved3                : 5;
        //     uint16_t ff_control_sensitivity   : 8;
        //     uint16_t current_cylinders;
        //     uint16_t current_heads;
        //     uint16_t current_sectors_per_track;
        //     uint32_t current_sector_capacity;
        //     uint8_t  current_multi_sector_setting;
        //     uint8_t  multi_sector_setting_valid     : 1;
        //     uint8_t  reserved4                      : 3;
        //     uint8_t  sanitize_supported             : 1;
        //     uint8_t  crypto_scramble_ext_supported  : 1;
        //     uint8_t  overwrite_ext_supported        : 1;
        //     uint8_t  blockerase_ext_supported       : 1;
        //     uint32_t user_addressable_sectors;
        //     uint16_t obsolete3;
        //     uint16_t multiword_dma_supported    : 8;
        //     uint16_t multiword_dma_active       : 8;
        //     uint16_t advanced_pio_modes         : 8;
        //     uint16_t reserved5                  : 8;
        //     uint16_t min_mw_transfer_cycle_time;
        //     uint16_t rec_mw_transfer_cycle_time;
        //     uint16_t min_pio_cycle_time;
        //     uint16_t min_pio_cycle_time_iordy;

        //     struct additional_capabilities_t {
        //         uint16_t zoned_capabilities                             : 2;
        //         uint16_t non_volatile_write_cache                       : 1;
        //         uint16_t extended_user_addressable_sectors_supported    : 1;
        //         uint16_t device_encrypts_all_user_data                  : 1;
        //         uint16_t read_zero_after_trim_supported                 : 1;
        //         uint16_t optional_128bit_commands_supported             : 1;
        //         uint16_t ieee1667                                       : 1;
        //         uint16_t download_microcode_dma_supported : 1;
        //         uint16_t set_max_set_password_unlock_dma_supported : 1;
        //         uint16_t write_buffer_dma_supported : 1;
        //         uint16_t read_buffer_dma_supported : 1;
        //         uint16_t device_config_identify_set_dma_supported : 1;
        //         uint16_t LPSAERCsupported : 1;
        //         uint16_t deterministic_read_after_trim_supported : 1;
        //         uint16_t Cfast_spec_supported : 1;
        //     } additional_supported;

        //     uint16_t reserved_words70[5];
        //     uint16_t queue_depth : 5;
        //     uint16_t reserved_word75 : 11;

        //     struct sata_capabilities_t {
        //         uint16_t reserved0 : 1;
        //         uint16_t sata_gen1 : 1;
        //         uint16_t sata_gen2 : 1;
        //         uint16_t sata_gen3 : 1;
        //         uint16_t reserved1 : 4;
        //         uint16_t NCQ : 1;
        //         uint16_t HIPM : 1;
        //         uint16_t phy_events : 1;
        //         uint16_t ncq_unload : 1;
        //         uint16_t ncq_priority : 1;
        //         uint16_t host_autoPS : 1;
        //         uint16_t device_autoPS : 1;
        //         uint16_t read_logDMA : 1;
        //         uint16_t reserved2 : 1;
        //         uint16_t current_speed : 3;
        //         uint16_t ncq_streaming : 1;
        //         uint16_t ncq_queue_mgmt : 1;
        //         uint16_t ncq_receive_send : 1;
        //         uint16_t DEVSLpto_reduced_pwr_state : 1;
        //         uint16_t reserved3 : 8;
        //     } sata_capabilities;

        //     struct sata_features_supported_t {
        //         uint16_t reserved0 : 1;
        //         uint16_t non_zero_offsets : 1;
        //         uint16_t dma_setup_auto_activate : 1;
        //         uint16_t DIPM : 1;
        //         uint16_t in_order_data : 1;
        //         uint16_t hardware_feature_control : 1;
        //         uint16_t software_settings_preservation : 1;
        //         uint16_t NCQautosense : 1;
        //         uint16_t DEVSLP : 1;
        //         uint16_t hybrid_information : 1;
        //         uint16_t reserved1 : 6;
        //     } sata_features_supported;
    
        //     struct sata_features_enabled_t {
        //         uint16_t reserved0 : 1;
        //         uint16_t non_zero_offsets : 1;
        //         uint16_t dma_setup_auto_activate : 1;
        //         uint16_t DIPM : 1;
        //         uint16_t in_order_data : 1;
        //         uint16_t hardware_feature_control : 1;
        //         uint16_t software_settings_preservation : 1;
        //         uint16_t device_autoPS : 1;
        //         uint16_t DEVSLP : 1;
        //         uint16_t hybrid_information : 1;
        //         uint16_t reserved1 : 6;
        //     } sata_features_enabled;

        //     uint16_t major_revision;
        //     uint16_t minor_revision;

        //     struct command_set_support_t {
        //         uint16_t smart_commands : 1;
        //         uint16_t security_mode : 1;
        //         uint16_t removable_media_feature : 1;
        //         uint16_t power_management : 1;
        //         uint16_t reserved1 : 1;
        //         uint16_t write_cache : 1;
        //         uint16_t look_ahead : 1;
        //         uint16_t release_interrupt : 1;
        //         uint16_t service_interrupt : 1;
        //         uint16_t device_reset : 1;
        //         uint16_t host_protected_area : 1;
        //         uint16_t obsolete1 : 1;
        //         uint16_t write_buffer : 1;
        //         uint16_t read_buffer : 1;
        //         uint16_t nop : 1;
        //         uint16_t obsolete2 : 1;
        //         uint16_t download_microcode : 1;
        //         uint16_t dma_queued : 1;
        //         uint16_t cfa : 1;
        //         uint16_t advanced_pm : 1;
        //         uint16_t msn : 1;
        //         uint16_t power_up_in_standby : 1;
        //         uint16_t manual_power_up : 1;
        //         uint16_t reserved2 : 1;
        //         uint16_t set_max : 1;
        //         uint16_t acoustics : 1;
        //         uint16_t big_lba : 1;
        //         uint16_t device_config_overlay : 1;
        //         uint16_t flush_cache : 1;
        //         uint16_t flush_cache_ext : 1;
        //         uint16_t word_valid83 : 2;
        //         uint16_t smart_error_log : 1;
        //         uint16_t smart_self_test : 1;
        //         uint16_t media_serial_number : 1;
        //         uint16_t media_card_pass_through : 1;
        //         uint16_t streaming_feature : 1;
        //         uint16_t gp_logging : 1;
        //         uint16_t write_fua : 1;
        //         uint16_t write_queued_fua : 1;
        //         uint16_t WWN64bit : 1;
        //         uint16_t URGread_stream : 1;
        //         uint16_t URGwrite_stream : 1;
        //         uint16_t reserved_for_tech_report : 2;
        //         uint16_t idle_with_unload_feature : 1;
        //         uint16_t word_valid : 2;
        //     } command_set_support;

        //     struct command_set_active_t {
        //         uint16_t smart_commands : 1;
        //         uint16_t security_mode : 1;
        //         uint16_t removable_media_feature : 1;
        //         uint16_t power_management : 1;
        //         uint16_t reserved1 : 1;
        //         uint16_t write_cache : 1;
        //         uint16_t look_ahead : 1;
        //         uint16_t release_interrupt : 1;
        //         uint16_t service_interrupt : 1;
        //         uint16_t device_reset : 1;
        //         uint16_t host_protected_area : 1;
        //         uint16_t obsolete1 : 1;
        //         uint16_t write_buffer : 1;
        //         uint16_t read_buffer : 1;
        //         uint16_t nop : 1;
        //         uint16_t obsolete2 : 1;
        //         uint16_t download_microcode : 1;
        //         uint16_t dma_queued : 1;
        //         uint16_t cfa : 1;
        //         uint16_t advanced_pm : 1;
        //         uint16_t msn : 1;
        //         uint16_t power_up_in_standby : 1;
        //         uint16_t manual_power_up : 1;
        //         uint16_t reserved2 : 1;
        //         uint16_t set_max : 1;
        //         uint16_t acoustics : 1;
        //         uint16_t big_lba : 1;
        //         uint16_t device_config_overlay : 1;
        //         uint16_t flush_cache : 1;
        //         uint16_t flush_cache_ext : 1;
        //         uint16_t resrved3 : 1;
        //         uint16_t words119_120valid : 1;
        //         uint16_t smart_error_log : 1;
        //         uint16_t smart_self_test : 1;
        //         uint16_t media_serial_number : 1;
        //         uint16_t media_card_pass_through : 1;
        //         uint16_t streaming_feature : 1;
        //         uint16_t gp_logging : 1;
        //         uint16_t write_fua : 1;
        //         uint16_t write_queued_fua : 1;
        //         uint16_t WWN64bit : 1;
        //         uint16_t URGread_stream : 1;
        //         uint16_t URGwrite_stream : 1;
        //         uint16_t reserved_for_tech_report : 2;
        //         uint16_t idle_with_unload_feature : 1;
        //         uint16_t reserved4 : 2;
        //     } command_set_active;

        //     uint16_t udma_support : 8;
        //     uint16_t udma_active : 8;

        //     struct normal_security_erase_unit_t {
        //         uint16_t time_required : 15;
        //         uint16_t extended_time_reported : 1;
        //     } normal_security_erase_unit;
    
        //     struct enhanced_security_erase_unit_t {
        //         uint16_t time_required : 15;
        //         uint16_t extended_time_reported : 1;
        //     } enhanced_security_erase_unit;

        //     uint16_t currentAPM_level : 8;
        //     uint16_t reserved_word91 : 8;
        //     uint16_t master_passwordID;
        //     uint16_t hardware_reset_result;
        //     uint16_t current_acoustic_value : 8;
        //     uint16_t recommended_acoustic_value : 8;
        //     uint16_t stream_min_request_size;
        //     uint16_t streaming_transfer_timeDMA;
        //     uint16_t streaming_access_latencyDMAPIO;
        //     uint32_t streaming_perf_granularity;
        //     uint32_t max48bitLBA[2];
        //     uint16_t streaming_transfer_time;
        //     uint16_t dsm_cap;

        //     struct physical_logical_sector_size_t {
        //         uint16_t logical_sectors_per_physical_sector : 4;
        //         uint16_t reserved0 : 8;
        //         uint16_t logical_sector_longer_than256words : 1;
        //         uint16_t multiple_logical_sectors_per_physical_sector : 1;
        //         uint16_t reserved1 : 2;
        //     } physical_logical_sector_size;

        //     uint16_t inter_seek_delay;
        //     uint16_t world_wide_name[4];
        //     uint16_t reserved_for_world_wide_name128[4];
        //     uint16_t reserved_for_tlc_technical_report;
        //     uint16_t words_per_logical_sector[2];

        //     struct command_set_support_ext_t {
        //         uint16_t reserved_for_drq_technical_report : 1;
        //         uint16_t write_read_verify : 1;
        //         uint16_t write_uncorrectable_ext : 1;
        //         uint16_t read_write_log_dma_ext : 1;
        //         uint16_t download_microcode_mode3 : 1;
        //         uint16_t freefall_control : 1;
        //         uint16_t sense_data_reporting : 1;
        //         uint16_t extended_power_conditions : 1;
        //         uint16_t reserved0 : 6;
        //         uint16_t word_valid : 2;
        //     } command_set_support_ext;

        //     struct command_set_active_ext_t {
        //         uint16_t reserved_for_drq_technical_report : 1;
        //         uint16_t write_read_verify : 1;
        //         uint16_t write_uncorrectable_ext : 1;
        //         uint16_t read_write_log_dma_ext : 1;
        //         uint16_t download_microcode_mode3 : 1;
        //         uint16_t freefall_control : 1;
        //         uint16_t sense_data_reporting : 1;
        //         uint16_t extended_power_conditions : 1;
        //         uint16_t reserved0 : 6;
        //         uint16_t reserved1 : 2;
        //     } command_set_active_ext;

        //     uint16_t reserved_for_expanded_supportand_active[6];
        //     uint16_t msn_support : 2;
        //     uint16_t reserved_word127 : 14;

        //     struct security_status_t {
        //         uint16_t security_supported : 1;
        //         uint16_t security_enabled : 1;
        //         uint16_t security_locked : 1;
        //         uint16_t security_frozen : 1;
        //         uint16_t security_count_expired : 1;
        //         uint16_t enhanced_security_erase_supported : 1;
        //         uint16_t reserved0 : 2;
        //         uint16_t security_level : 1;
        //         uint16_t reserved1 : 7;
        //     } security_status;

        //     uint16_t reserved_word129[31];

        //     struct cfa_power_mode1_t {
        //         uint16_t maximum_current_inMA : 12;
        //         uint16_t cfa_power_mode1disabled : 1;
        //         uint16_t cfa_power_mode1required : 1;
        //         uint16_t reserved0 : 1;
        //         uint16_t word160supported : 1;
        //     } cfa_power_mode1;

        //     uint16_t reserved_for_cfa_word161[7];
        //     uint16_t nominal_form_factor : 4;
        //     uint16_t reserved_word168 : 12;

        //     struct data_set_management_feature_t {
        //         uint16_t supports_trim : 1;
        //         uint16_t reserved0 : 15;
        //     } data_set_management_feature;

        //     uint16_t additional_productID[4];
        //     uint16_t reserved_for_cfa_word174[2];
        //     uint16_t current_media_serial_number[30];

        //     struct SCTcommand_transport_t {
        //         uint16_t supported : 1;
        //         uint16_t reserved0 : 1;
        //         uint16_t write_same_suported : 1;
        //         uint16_t error_recovery_control_supported : 1;
        //         uint16_t feature_control_suported : 1;
        //         uint16_t data_tables_suported : 1;
        //         uint16_t reserved1 : 6;
        //         uint16_t vendor_specific : 4;
        //     } SCTcommand_transport;

        //     uint16_t reserved_word207[2];

        //     struct block_alignment_t {
        //         uint16_t alignment_of_logical_within_physical : 14;
        //         uint16_t word209supported : 1;
        //         uint16_t reserved0 : 1;
        //     } block_alignment;

        //     uint16_t write_read_verify_sector_count_mode3only[2];
        //     uint16_t write_read_verify_sector_count_mode2only[2];

        //     struct NVcache_capabilities_t {
        //         uint16_t NVcache_power_mode_enabled : 1;
        //         uint16_t reserved0 : 3;
        //         uint16_t NVcache_feature_set_enabled : 1;
        //         uint16_t reserved1 : 3;
        //         uint16_t NVcache_power_mode_version : 4;
        //         uint16_t NVcache_feature_set_version : 4;
        //     } NVcache_capabilities;

        //     uint16_t NVcache_sizeLSW;
        //     uint16_t NVcache_sizeMSW;
        //     uint16_t nominal_media_rotation_rate;
        //     uint16_t reserved_word218;

        //     struct NVcache_options_t {
        //         uint8_t NVcache_estimated_time_to_spin_up_in_seconds;
        //         uint8_t reserved;
        //     } NVcache_options;

        //     uint16_t write_read_verify_sector_count_mode : 8;
        //     uint16_t reserved_word220 : 8;
        //     uint16_t reserved_word221;

        //     struct transport_major_version_t {
        //         uint16_t major_version : 12;
        //         uint16_t transport_type : 4;
        //     } transport_major_version;

        //     uint16_t transport_minor_version;
        //     uint16_t reserved_word224[6];
        //     uint32_t extended_number_of_user_addressable_sectors[2];
        //     uint16_t min_blocks_per_download_microcode_mode03;
        //     uint16_t max_blocks_per_download_microcode_mode03;
        //     uint16_t reserved_word236[17];
        //     uint16_t signature : 8;
        //     uint16_t checksum : 8;
        // } ata_identify_buf;

        // std::memset(&ata_identify_buf, 0, sizeof(ata_identify_t));

        // ata_identify_buf.config.fixed       = true;
        // ata_identify_buf.cylinders          = 0xffff;
        // ata_identify_buf.heads              = 16;
        // ata_identify_buf.sectors_per_track  = 63;

        // Generate serial
        // for (int i = 0; i < 20; i++)
        //     ata_identify_buf.serial[i] = "0123456789ABCDEF"[rand() % 16];
        
        // std::strcpy(ata_identify_buf.firmware, "hyvmidec");
        // std::strcpy(ata_identify_buf.model, "WDC WD4005FZBX-00K5WB0");

        uint16_t id_buf[ATA_SECTOR_SIZE / 2];

        id_buf[0] = ATA_ID_CFG_FIXED; // Fixed, non-removable, ATA device
        id_buf[1] = 65535; // logical cylinders
        id_buf[3] = 16; // logical heads
        id_buf[6] = 63; // sectors per track
        id_buf[22] = 4; // number of bytes available in READ/WRITE LONG cmds
        id_buf[47] = 0; // read-write multipe commands not implemented
        id_buf[49] = (1 << 9); // Capabilities - LBA supported, DMA supported
        id_buf[50] = (1 << 14); // Capabilities - bit 14 needs to be set as required by ATA/ATAPI-5 spec
        id_buf[51] = (4 << 8); // PIO data transfer cycle timing mode
        id_buf[53] = 1 | 2 | 4; // fields 54-58, 64-70 and 88 are valid
        id_buf[54] = 65535; // logical cylinders
        id_buf[55] = 16; // logical heads
        id_buf[56] = 63; // sectors per track
        id_buf[57] = 0xffff;
        id_buf[58] = 0xffff;
        id_buf[60] = 0xffff;
        id_buf[61] = 0xffff;
        id_buf[64] = 0; // advanced PIO modes not supported
        id_buf[67] = 1; // PIO transfer cycle time without flow control
        id_buf[68] = 1; // PIO transfer cycle time with IORDY flow control
        id_buf[80] = 1 << 6; // ATA major version
        id_buf[88] = 0; // UDMA mode 5 not supported

        char serial[20];

        // Generate serial
        for (int i = 0; i < 20; i++)
            serial[i] = "0123456789ABCDEF"[rand() % 16];
        
        serial[20] = 0;
        
        std::memcpy(&id_buf[10], serial, 20);
        std::memcpy(&id_buf[23], "hyvmidec", 8);
        std::memcpy(&id_buf[27], "WDC WD4005FZBX-00K5WB0\0                ", 40);

        std::memcpy(CURRENT_DRIVE.rw_buf, id_buf, ATA_SECTOR_SIZE);

        CURRENT_DRIVE.rw_pending_bytes = ATA_SECTOR_SIZE;
        CURRENT_DRIVE.rw_direction     = false;
    }

    bool ata_check_access_and_channel() {
        // Figure out channel from port
        if ((iobus->port >= iobus_pri_io_base) && (iobus->port <= (iobus_pri_io_base + IOBUS_ATA_IO_SIZE))) {
            index = ATA_PRIMARY;            

            return true;
        } else if ((iobus->port >= iobus_sec_io_base) && (iobus->port <= (iobus_sec_io_base + IOBUS_ATA_IO_SIZE))) {
            index = ATA_SECONDARY;

            return true;
        }

        return false;
    }

    void ata_io_handle_hddevsel() {
        if (iobus->rw == RW_READ) {
            iobus->data = 0xe0 | (CURRENT_CHANNEL.drive_number << 4);

            return;
        } else {
            CURRENT_CHANNEL.drive_number = (iobus->data >> 4) & 0x1;

            return;
        }
    }

    void ata_io_handle_command() {
        if (iobus->rw == RW_READ) {
            iobus->data = CURRENT_DRIVE.status;

            return;
        } else {
            switch (iobus->data) {
                case ATA_CMD_READ_PIO: {

                } break;

                case ATA_CMD_IDENTIFY: {
                    if (!CURRENT_DRIVE.blk.is_open()) {
                        // No drive here
                        CURRENT_DRIVE.status = 0x0;

                        break;
                    }

                    // Immediately set status to ready and request available
                    CURRENT_DRIVE.status = ATA_SR_DRDY | ATA_SR_DRQ;

                    // Identify an ATA drive through LBA1 and LBA2
                    CURRENT_DRIVE.rw_base_lba = 0x0;
                    //CURRENT_DRIVE.rw_base_lba = 0x0;

                    // Clear error
                    CURRENT_DRIVE.error = 0x0;

                    store_identify_buffer();
                }
            }
        }
    }

    void ata_io_handle_data() {
        if (!CURRENT_DRIVE.rw_pending_bytes) return;


        switch (iobus->rw) {
            case RW_READ: {
                if (CURRENT_DRIVE.rw_direction != RW_READ) {
                    // Attempted reading from ATA_REG_DATA during a write
                    // operation

                    return;
                }
                
                int rw_index = ATA_SECTOR_SIZE - CURRENT_DRIVE.rw_pending_bytes;

                switch (iobus->size) {
                    case 0: {
                        iobus->data = CURRENT_DRIVE.rw_buf[rw_index];

                        CURRENT_DRIVE.rw_pending_bytes -= 1;
                    } break;

                    case 1: {
                        iobus->data = *(uint16_t*)(&CURRENT_DRIVE.rw_buf[rw_index]);

                        CURRENT_DRIVE.rw_pending_bytes -= 2;
                    } break;

                    case 2: default: {
                        iobus->data = *(uint32_t*)(&CURRENT_DRIVE.rw_buf[rw_index]);

                        CURRENT_DRIVE.rw_pending_bytes -= 4;
                    }
                }
            } break;

            case RW_WRITE: {
                // To-do
            } break;
        }
    }

public:
    bool attach_drive(const std::string& path, int attachment) {
        switch (attachment) {
            case ATA_PRI_MASTER: return channel[ATA_PRIMARY].drive[ATA_MASTER].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_PRI_SLAVE : return channel[ATA_PRIMARY].drive[ATA_SLAVE].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_SEC_MASTER: return channel[ATA_SECONDARY].drive[ATA_MASTER].blk.open(path, ATA_SECTOR_SIZE);
            case ATA_SEC_SLAVE : return channel[ATA_SECONDARY].drive[ATA_SLAVE].blk.open(path, ATA_SECTOR_SIZE);
            default: return false;
        }

        return false;
    }

    pci_device_t* get_pci_desc() {
        return &dev;
    }

    void redefine_ports(
        hyu16_t pri_io_base   = IOBUS_ATA_PRI_IO,
        hyu16_t pri_ctrl_base = IOBUS_ATA_PRI_CTRL,
        hyu16_t sec_io_base   = IOBUS_ATA_SEC_IO,
        hyu16_t sec_ctrl_base = IOBUS_ATA_SEC_CTRL) {
        
        iobus_pri_io_base   = pri_io_base;
        iobus_pri_ctrl_base = pri_ctrl_base;
        iobus_sec_io_base   = sec_io_base;
        iobus_sec_ctrl_base = sec_ctrl_base;

        // Update PCI BAR data
        dev.bar[0] = PCI_BAR_IO | (iobus_pri_io_base   << 2); // Primary Channel IO
        dev.bar[1] = PCI_BAR_IO | (iobus_pri_ctrl_base << 2); // Primary Channel CTRL
        dev.bar[2] = PCI_BAR_IO | (iobus_sec_io_base   << 2); // Secondary Channel IO
        dev.bar[3] = PCI_BAR_IO | (iobus_sec_ctrl_base << 2); // Secondary Channel CTRL
    }

    void init(iobus_ext_t* iobus) override {
        this->iobus = iobus;

        iobus_pri_io_base   = IOBUS_ATA_PRI_IO;
        iobus_pri_ctrl_base = IOBUS_ATA_PRI_CTRL;
        iobus_sec_io_base   = IOBUS_ATA_SEC_IO;
        iobus_sec_ctrl_base = IOBUS_ATA_SEC_CTRL;

        // Initialize PCI desc structure
        dev.devid    = 0x01de;  // lol
        dev.vendor   = 0x8086;  // Intel Corp.
        dev.status   = 0x00;
        dev.command  = 0xff;
        dev.devclass = 0x01;    // Mass-storage Device
        dev.subclass = 0x01;    // IDE Controller
        dev.pif      = 0x00;    // PCI Native Interface
        dev.rev      = 0x01;    // Rev. 1
        dev.bist     = 0x00;
        dev.hdr      = 0x00;    // Header Type 0
        dev.lat      = 0x00;
        dev.clsize   = 0x00;
        dev.bar[0]   = PCI_BAR_IO | (iobus_pri_io_base   << 2); // Primary Channel IO
        dev.bar[1]   = PCI_BAR_IO | (iobus_pri_ctrl_base << 2); // Primary Channel CTRL
        dev.bar[2]   = PCI_BAR_IO | (iobus_sec_io_base   << 2); // Secondary Channel IO
        dev.bar[3]   = PCI_BAR_IO | (iobus_sec_ctrl_base << 2); // Secondary Channel CTRL
    }

    // ATA has two buses, a "Primary" bus, and a "Secondary" bus
    // supporting up to two drives each, named "Master" and "Slave"
    void update() override {
        if (!ata_check_access_and_channel()) return;

        switch (iobus->port & 0xf) {
            case ATA_REG_DATA: {
                ata_io_handle_data();
            } break;

            case ATA_REG_ERROR: { // for R, ATA_REG_FEATURES for W

            }

            case ATA_REG_HDDEVSEL: {
                ata_io_handle_hddevsel();
            } break;

            case ATA_REG_COMMAND: { // for W, ATA_REG_STATUS for R
                ata_io_handle_command();
            } break;
        }
    }
};