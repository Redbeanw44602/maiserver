// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "config.h"
#include "mem/function.h"

using namespace mai::mem;

HOOK_DELAYED(init_browser, void* a1, void* a2) {
    if (!g_slim) origin(a1, a2);
}

HOOK(init_wxocr, void* a1) {
    if (!g_slim) origin(a1);
}

HOOK(init_wxutility, void* a1) {
    if (!g_slim) origin(a1);
}

HOOK(init_wxplayer, void* a1) {
    if (!g_slim) origin(a1);
}

HOOK(init_wxplayer_2, void* a1) {
    if (!g_slim) origin(a1);
}

HOOK(init_crashpad, void* a1, void* a2, void* a3, void* a4, void* a5) {
    if (!g_slim) origin(a1, a2, a3, a4, a5);
}
