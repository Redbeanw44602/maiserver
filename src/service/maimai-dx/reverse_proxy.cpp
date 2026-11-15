// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <cpr/cpr.h>

#include "service/maimai-dx/helper.h"
#include "service/maimai-dx/reverse_proxy.h"
#include "util/simple_web.h"
#include "util/string.h"

using namespace mai::util;

namespace mai::service {

namespace {

using Headers = SimpleWeb::CaseInsensitiveMultimap;

static const std::unordered_map<std::string, session::cpr_method_t> methods_{
    {"GET",     &cpr::Session::Get    },
    {"HEAD",    &cpr::Session::Head   },
    {"POST",    &cpr::Session::Post   },
    {"PUT",     &cpr::Session::Put    },
    {"PATCH",   &cpr::Session::Patch  },
    {"DELETE",  &cpr::Session::Delete },
    {"OPTIONS", &cpr::Session::Options}
};

static std::string path_prefix_;

auto get_skipped_headers(const auto& headers) {
    using namespace util::string;
    std::unordered_set<std::string> result{
        // hop-by-hop
        "connection",
        "keep-alive",
        "proxy-connection",
        "proxy-authorization",
        "proxy-authenticate",
        "transfer-encoding",
        "upgrade",
        // managed by maiserver
        "accept-encoding",
        "cookie",
        "set-cookie",
        "user-agent"
    };
    for (const auto& [name, value] : headers) {
        if (lower(name) != "connection") continue;
        for (auto& field : split_view(value, ",")) {
            result.emplace(lower(static_cast<std::string>(strip(field))));
        }
    }
    return result;
}

// libcpr does not support multiple response headers with the same name
// see https://github.com/libcpr/cpr/issues/288
Headers parse_headers(std::string_view raw_text) {
    Headers result;
    bool    in_block = false;
    for (auto line : string::split_view(raw_text, "\n")) {
        line = string::strip(line, false);
        if (line.starts_with("HTTP/")) {
            result.clear();
            in_block = true;
        } else if (line.empty()) {
            in_block = false;
            // Multiple response may be present, such as 101 Continue, so don't
            // break here
        } else if (in_block) {
            if (auto pos = line.find(':'); pos != line.npos) {
                result.emplace(
                    line.substr(0, pos),
                    string::strip(line.substr(pos + 1), false)
                );
            }
        }
    }
    return result;
}

void forward(
    std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request>  request
) {
    using status_code = SimpleWeb::StatusCode;

    const auto& method       = request->method;
    const auto& path         = request->path;
    const auto& query_string = request->query_string;
    const auto& header       = request->header;
    const auto& content      = request->content;

    if (!methods_.contains(method)) {
        return response->write(status_code::client_error_bad_request);
    }

    auto origin_path  = path.substr(path_prefix_.size());
    auto origin_query = query_string.empty() ? "" : "?" + query_string;
    auto origin_url =
        std::format("{}{}{}", url::ORIGIN, origin_path, origin_query);
    if (origin_url.contains('#')) {
        return response->write(status_code::client_error_bad_request);
    }

    auto skipped_headers = get_skipped_headers(header);
    skipped_headers.insert({"host", "content-length", "expect", "forwarded"});
    cpr::Header origin_headers;
    for (const auto& [key, value] : header) {
        auto header = string::lower(key);
        if (skipped_headers.contains(header)
            || header.starts_with("x-forwarded-"))
            continue;
        auto [it, inserted] = origin_headers.emplace(header, value);

        // If a request header with the same name exists, try to concat them.
        if (!inserted) it->second += (header == "cookie" ? "; " : ", ") + value;
    }
    DBG("origin_url = {}, origin_headers = {}", origin_url, origin_headers);

    cpr::Session session;
    session::request_later(
        session,
        cpr::Url{origin_url},
        cpr::Header{origin_headers},
        cpr::Redirect{false}
        // TODO: Handle `Accept-Encoding` correctly, such as by finding the
        // intersection between the client and the proxy.
    );
    if (content.size()) {
        session.SetBodyView(cpr::BodyView{content.string()});
    }
    DBG("sending upstream request...");
    auto upstream = session::request(methods_.at(method), session);

    if (upstream.error.code != cpr::ErrorCode::OK
        || upstream.status_code < 200) {
        DBG("upstream request failed with error: {} ({})",
            static_cast<uint32_t>(upstream.error.code),
            upstream.error.message);
        return response->write(
            upstream.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT
                ? status_code::server_error_gateway_timeout
                : status_code::server_error_bad_gateway
        );
    }

    auto upstream_status_code = static_cast<status_code>(upstream.status_code);
    // Simple-Web-Server cannot recognize non-standard HTTP status codes
    // 567: Bloced by Tencent Cloud EdgeOne
    if (upstream.status_code == 567) {
        upstream_status_code = status_code::server_error_bad_gateway;
    }

    auto upstream_headers = parse_headers(upstream.raw_header);
    skipped_headers       = get_skipped_headers(upstream_headers);
    for (auto it = upstream_headers.begin(); it != upstream_headers.end();) {
        if (skipped_headers.contains(it->first)) {
            it = upstream_headers.erase(it);
        } else {
            auto name = string::lower(it->first);
            if (name == "location" || name == "refresh") {
                string::replace_all(it->second, url::ORIGIN, path_prefix_);
            }
            ++it;
        }
    }
    DBV("upstream status_code = {}, raw_headers = \n'{}'",
        static_cast<uint32_t>(upstream_status_code),
        upstream.raw_header);

    if (upstream.text.empty()) {
        response->write(upstream_status_code, upstream_headers);
    } else {
        string::replace_all(upstream.text, url::ORIGIN, path_prefix_);
        string::replace_all(
            upstream.text,
            "var dir =  arrayL[0] + \"//\" + arrayL[2]",
            std::format("var dir = \"{}\"", path_prefix_)
        );
        response->write(upstream_status_code, upstream.text, upstream_headers);
    }
}

} // namespace

void register_reverse_proxy(HttpServer& server, std::string_view path_prefix) {
    for (const auto& [method, _] : methods_) {
        server.resource[std::format("{}(/.*)?", path_prefix)][method] = forward;
    }
    path_prefix_ = path_prefix;
}

} // namespace mai::service
