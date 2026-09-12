// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "mem/hook.h"
#include "mem/module.h"

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
        const auto UpdateU64 = CALLABLE_ADDR(
            void*,
            libilink2::rel(0x557000),
            void*,
            uint32_t,
            uint64_t
        );
        UpdateU64(object_, Key, value);
    }

    template <uint32_t Key>
    void set_string(std::string_view value) {
        const auto UpdateStdString = CALLABLE_ADDR(
            void*,
            libilink2::rel(0x557370),
            void*,
            uint32_t,
            std::string const& // // todo: Use GNUString
        );
        UpdateStdString(object_, Key, std::string(value));
    }

    uint32_t size() const { return *((uint32_t*)object_ + 2); }

    uint64_t deserialize(std::span<char> in) {
        const auto Deserialize = CALLABLE_ADDR(
            uint64_t,
            libilink2::rel(0x557CA0),
            void*,
            char*,
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
