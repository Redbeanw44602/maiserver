// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>
#include <expected>
#include <string>

#include "error.h"

namespace mai::mem::async_wrapper::cloud_proxy {

using SessionInfoOrErr = std::expected<std::string, MaiError>;
using PayloadOrErr     = std::expected<std::string, MaiError>;

SessionInfoOrErr auth();

PayloadOrErr transfer(
    uint32_t    cmd_id,
    std::string cmd_host,
    std::string cmd_url,
    std::string payload
);

} // namespace mai::mem::async_wrapper::cloud_proxy
