// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "error.h"

namespace mai::service {

struct AimeToken {
    std::string url;
};

using AimeTokenOrErr = std::expected<AimeToken, MaiError>;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AimeToken, url);

/* Retrieve the URL with the `t` param included synchronously, which has been
 * authenticated via WeChat OAuth for logging into maimai DX NET / CHUNITHM NET.
 * Known service names: 'maimai-dx', 'chunithm'.
 */
AimeTokenOrErr get_aimetoken(std::string_view service_name);

} // namespace mai::service
