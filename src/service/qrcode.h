// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>
#include <string>

#include "error.h"

namespace mai::service {

struct MaiQRCode {
    std::string url;
    std::string description;
    std::string maid;
    int64_t     expires_in;
};

std::expected<MaiQRCode, MaiError> qrcode();

} // namespace mai::service
