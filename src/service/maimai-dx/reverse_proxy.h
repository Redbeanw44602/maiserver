// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "util/simple_web.h"

namespace mai::service {

void register_reverse_proxy(HttpServer& server, std::string_view path_prefix);

} // namespace mai::service
