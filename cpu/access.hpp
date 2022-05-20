#pragma once

#include "types.hpp"

namespace cpu {
    enum access_error_t : u8 {
        A32_OK,
        A32_ENOENT,
        A32_EIO,
        A32_EACCESS,
        A32_EFAULT,
        A32_EBUSY
    };

    enum access_intent_t : u8 {
        A32_IREAD,
        A32_IWRITE,
        A32_IEXEC
    };

    struct access_result_t {
        u32 value;
        u8 error;
    };

    typedef access_result_t (*cpu_read_callback_t)(u32, int);
    typedef access_result_t (*cpu_write_callback_t)(u32, u32, int);

    cpu_read_callback_t read_cb;
    cpu_write_callback_t write_cb;
}

