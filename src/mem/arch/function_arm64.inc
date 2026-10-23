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
    0x4558080,
    int(int, char**)>
    main;

inline DefineFunction<
    WeChat,
    0x89BD84C,
    void(uint32_t)>
    set_uin;

inline DefineFunction<
    WeChat,
    0x60BD1B4,
    void(const std::vector<MsgItem>&, void*, void*)>
    parse_newsync_messages;

inline DefineFunction<
    WeChat,
    0x81A822C,
    void*()>
    load_wmpf_host_export;

inline DefineFunction<
    WeChat,
    0x70A0E90,
    LLVMStringNA*()>
    get_device_id;

inline DefineFunction<
    WeChat,
    0x740CBE8,
    void(void*, uint32_t, const LLVMStringNA&, uint32_t, const LLVMStringNA&)>
    send_biz_menu_event;

inline DefineFunction<
    WeChat,
    0x4557F80, 
    void()>
    init_crashpad;

inline DefineFunction<
    WeChat,
    0x7E5A154,
    uint32_t(void*)>
    init_wxocr;

inline DefineFunction<
    WeChat,
    0x7E38810,
    void(void*)>
    init_wxplayer;

inline DefineFunction<
    WeChat,
    0x814E804,
    void(void*)>
    init_wxplayer_2;

inline DefineFunction<
    WeChat,
    0x7E4C804,
    void(void*)>
    init_wxutility;

#define BrowserHost "libwmpf_host_export.so" // ----------------------------------

inline DefineFunction<
    BrowserHost,
    0x201480,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_transfer_request;

inline DefineFunction<
    BrowserHost,
    0x201094,
    void*(void*, uint32_t, const LLVMString&, void*)>
    send_cloud_proxy_auth_request;

inline DefineFunction<
    BrowserHost,
    0x1FE0BC,
    void(void*, uint32_t, const LLVMString&)>
    run_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x201230,
    void*(void*, uint32_t, void*)>
    insert_cloud_proxy_callback;

inline DefineFunction<
    BrowserHost,
    0x279E8C,
    void(void*, void*)>
    init_browser;

inline ResolveFunction<
    BrowserHost,
    "GetBrowsingService",
    void*()>
    get_browsing_service;

#define ILink2 "libilink2.so" // -------------------------------------------------

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
