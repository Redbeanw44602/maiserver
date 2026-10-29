// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <print> // IWYU pragma: export

#if MAI_DEBUG
#define DBG(...) std::println("[maiserver:debug] " __VA_ARGS__)
#else
#define DBG(...) ((void)0)
#endif
