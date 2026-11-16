// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "service/maimai-dx/session.h"
#include "log.h"
#include "service/aimetoken.h"
#include "service/maimai-dx/helper.h"

using namespace mai::util;

namespace mai::service::session::detail {

namespace {

static std::mutex   mutex_;
static cpr::Cookies cookies_;

static thread_local cpr::ConnectionPool pool_;

void update_cookies(cpr::Cookies&& cookies) {
    cookies_        = std::move(cookies);
    cookies_.encode = false;
    for ([[maybe_unused]] const auto& cookie : cookies_) {
        DBV("UPDATE COOKIE: '{}': '{}'", cookie.GetName(), cookie.GetValue());
    }
}

std::expected<void, MaiError> login() {
    auto token = get_aimetoken_maimai_dx();
    if (!token) {
        return std::unexpected(token.error());
    }
    rate_limiter::acquire(token->url);
    auto response = cpr::Get(
        cpr::Url{token->url},
        cpr::Redirect{false},
#if MAI_LOG_VERBOSE
        cpr::Verbose{true},
#endif
        pool_,
        BROWSER_USER_AGENT
    );
    if (!response.header.contains("location")) {
        return std::unexpected(MaiError::EXPECT_REDIRECT);
    }
    DBG("login result: {}", response.text);
    if (!response.text.contains("login={\"errorID\":0,")) {
        return std::unexpected(MaiError::FAILED_TO_LOGIN_TO_MAIMAI_DX_NET);
    }
    update_cookies(std::move(response.cookies));
    return {};
}

} // namespace

void intercept(cpr::Session& session) {
    std::lock_guard lock(mutex_);
    if (!std::any_of(cookies_.begin(), cookies_.end(), [](const auto& cookie) {
            return cookie.GetName() == "friendCodeList";
        })) {
        DBG("session invalid, trying to login...");
        auto result = login();
        if (!result) {
            return DBG("login failed, error: {}", to_string(result.error()));
        }
    }
    session.SetUserAgent(BROWSER_USER_AGENT);
    // Copied; libcpr internally encodes and sets CURLOPT_COOKIE.
    session.SetCookies(cookies_);
    session.SetConnectionPool(pool_);
#if MAI_LOG_VERBOSE
    session.SetVerbose(cpr::Verbose{true});
#endif
}

void intercept(cpr::Response& response) {
    std::lock_guard lock(mutex_);
    auto            url = std::string_view(response.url.c_str());
    if (url.ends_with("/maimai-mobile/error/")
        || url.ends_with("/maimai-mobile/logout/")) {
        DBG("invalid session detected, reset cookies");
        update_cookies(cpr::Cookies{});
    }
}

} // namespace mai::service::session::detail
