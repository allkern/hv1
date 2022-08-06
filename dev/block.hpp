#pragma once

#include <fstream>
#include <string>
#include <cmath>

struct block_dev_t {
    std::fstream file;

    size_t sector_size;
    size_t sectors;
    size_t bytes;

    bool open(std::string path, size_t sector_size) {
        file.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

        this->sector_size = sector_size;

        if ((!file.is_open()) || (!file.good())) {
            // _log(error, "Couldn't open file %s (%u, %u)", path.c_str(), file.is_open(), file.good());

            return false;
        }

        bytes = file.tellg();
        sectors = bytes / sector_size;

        file.seekg(0);
        
        return true;
    }

    size_t size() const { return sectors; }

    void read(int sector, int count, void* data) {
        file.seekg(sector * sector_size);
        file.read((char*)data, count * sector_size);
    }

    void write(int sector, void* data, size_t size) {
        file.seekg(sector * sector_size);
        file.write((char*)data, size);

        // Align back to sector boundary
        size_t write_sectors = std::ceil((double)size / sector_size);

        file.seekg((sector + write_sectors) * sector_size);
    }
};