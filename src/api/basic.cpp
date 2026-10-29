// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "api/basic.h"

#include "log.h"
#include "service/aimetoken.h"
#include "service/qrcode.h"

using namespace mai::service;

namespace mai::api {

void endpoint_qrcode(
    std::shared_ptr<HttpServer::Response>                 response_,
    [[maybe_unused]] std::shared_ptr<HttpServer::Request> request_
) {
    HttpResponder response(*response_);
    if (auto result = request_qrcode()) {
        // clang-format off
        return response(
            nlohmann::ordered_json{
                {"url",         result->url        },
                {"description", result->description},
                {"maid",        result->maid       },
                {"expires_in",  result->expires_in }
            }
        );
        // clang-format on
    } else {
        return response(result.error());
    }
}

void endpoint_aimetoken(
    std::shared_ptr<HttpServer::Response>                 response_,
    [[maybe_unused]] std::shared_ptr<HttpServer::Request> request_
) {
    HttpResponder response(*response_);
    auto          path         = std::string_view(request_->path);
    auto          service_name = path.substr(path.find_last_of('/') + 1);
    if (auto result = get_aimetoken(service_name)) {
        // clang-format off
        return response(
            nlohmann::ordered_json{
                {"url", result->url}
            }
        );
        // clang-format on
    } else {
        return response(result.error());
    }
}

void endpoint_default(
    std::shared_ptr<HttpServer::Response>                 response_,
    [[maybe_unused]] std::shared_ptr<HttpServer::Request> request_
) {
    return response_->write(SimpleWeb::StatusCode::client_error_not_found);
}

BasicAPI::BasicAPI(std::string address, uint16_t port) {
    using namespace std::placeholders;

    server_.config.address                    = std::move(address);
    server_.config.port                       = port;
    server_.default_resource["GET"]           = endpoint_default;
    GET("^/api/v1/auth/qrcode$")              = endpoint_qrcode;
    GET("^/api/v1/auth/aimetoken/maimai-dx$") = endpoint_aimetoken;
    GET("^/api/v1/auth/aimetoken/chunithm$")  = endpoint_aimetoken;
}

BasicAPI::~BasicAPI() { stop_server(); }

void BasicAPI::start_server() {
    std::lock_guard lock(mutex_);

    if (thread_.joinable()) {
        return DBG("warning! BasicAPI server has already started");
    }

    thread_ = std::thread([this]() {
        std::println(
            "Starting to listen on {}:{} ...",
            server_.config.address,
            server_.config.port
        );
        server_.start();
    });
}

void BasicAPI::stop_server() {
    std::lock_guard lock(mutex_);

    server_.stop();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void BasicAPI::upgradable(WsServer& ws_server) {
    server_.on_upgrade = [&ws_server](
                             std::unique_ptr<SimpleWeb::HTTP>&    socket_,
                             std::shared_ptr<HttpServer::Request> request_
                         ) {
        auto connection =
            std::make_shared<WsServer::Connection>(std::move(socket_));
        connection->method       = std::move(request_->method);
        connection->path         = std::move(request_->path);
        connection->query_string = std::move(request_->query_string);
        connection->http_version = std::move(request_->http_version);
        connection->header       = std::move(request_->header);
        ws_server.upgrade(connection);
    };
}

} // namespace mai::api