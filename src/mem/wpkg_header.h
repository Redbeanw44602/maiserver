// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "mem/hook.h"
#include "mem/module.h"
#include "mem/std_string.h"

namespace mai::mem {

class WPKGHeader {
public:
    explicit WPKGHeader(uint32_t unk_1 = 1) {
        const auto Ctor =
            CALLABLE_ADDR(void*, libilink2::rel(0x556F60), void*, uint32_t);
        Ctor(object_, unk_1);
    }

    ~WPKGHeader() {
        const auto Dtor = CALLABLE_ADDR(void*, libilink2::rel(0x556F90), void*);
        Dtor(object_);
    }

    WPKGHeader(const WPKGHeader&)             = delete;
    WPKGHeader& operator=(const WPKGHeader&)  = delete;
    WPKGHeader(const WPKGHeader&&)            = delete;
    WPKGHeader& operator=(const WPKGHeader&&) = delete;

    template <uint32_t Key>
    void set_uint64(uint64_t value) {
        const auto SetU64 = CALLABLE_ADDR(
            void*,
            libilink2::rel(0x557000),
            void*,
            uint32_t,
            uint64_t
        );
        SetU64(object_, Key, value);
    }

    template <uint32_t Key>
    void set_string(std::string_view value) {
        const auto SetStdString = CALLABLE_ADDR(
            void*,
            libilink2::rel(0x557370),
            void*,
            uint32_t,
            LLVMStringNA const&
        );
        SetStdString(object_, Key, value);
    }

#if MAI_DEBUG
    template <uint32_t Key>
    std::optional<uint64_t> get_uint64() const {
        const auto GetU64 = CALLABLE_ADDR(
            int,
            libilink2::rel(0x557450),
            const void*,
            uint32_t,
            uint64_t*
        );
        uint64_t out;
        return GetU64(object_, Key, &out) == 0 ? std::make_optional(out)
                                               : std::nullopt;
    }

    template <uint32_t Key>
    std::optional<std::string> get_string() const {
        const auto GetStdString = CALLABLE_ADDR(
            int,
            libilink2::rel(0x5574A0),
            const void*,
            uint32_t,
            LLVMStringNA*
        );
        LLVMStringNA out;
        return GetStdString(object_, Key, &out) == 0
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
        const auto Deserialize = CALLABLE_ADDR(
            uint64_t,
            libilink2::rel(0x557CA0),
            void*,
            const char*,
            uint32_t
        );
        return Deserialize(object_, in.data(), in.size());
    }

    uint32_t serialize(std::span<char> out) const {
        const auto Serialize = CALLABLE_ADDR(
            void*,
            libilink2::rel(0x557C80),
            const void*,
            char*,
            uint32_t*,
            uint32_t
        );
        uint32_t out_size{0};
        Serialize(object_, out.data(), &out_size, out.size());
        return out_size;
    }

private:
    char object_[128] = {0};
};

} // namespace mai::mem
