// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>
#include <string>

namespace mai {

inline namespace shared_variables {

inline uint32_t    g_uin{0};
inline std::string g_cloud_proxy_device_id;
inline std::string g_cloud_proxy_session_info;
inline bool        g_slim{false};

} // namespace shared_variables

constexpr uint32_t MAGIC_TASK_ID = 114514'1919;

constexpr auto SERVICE_ACCOUNT_GH_USERNAME = "gh_6cfb73ca89e6";
constexpr auto SERVICE_ACCOUNT_MENU_ID     = 455590723;
constexpr auto SERVICE_ACCOUNT_MENU_KEY    = "rselfmenu_1";

constexpr auto CLOUD_PROXY_TRANSFER_CMD_ID   = 1254;
constexpr auto CLOUD_PROXY_TRANSFER_CMD_HOST = "shortcloud.weixin.com";
constexpr auto CLOUD_PROXY_TRANSFER_CMD_URL =
    "/ilink/ilinkapp/mm/bizoauth/oauth_authorize";
constexpr auto CLOUD_PROXY_SHORTCONN_HEADER_SIZE = 16;

constexpr auto WEIXIN_WAHLAP_AIME_URL =
    "https://tgk-wcaime.wahlap.com/wc_auth/oauth/authorize/maimai-dx";

} // namespace mai
