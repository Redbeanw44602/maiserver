// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cpr/cpr.h>

#include "error.h"
#include "log.h"
#include "service/maimai-dx/rate_limiter.h"

namespace mai::service::session {

namespace detail {

void intercept(cpr::Session& session);
void intercept(cpr::Response& response);

} // namespace detail

using cpr_method_t = decltype(&cpr::Session::Get);

/* This is just a workaround that allows you to operate `cpr::Session` on your
 * own; if you don't send the request as soon as possible, it will interfere
 * with the rate limiter's operation. */
template <typename... Options>
void request_later(cpr::Session& session, Options&&... opts) {
    cpr::priv::set_option(session, std::forward<Options>(opts)...);
    rate_limiter::acquire(session.GetFullRequestUrl());
    detail::intercept(session);
}

inline cpr::Response request(cpr_method_t method, cpr::Session& session) {
    if (!method) {
        DBG("warning! method is nullptr");
        return {};
    }
    auto response = (session.*method)();
    detail::intercept(response);
    return response;
}

template <typename... Options>
cpr::Response request(cpr_method_t method, Options&&... opts) {
    cpr::Session session;
    request_later(session, std::forward<Options>(opts)...);
    return request(method, session);
}

/* Utility Function:
 * GET the resource and return the response body. */
template <typename... Options>
std::expected<std::string, MaiError> get_text(Options&&... opts) {
    auto response = request(&cpr::Session::Get, std::forward<Options>(opts)...);
    if (response.status_code != 200) {
        DBG("get text failed with status code: {}", response.status_code);
        return std::unexpected(MaiError::EXPECT_OK);
    }
    return response.text;
}

} // namespace mai::service::session
