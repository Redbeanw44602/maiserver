// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cpr/cpr.h>

#include "error.h"

namespace mai::service::session {

std::expected<std::string, MaiError> simple_get(
    std::string_view       url,
    bool                   follow_redirect      = true,
    const cpr::Parameters& parameters           = {},
    bool                   force_refresh_cookie = false
);

}
