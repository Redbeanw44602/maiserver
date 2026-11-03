// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <cpr/cpr.h>

#include "log.h"
#include "service/aimetoken.h"
#include "service/maimai-dx/helper.h"
#include "service/maimai-dx/session.h"
#include "util/string.h"

using namespace mai::util;

namespace mai::service::session {

namespace {

static std::mutex   mutex_;
static cpr::Session session_;

cpr::Response get(std::string_view url, bool follow_redirect = true) {
    static const cpr::UserAgent ua = BROWSER_USER_AGENT;
    session_.RemoveContent();
    session_.GetHeader().clear();
    session_.SetUserAgent(ua);
    session_.SetUrl(cpr::Url{url});
    session_.SetRedirect(cpr::Redirect{follow_redirect});
    return session_.Get();
}

bool has_cookie(std::string_view name) {
    auto        curl    = session_.GetCurlHolder();
    curl_slist* cookies = nullptr;
    auto        result =
        curl_easy_getinfo(curl->handle, CURLINFO_COOKIELIST, &cookies);
    if (!result && cookies) {
        curl_slist* each = cookies;
        while (each) {
            auto cookie = string::split_view(each->data, "\t");
            each        = each->next;
            if (cookie.size() != 7 && cookie[5] == name) {
                return true;
            }
        }
        curl_slist_free_all(cookies);
    }
    return false;
}

bool is_session_valid() { return get(url::HOME, false).status_code == 200; }

std::expected<void, MaiError> login() {
    auto token = get_aimetoken_maimai_dx();
    if (!token) {
        return std::unexpected(token.error());
    }
    auto response = get(token->url, false);
    if (!response.header.contains("location")) {
        return std::unexpected(MaiError::EXPECT_REDIRECT);
    }
    DBG("login result: {}", response.text);
    if (!response.text.contains("login={\"errorID\":0,")) {
        return std::unexpected(MaiError::FAILED_TO_LOGIN_TO_MAIMAI_DX_NET);
    }
    return {};
}

std::expected<void, MaiError> prepare_session() {
    if (!has_cookie("friendCodeList") || !is_session_valid()) {
        DBG("session invalid, trying to login...");
        auto result = login();
        if (!result) {
            return result;
        }
    }
    return {};
}

} // namespace

std::expected<std::string, MaiError> simple_get(std::string_view url) {
    std::lock_guard lock(mutex_);
    auto            result = prepare_session();
    if (!result) {
        return std::unexpected(result.error());
    }
    auto response = get(url);
    if (response.status_code != 200) {
        DBG("simple get failed with status code: {}", response.status_code);
        return std::unexpected(MaiError::EXPECT_OK);
    }
    return response.text;
}

} // namespace mai::service::session
