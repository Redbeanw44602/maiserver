// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <shared_mutex>
#include <string>

namespace mai {

inline namespace shared_variables {

// RACE: May be set multiple times.
inline std::atomic<uint32_t> g_uin{0};

// RACE: Set once, but cannot be predicted, related to
// `g_cloud_proxy_device_id`.
inline std::string       g_cloud_proxy_session_info;
inline std::shared_mutex g_cloud_proxy_session_info_lock;

// NO RACE: Guarantees that it is set only once, at startup.
inline std::string g_cloud_proxy_device_id;
inline bool        g_slim{false};

} // namespace shared_variables

constexpr auto SERVICE_ACCOUNT_GH_USERNAME = "gh_6cfb73ca89e6";
constexpr auto SERVICE_ACCOUNT_MENU_ID     = 455590723;
constexpr auto SERVICE_ACCOUNT_MENU_KEY    = "rselfmenu_1";

constexpr auto CLOUD_PROXY_TRANSFER_CMD_ID   = 1254;
constexpr auto CLOUD_PROXY_TRANSFER_CMD_HOST = "shortcloud.weixin.com";
constexpr auto CLOUD_PROXY_TRANSFER_CMD_URL =
    "/ilink/ilinkapp/mm/bizoauth/oauth_authorize";
constexpr auto CLOUD_PROXY_SHORTCONN_HEADER_SIZE = 16;

constexpr auto TGK_WCAIME_OAUTH_URL =
    "https://tgk-wcaime.wahlap.com/wc_auth/oauth/authorize";

} // namespace mai
