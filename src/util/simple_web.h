// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <simple-web-server/server_http.hpp>
#include <simple-websocket-server/server_ws.hpp>

#include "error.h"
#include "log.h"

template <typename Resp, typename Error>
class Responder {
public:
    Responder(Resp& response) : response_(response) {}

    void operator()(Error error) {
        // clang-format off
        write_response(
            SimpleWeb::StatusCode::server_error_bad_gateway,
            nlohmann::ordered_json{
                {"code",    error           },
                {"message", to_string(error)}
            }
        );
        // clang-format on
    }

    void operator()(nlohmann::ordered_json&& data) {
        // clang-format off
        write_response(
            SimpleWeb::StatusCode::success_ok,
            nlohmann::ordered_json{
                {"code",    0               },
                {"message", "Success."      },
                {"data",    std::move(data) }
            }
        );
        // clang-format on
    }

private:
    Resp& response_;

    void write_response(
        SimpleWeb::StatusCode    status_code,
        nlohmann::ordered_json&& data
    ) {
        // clang-format off
        response_.write(
            status_code,
            data.dump(4),
            SimpleWeb::CaseInsensitiveMultimap{
                {"Content-Type", "application/json"}
            }
        );
        // clang-format on
    }
};

template <typename Conn, typename Error>
class Sender {
public:
    Sender(Conn& connection) : connection_(connection) {}

    void operator()(Error error) {
        send_reply([&](auto& message) {
            message["code"]    = error;
            message["message"] = to_string(error);
        });
    }

    void operator()(nlohmann::ordered_json&& data) {
        send_reply([&](auto& message) {
            message["code"]    = 0;
            message["message"] = "Success.";
            message["data"]    = std::move(data);
        });
    }

    // broadcast
    void operator()(std::string_view sub_type, nlohmann::ordered_json&& data) {
        send_broadcast([&](auto& message) {
            message["sub_type"] = sub_type;
            message["data"]     = std::move(data);
        });
    }

    void echo(uint32_t value) { echo_ = value; }

private:
    Conn&    connection_;
    uint32_t echo_{};

    void send(auto&& modifier) {
        nlohmann::ordered_json message;
        modifier(message);
        connection_.send(message.dump(), [](const std::error_code& ec) {
            if (ec) {
                DBG("websocket: error sending message, code = {} ({})",
                    ec.value(),
                    ec.message());
            }
        });
    }

    void send_reply(auto&& modifier) {
        send([&](auto& message) {
            message["type"] = "reply";
            if (echo_) message["echo"] = echo_;
            modifier(message);
        });
    }

    void send_broadcast(auto&& modifier) {
        send([&](auto& message) {
            message["type"] = "broadcast";
            modifier(message);
        });
    }
};

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using WsServer   = SimpleWeb::SocketServer<SimpleWeb::WS>;

using HttpResponder = Responder<HttpServer::Response, mai::MaiError>;
using WsSender      = Sender<WsServer::Connection, mai::MaiError>;
