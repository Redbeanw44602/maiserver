// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstring>
#include <string_view>

namespace mai::mem {

namespace detail::llvm {

template <typename ST, typename LT>
union StringBase {
    ST s; // data, size, is_long
    LT l; // data, size, cap, is_long

    StringBase() {
        s.is_long = 0;
        s.size    = 0;
        s.data[0] = '\0';
    }

    StringBase(std::string_view sv) {
        if (sv.size() < sizeof(s.data)) {
            s.is_long = 0;
            s.size    = static_cast<decltype(s.size)>(sv.size());
            if (sv.size() > 0) {
                std::memcpy(s.data, sv.data(), sv.size());
            }
            s.data[sv.size()] = '\0';
        } else {
            l.is_long      = 1;
            l.size         = sv.size();
            l.cap          = sv.size();
            auto heap_data = new char[sv.size() + 1];
            std::memcpy(heap_data, sv.data(), sv.size());
            heap_data[sv.size()] = '\0';
            l.data               = heap_data;
        }
    }

    ~StringBase() {
        if (s.is_long) {
            delete[] (l.data);
        }
    }

    StringBase(const StringBase&)            = delete; // todo
    StringBase& operator=(const StringBase&) = delete;

    StringBase(StringBase&& other) {
        if (other.s.is_long) {
            l.is_long       = 1;
            l.size          = other.l.size;
            l.cap           = other.l.cap;
            l.data          = other.l.data;
            other.s.is_long = 0;
            other.s.size    = 0;
            other.s.data[0] = '\0';
        } else {
            s.is_long = 0;
            s.size    = other.s.size;
            std::memcpy(s.data, other.s.data, other.s.size + 1);
        }
    }

    std::string_view view() const {
        if (s.is_long) {
            return {l.data, l.size};
        } else {
            return {s.data, s.size};
        }
    }
};

// libc++ std::string (alternate string layout)
// https://github.com/llvm/llvm-project/blob/055a77bb291a8df8a8be1084472984e80cf203b5/libcxx/include/string#L793

struct StringShort {
    char          data[23];
    unsigned char size    : 7;
    unsigned char is_long : 1;
};

struct StringLong {
    char*  data;
    size_t size;
    size_t cap     : 63;
    size_t is_long : 1;
};

// libc++ std::string (NA = not alternate string layout)

struct StringShortNA {
    unsigned char is_long : 1;
    unsigned char size    : 7;
    char          data[23];
};

struct StringLongNA {
    size_t is_long : 1;
    size_t cap     : 63;
    size_t size;
    char*  data;
};

using String   = StringBase<StringShort, StringLong>;
using StringNA = StringBase<StringShortNA, StringLongNA>;

static_assert(sizeof(String) == 24);
static_assert(sizeof(StringNA) == 24);

} // namespace detail::llvm

namespace detail::gnu {

struct String {
    const char* data;
    size_t      size;
    union {
        char   buf[16]; // SSO
        size_t cap;
    };

    std::string_view view() const { return {data, size}; }
};

} // namespace detail::gnu

using LLVMString   = detail::llvm::String;
using LLVMStringNA = detail::llvm::StringNA;

using GNUString = detail::gnu::String;

} // namespace mai::mem
