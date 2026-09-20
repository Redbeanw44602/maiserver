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
    0x44FEA10,
    int(int, char**)>
    main;

inline DefineFunction<
    WeChat,
    0x8E1B800,
    void(uint32_t)>
    set_uin;

inline DefineFunction<
    WeChat,
    0x6216510,
    void(const std::vector<MsgItem>&, void*, void*)>
    parse_newsync_messages;

inline DefineFunction<
    WeChat,
    0x8525910,
    void*()>
    load_wmpf_host_export;

inline DefineFunction<
    WeChat,
    0x736DFD0,
    LLVMStringNA*()>
    get_device_id;

inline DefineFunction<
    WeChat,
    0x770DFA0,
    void(void*, uint32_t, const LLVMStringNA&, uint32_t, const LLVMStringNA&)>
    send_biz_menu_event;

inline DefineFunction<
    WeChat,
    0x8195B80,
    void(void*)>
    init_wxocr;

inline DefineFunction<
    WeChat,
    0x81885E0,
    void(void*)>
    init_wxutility;

inline DefineFunction<
    WeChat,
    0x84C5050,
    void(void*)>
    init_wxplayer;

inline DefineFunction<
    WeChat,
    0x8173590,
    void(void*)>
    init_wxplayer_2;

inline DefineFunction<
    WeChat,
    0x44F9800,
    void(void*, void*, void*, void*, void*)>
    init_crashpad;

#define BrowserHost "libwmpf_host_export" // ----------------------------------

inline DefineFunction<
    BrowserHost,
    0x1EEFB0,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_transfer_request;

inline DefineFunction<
    BrowserHost,
    0x1EEB30,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_auth_request;

inline DefineFunction<
    BrowserHost,
    0x1EB6B0,
    void(void*, uint32_t, const LLVMString&)>
    run_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x1EECF0,
    void*(void*, uint32_t, void*)>
    insert_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x26D8C0,
    void(void*, void*)>
    init_browser;

inline ResolveFunction<
    "GetBrowsingService",
    void*()>
    get_browsing_service;

#define ILink2 "libilink2" // -------------------------------------------------

inline DefineFunction<
    ILink2,
    0x556F60,
    void*(void*, uint32_t)>
    wpkg_header_ctor;

inline DefineFunction<
    ILink2,
    0x556F90,
    void*(void*)>
    wpkg_header_dtor;

inline DefineFunction<
    ILink2,
    0x557000,
    void*(void*, uint32_t, uint64_t)>
    wpkg_header_set_u64;

inline DefineFunction<
    ILink2,
    0x557370,
    void*(void*, uint32_t, const LLVMStringNA&)>
    wpkg_header_set_string;

inline DefineFunction<
    ILink2,
    0x557450,
    int(const void*, uint32_t, uint64_t*)>
    wpkg_header_get_u64;

inline DefineFunction<
    ILink2,
    0x5574A0,
    int(const void*, uint32_t, LLVMStringNA*)>
    wpkg_header_get_string;

inline DefineFunction<
    ILink2,
    0x557CA0,
    uint64_t(void*, const char*, uint32_t)>
    wpkg_header_deserialize;

inline DefineFunction<
    ILink2,
    0x557C80,
    void*(const void*, char*, uint32_t*, uint32_t)>
    wpkg_header_serialize;

} // namespace mai::mem

// clang-format on
