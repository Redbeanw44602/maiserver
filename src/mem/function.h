// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#if __x86_64__
#include "mem/arch/function_x86_64.inc"
#elif __aarch64__
#include "mem/arch/function_arm64.inc"
#else
#error "unsupported architecture!"
#endif
