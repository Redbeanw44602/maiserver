// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <expected>

#include <nlohmann/json.hpp>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <simple-web-server/server_http.hpp>
#include <simple-websocket-server/server_ws.hpp>

#include "error.h"
#include "log.h"

namespace mai::util::detail {

template <typename T>
struct ResponseOk {
    uint32_t    code;
    std::string message;
    T           data;
};

struct ResponseError {
    uint32_t    code;
    std::string message;
};

template <typename Resp, typename Error>
class Responder {
public:
    Responder(Resp& response) : response_(response) {}

    void operator()(Error error) {
        ResponseError obj{
            static_cast<uint32_t>(error),
            static_cast<std::string>(to_string(error)),
        };
        write_response(
            SimpleWeb::StatusCode::server_error_bad_gateway,
            rfl::json::write(obj, rfl::json::pretty)
        );
    }

    template <typename T, typename E>
    void operator()(std::expected<T, E>&& data) {
        if (!data.has_value()) {
            return operator()(data.error());
        }
        ResponseOk<T> obj{0, "Success.", std::move(*data)};
        write_response(
            SimpleWeb::StatusCode::success_ok,
            rfl::json::write(obj, rfl::json::pretty)
        );
    }

private:
    Resp& response_;

    void write_response(SimpleWeb::StatusCode code, std::string&& data) {
        // clang-format off
        response_.write(
            code,
            std::move(data),
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

} // namespace mai::util::detail

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using WsServer   = SimpleWeb::SocketServer<SimpleWeb::WS>;

using HttpResponder =
    mai::util::detail::Responder<HttpServer::Response, mai::MaiError>;
using WsSender = mai::util::detail::Sender<WsServer::Connection, mai::MaiError>;
