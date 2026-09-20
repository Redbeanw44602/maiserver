// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string>

#include "error.h"
#include "util/string.h"

namespace mai::util::env {

template <typename T>
constexpr std::expected<T, MaiError> get(string::Literal name)
    requires std::same_as<T, std::string>
{
    if (auto value = std::getenv(name.c_str())) {
        return value;
    }
    return std::unexpected(MaiError::ENVIRONMENT_VARIABLE_NOT_FOUND);
}

template <typename T>
constexpr std::expected<T, MaiError> get(string::Literal name)
    requires std::is_integral_v<T>
          && (!std::is_same_v<std::remove_cv_t<T>, bool>)
{
    if (auto val = get<std::string>(name)) {
        return string::to_integer<T>(*val);
    } else {
        return std::unexpected(val.error());
    }
}

template <typename T>
constexpr std::expected<T, MaiError> get(string::Literal name)
    requires std::is_same_v<std::remove_cv_t<T>, bool>
{
    return get<uint8_t>(name);
}

} // namespace mai::util::env
