// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <format>
#include <span>
#include <string>
#include <string_view>

namespace mai::util::string {

template <size_t N>
struct Fixed {
    char buf[N]{};

    consteval Fixed(const char (&str)[N]) {
        if (N <= 0 || str[N - 1] != '\0') {
            throw "String literals must be zero-terminated!";
        }
        std::copy_n(str, N, buf);
    }

    consteval operator std::string_view() const { return {buf, N - 1}; }

    consteval const char* c_str() const { return buf; }
    consteval char*       data() { return buf; }
    consteval size_t      size() { return N; }
};

inline std::string_view
middle(std::string_view text, std::string_view left, std::string_view right) {
    const auto begin = text.find(left);
    if (begin == std::string_view::npos) return {};

    const auto start = begin + left.size();
    const auto end   = text.find(right, start);
    if (end == std::string_view::npos) return {};

    return text.substr(start, end - start);
}

inline std::string_view right(std::string_view text, std::string_view left) {
    const auto begin = text.find(left);
    if (begin == std::string_view::npos) return {};

    return text.substr(begin + left.size());
}

inline std::string hex(std::span<const char> sv) {
    std::string ret;
    ret.reserve(sv.size() * 2);
    for (char c : sv) {
        ret += std::format("{:02x}", static_cast<unsigned char>(c));
    }
    return ret;
}

} // namespace mai::util::string
