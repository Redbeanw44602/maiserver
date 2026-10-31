// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <nlohmann/json.hpp>

#include "api/streaming.h"
#include "log.h"
#include "service/aimetoken.h"
#include "service/qrcode.h"

using namespace mai::service;

namespace mai::api {

namespace {

void cmd_qrcode([[maybe_unused]] std::string_view path, WsSender& send) {
    auto result = request_qrcode_async();
    if (!result) {
        send(result.error());
    }
}

void cmd_aimetoken(std::string_view path, WsSender& send) {
    auto service_name = path.substr(path.find_last_of('/') + 1);
    if (auto result = get_aimetoken(service_name)) {
        // clang-format off
        send(
            nlohmann::ordered_json{
                {"url", result->url}
            }
        );
        // clang-format on
    } else {
        send(result.error());
    }
}

void on_message(
    std::shared_ptr<WsServer::Connection> connection,
    std::shared_ptr<WsServer::InMessage>  message
) {
    // Incoming message:
    // {
    //   "path": "/api/v1/...",
    //   "echo": 1919810   # non-zero, optional
    // }
    //
    // Outcoming message:
    // {
    //   "type": "reply",
    //   "echo": 1919810,  # optional
    //   "code": 0,
    //   "message": "...",
    //   "data": { ... }   # optional if code != 0
    // }
    // {
    //   "type": "broadcast",
    //   "sub_type": "...",
    //   "data": { ... }
    // }

    using cmd_callback_t = std::function<void(std::string_view, WsSender&)>;
    static std::unordered_map<std::string_view, cmd_callback_t> commands = {
        {"/api/v1/auth/qrcode",              cmd_qrcode   },
        {"/api/v1/auth/aimetoken/maimai-dx", cmd_aimetoken},
        {"/api/v1/auth/aimetoken/chunithm",  cmd_aimetoken}
    };

    WsSender send(*connection);

    try {
        auto incoming = nlohmann::json::parse(message->string());
        auto path     = incoming.at("path").get<std::string_view>();
        if (incoming.contains("echo")) {
            auto echo = incoming["echo"].get<uint32_t>();
            send.echo(echo);
        }
        if (auto it = commands.find(path); it != commands.end()) {
            it->second(path, send);
        } else {
            send(MaiError::COMMAND_NOT_FOUND);
        }
    } catch (const nlohmann::json::exception& e) {
        send(MaiError::ILLEGAL_REQUEST);
    }
}

void on_open(std::shared_ptr<WsServer::Connection> connection) {
    auto& remote = connection.get()->remote_endpoint();
    DBG("websocket: opened connection -> {}:{}",
        remote.address().to_string(),
        remote.port());
}

void on_close(
    std::shared_ptr<WsServer::Connection> connection,
    int                                   code,
    const std::string&                    reason
) {
    auto& remote = connection.get()->remote_endpoint();
    DBG("websocket: closed connection -> {}:{}, code = {}, reason = {}",
        remote.address().to_string(),
        remote.port(),
        code,
        reason);
}

void on_error(
    std::shared_ptr<WsServer::Connection> connection,
    const std::error_code&                ec
) {
    auto& remote = connection.get()->remote_endpoint();
    DBG("websocket: error in connection -> {}:{}, code = {} ({})",
        remote.address().to_string(),
        remote.port(),
        ec.value(),
        ec.message());
}

} // namespace

StreamingAPI::StreamingAPI() {
    auto& stream_endpoint      = server_.endpoint["^/api/v1/stream$"];
    stream_endpoint.on_open    = on_open;
    stream_endpoint.on_close   = on_close;
    stream_endpoint.on_message = on_message;
    stream_endpoint.on_error   = on_error;
    on_qrcode_received_connection =
        on_qrcode_received.connect([this](const MaiQRCodeOrErr& qrcode_or_err) {
            if (!qrcode_or_err) {
                return;
            }
            broadcast(
                "qrcode_received",
                static_cast<nlohmann::ordered_json>(*qrcode_or_err)
            );
        });
}

WsServer& StreamingAPI::get_server() { return server_; }

void StreamingAPI::broadcast(
    std::string_view         sub_type,
    nlohmann::ordered_json&& data
) {
    for (auto connection : server_.get_connections()) {
        WsSender send(*connection);
        send(sub_type, std::move(data));
    }
}

} // namespace mai::api
