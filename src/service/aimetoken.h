// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string>

#include "error.h"

namespace mai::service {

struct AimeToken {
    std::string url;
};

std::expected<AimeToken, MaiError> aimetoken();

} // namespace mai::service
