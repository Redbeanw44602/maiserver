// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <print>
#include <source_location>

#include "util/string.h"

namespace mai::detail {

template <typename... Args>
void print_debug_string(
    std::source_location        location,
    std::string_view            function_name,
    std::format_string<Args...> fmt,
    Args&&... args
) {
    using namespace util::string;
    std::println(
        "[maiserver:debug] {}:{} `{}`: {}",
        remove_prefix_r(location.file_name(), "../src/"),
        location.line(),
        function_name,
        std::format(fmt, std::forward<Args>(args)...)
    );
}

} // namespace mai::detail

#if MAI_DEBUG
#define DBG(...)                                                               \
    mai::detail::print_debug_string(                                           \
        std::source_location::current(),                                       \
        __FUNCTION__,                                                          \
        __VA_ARGS__                                                            \
    )
#else
#define DBG(...) ((void)0)
#endif
