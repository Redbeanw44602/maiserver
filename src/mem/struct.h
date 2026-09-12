// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>

#include "mem/std_string.h"

namespace mai::mem {

struct ProtoString {
    void*         unused_0;
    LLVMStringNA* str;

    static std::string_view Read(ProtoString* pstr) {
        if (pstr && pstr->str) {
            return pstr->str->view();
        }
        return "INVALID_PSTR";
    }
};

struct MsgItem {
    uint64_t     unused_0;
    ProtoString* sender_username; // +8, from: 68F70F0
    uint32_t     unused_1;
    int32_t      msg_type;          // +20, from: 62154D0
    ProtoString* receiver_username; // +24, from: 68F70F0
    ProtoString* content;           // +32, from: 6220A80
    uint64_t     unused_2;
    uint64_t     unused_3;
    uint64_t     unused_4;
    uint64_t     unused_5;
    uint32_t     create_time; // +72, from: 62154D0
    uint32_t     unused_6;
    uint64_t     unk_2; // +80, from: 62154D0, not confirmed
    uint64_t     unused_7;
    uint64_t     unused_8;
    uint64_t     unused_9;
    uint64_t     create_time_ms; // +112, from: 62154D0
};

static_assert(sizeof(MsgItem) == 120);

} // namespace mai::mem
