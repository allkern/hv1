#pragma once

#include <fstream>
#include <string>
#include <cmath>

struct block_dev_t {
    std::fstream m_file;

    size_t m_sector_size;
    size_t m_sectors;
    size_t m_bytes;

    bool m_open = false;

    bool open(std::string path, size_t m_sector_size) {
        m_file.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

        this->m_sector_size = m_sector_size;

        if ((!m_file.is_open()) || (!m_file.good())) {
            // _log(error, "Couldn't open file %s (%u, %u)", path.c_str(), file.is_open(), file.good());

            m_open = false;

            return false;
        }

        m_bytes = m_file.tellg();
        m_sectors = m_bytes / m_sector_size;

        m_file.seekg(0);
        
        m_open = true;

        return true;
    }

    bool is_open() {
        return m_open;
    }

    size_t size() const { return m_sectors; }

    void read(int sector, int count, void* data) {
        m_file.seekg(sector * m_sector_size);
        m_file.read((char*)data, count * m_sector_size);
    }

    void write(int sector, void* data, size_t size) {
        m_file.seekg(sector * m_sector_size);
        m_file.write((char*)data, size);

        // Align back to sector boundary
        size_t write_m_sectors = std::ceil((double)size / m_sector_size);

        m_file.seekg((sector + write_m_sectors) * m_sector_size);
    }
};