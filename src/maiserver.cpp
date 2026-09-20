// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <asio/ip/address.hpp>
#include <base64.hpp>

#include "config.h"
#include "error.h"
#include "mem/function.h"
#include "service/aimetoken.h"
#include "service/qrcode.h"
#include "util/env.h"
#include "util/http_server.h"
#include "util/string.h"

using namespace mai;
using namespace mai::util;
using namespace mai::service;
using namespace mai::mem;

using HttpServer    = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpResponder = Responder<HttpServer::Response, MaiError>;

void endpoint_qrcode(
    std::shared_ptr<HttpServer::Response>                 response_,
    [[maybe_unused]] std::shared_ptr<HttpServer::Request> request_
) {
    HttpResponder response(*response_);
    if (auto result = qrcode()) {
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
    if (auto result = aimetoken()) {
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

template <typename T>
auto environment(string::Literal name) {
    auto result = env::get<T>(name);
    if (!result && result.error() != MaiError::ENVIRONMENT_VARIABLE_NOT_FOUND) {
        std::println(
            "Invalid {} [error: \"{}\"]; use the default value.",
            static_cast<std::string_view>(name),
            to_string(result.error())
        );
    }
    return result;
}

auto config_from_environment() {
    // clang-format off
    struct {
        std::string address    = "0.0.0.0";
        uint16_t    port       = 8080;
        std::string device_id;
        bool        slim       = false;
    } config;

    auto address   = environment <std::string> ("MAISERVER_LISTEN_ADDRESS");
    auto port      = environment <uint16_t>    ("MAISERVER_LISTEN_PORT");
    auto device_id = environment <std::string> ("MAISERVER_CLOUD_PROXY_DEVICE_ID");
    auto slim      = environment <bool>        ("MAISERVER_SLIM");
    // clang-format on

    if (address) {
        asio::error_code ec;
        asio::ip::make_address(*address, ec);
        if (!ec) {
            std::println(
                "Invalid address '{}'; use the default value '{}'.",
                *address,
                config.address
            );
        } else {
            config.address = *address;
        }
    }

    if (port) {
        config.port = *port;
    }

    if (device_id) {
        config.device_id = base64::from_base64(*device_id);
        if (config.device_id.size() != 32) {
            std::println(
                "Invalid device ID; it must be 32 bytes long and "
                "base64-encoded."
            );
            config.device_id.clear();
        }
    }

    if (slim) {
        config.slim = *slim;
    }

    return config;
}

HOOK(main, int argc, char** argv) {
    std::println("Hello maiserver!");

    auto config = config_from_environment();

    if (config.device_id.empty()) {
        std::println("The device ID environment variable must be set.");
        return origin(argc, argv);
    }

    g_cloud_proxy_device_id = config.device_id;
    g_slim                  = config.slim;

    std::println("The device ID is set to: {}", string::hex(config.device_id));

    HttpServer server;
    server.config.address                             = config.address;
    server.config.port                                = config.port;
    server.resource["^/api/v1/auth/qrcode"]["GET"]    = endpoint_qrcode;
    server.resource["^/api/v1/auth/aimetoken"]["GET"] = endpoint_aimetoken;

    std::thread server_thread([&server]() {
        std::println(
            "Starting to listen on {}:{} ...",
            server.config.address,
            server.config.port
        );
        server.start();
    });
    server_thread.detach();

    return origin(argc, argv);
};

HOOK(set_uin, uint32_t uin) {
    g_uin = uin;
    return origin(uin);
}

__attribute__((constructor)) static void init(void) {
    // Prevent LD_PRELOAD Injection into Child Processes.
    unsetenv("LD_PRELOAD");
}
