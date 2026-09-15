// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <vector>

#include "mem/hook.h"
#include "mem/struct.h"

// clang-format off

namespace mai::mem {

#define WeChat "wechat" // ----------------------------------------------------

inline DefineFunction<
    WeChat,
    0x4490D30,
    int(int, char**)>
    main;

inline DefineFunction<
    WeChat,
    0x89A785C, // todo, not confirmed
    void(uint32_t)>
    set_uin;

inline DefineFunction<
    WeChat,
    0x60AD100, // todo, not confirmed
    void(const std::vector<MsgItem>&, void*, void*)>
    parse_newsync_messages;

inline DefineFunction<
    WeChat,
    0x8195294,
    void*()>
    load_wmpf_host_export;

inline DefineFunction<
    WeChat,
    0x708DE60,
    LLVMStringNA*()>
    get_device_id;

inline DefineFunction<
    WeChat,
    0x73F9714,
    void(void*, uint32_t, const LLVMStringNA&, uint32_t, const LLVMStringNA&)>
    send_biz_menu_event;

#define BrowserHost "libwmpf_host_export" // ----------------------------------

inline DefineFunction<
    BrowserHost,
    0x2011E8,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_transfer_request;

inline DefineFunction<
    BrowserHost,
    0x200DFC,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_auth_request;

inline DefineFunction<
    BrowserHost,
    0x1FDE24,
    void(void*, uint32_t, const LLVMString&)>
    run_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x200F98,
    void*(void*, uint32_t, void*)>
    insert_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x279BF4,
    void(void*, void*)>
    init_browser;

inline ResolveFunction<
    "GetBrowsingService",
    void*()>
    get_browsing_service;

#define ILink2 "libilink2" // -------------------------------------------------

inline DefineFunction<
    ILink2,
    0x582A78,
    void*(void*, uint32_t)>
    wpkg_header_ctor;

inline DefineFunction<
    ILink2,
    0x582AAC,
    void*(void*)>
    wpkg_header_dtor;

inline DefineFunction<
    ILink2,
    0x582B24,
    void*(void*, uint32_t, uint64_t)>
    wpkg_header_set_u64;

inline DefineFunction<
    ILink2,
    0x582F04,
    void*(void*, uint32_t, const LLVMStringNA&)>
    wpkg_header_set_string;

inline DefineFunction<
    ILink2,
    0x583010,
    int(const void*, uint32_t, uint64_t*)>
    wpkg_header_get_u64;

inline DefineFunction<
    ILink2,
    0x583064,
    int(const void*, uint32_t, LLVMStringNA*)>
    wpkg_header_get_string;

inline DefineFunction<
    ILink2,
    0x5837F4,
    uint64_t(void*, const char*, uint32_t)>
    wpkg_header_deserialize;

inline DefineFunction<
    ILink2,
    0x5837D4,
    void*(const void*, char*, uint32_t*, uint32_t)>
    wpkg_header_serialize;

} // namespace mai::mem

// clang-format on
