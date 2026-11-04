// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "api/basic.h"

#include "config.h"
#include "log.h"
#include "service/aimetoken.h"
#include "service/maimai-dx/home/home.h"
#include "service/maimai-dx/home/rating_target_music.h"
#include "service/qrcode.h"

using namespace mai::service;

namespace mai::api {

namespace {

#define SIMPLE_HTTP_API(endpoint, backend)                                     \
    server_.resource["^/api/v1" endpoint "$"]["GET"] =                         \
        [](std::shared_ptr<HttpServer::Response> response_,                    \
           std::shared_ptr<HttpServer::Request>) {                             \
            HttpResponder response(*response_);                                \
            if (g_uin == 0) {                                                  \
                response(MaiError::WECHAT_NOT_LOGGED_IN);                      \
            }                                                                  \
            response(backend());                                               \
        }

} // namespace

BasicAPI::BasicAPI(std::string address, uint16_t port) {
    using namespace std::placeholders;

    server_.config.address          = std::move(address);
    server_.config.port             = port;
    server_.default_resource["GET"] = [](auto response, auto _) {
        return response->write(SimpleWeb::StatusCode::client_error_not_found);
    };

    SIMPLE_HTTP_API("/auth/qrcode", request_qrcode);
    SIMPLE_HTTP_API("/auth/aimetoken/maimai-dx", get_aimetoken_maimai_dx);
    SIMPLE_HTTP_API("/auth/aimetoken/chunithm", get_aimetoken_chunithm);
    SIMPLE_HTTP_API("/maimai-dx/player/info", get_player_information);
    SIMPLE_HTTP_API("/maimai-dx/player/rating", get_rating_detail);
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
