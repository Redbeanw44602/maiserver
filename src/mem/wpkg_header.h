// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "mem/function.h"
#include "mem/std_string.h"

namespace mai::mem {

class WPKGHeader {
public:
    explicit WPKGHeader(uint32_t unk_1 = 1) {
        wpkg_header_ctor(object_, unk_1);
    }

    ~WPKGHeader() { wpkg_header_dtor(object_); }

    WPKGHeader(const WPKGHeader&)             = delete;
    WPKGHeader& operator=(const WPKGHeader&)  = delete;
    WPKGHeader(const WPKGHeader&&)            = delete;
    WPKGHeader& operator=(const WPKGHeader&&) = delete;

    template <uint32_t Key>
    void set_uint64(uint64_t value) {
        wpkg_header_set_u64(object_, Key, value);
    }

    template <uint32_t Key>
    void set_string(std::string_view value) {
        wpkg_header_set_string(object_, Key, value);
    }

#if MAI_DEBUG
    template <uint32_t Key>
    std::optional<uint64_t> get_uint64() const {
        uint64_t out;
        return wpkg_header_get_u64(object_, Key, &out) == 0
                 ? std::make_optional(out)
                 : std::nullopt;
    }

    template <uint32_t Key>
    std::optional<std::string> get_string() const {
        LLVMStringNA out;
        return wpkg_header_get_string(object_, Key, &out) == 0
                 ? std::make_optional(std::string(out.view()))
                 : std::nullopt;
    }
#endif

    uint32_t size() const {
        /* How to determine this offset:
         * At 0x555A70, locate the LZ4 decompression function; the second
         * parameter is the length of the input data. Check the cross-references
         * and observe the subtraction operations performed on the length
         * variable: one subtrahend is the length of the `shortconn` header, and
         * the other is obtained via this offset. */
        return *((uint32_t*)object_ + 2);
    }

    uint64_t deserialize(std::span<const char> in) {
        return wpkg_header_deserialize(object_, in.data(), in.size());
    }

    uint32_t serialize(std::span<char> out) const {
        uint32_t out_size{0};
        wpkg_header_serialize(object_, out.data(), &out_size, out.size());
        return out_size;
    }

private:
    char object_[128] = {0};
};

} // namespace mai::mem
