// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string>

#include <libbase64.h>

#include "error.h"

namespace mai::util::base64 {

inline std::string encode(std::string_view data) {
    std::string out(4 * (data.size() + 2 /*ceil*/) / 3, '\0');
    size_t      outlen{};
    base64_encode(data.data(), data.size(), out.data(), &outlen, 0);
    out.resize(outlen);
    return out;
}

inline std::expected<std::string, MaiError> decode(std::string_view data) {
    std::string out(3 * (data.size() / 4), '\0');
    size_t      outlen;
    auto        result =
        base64_decode(data.data(), data.size(), out.data(), &outlen, 0);
    if (result != 1) {
        if (result == -1) {
            return std::unexpected(
                MaiError::FAILED_TO_DECODE_BASE64_DECODER_NOT_FOUND
            );
        }
        return std::unexpected(MaiError::FAILED_TO_DECODE_BASE64_INVALID_INPUT);
    }
    out.resize(outlen);
    return out;
}

} // namespace mai::util::base64
