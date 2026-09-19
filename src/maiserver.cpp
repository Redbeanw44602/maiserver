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

HOOK(main, int argc, char** argv) {
    std::println("Hello maiserver!");

    auto address = std::getenv("MAISERVER_LISTEN_ADDRESS");
    if (address) {
        asio::error_code ec;
        asio::ip::make_address(address, ec);
        if (!ec) {
            std::println(
                "Invalid address '{}'; falling back to the default value '{}'.",
                address,
                DEFAULT_LISTEN_ADDRESS
            );
            address = nullptr;
        }
    }

    auto port = DEFAULT_LISTEN_PORT;
    if (auto port_s = std::getenv("MAISERVER_LISTEN_PORT")) {
        auto has_err = false;
        try {
            port = std::stoi(port_s);
        } catch (const std::invalid_argument&) {
            has_err = true;
        } catch (const std::out_of_range&) {
            has_err = true;
        }
        if (has_err || (port < 0 || port > 65535)) {
            std::println(
                "Invalid port number {}; falling back to the default value "
                "({}).",
                port,
                DEFAULT_LISTEN_PORT
            );
            port = DEFAULT_LISTEN_PORT;
        }
    }

    auto device_id = std::getenv("MAISERVER_CLOUD_PROXY_DEVICE_ID");
    if (!device_id) {
        std::println("The device ID environment variable must be set.");
        return origin(argc, argv);
    }
    g_cloud_proxy_device_id = base64::from_base64(device_id);
    if (g_cloud_proxy_device_id.size() != 32) {
        std::println(
            "Invalid device ID; it must be 32 bytes long and base64-encoded."
        );
        return origin(argc, argv);
    }
    std::println(
        "The device ID is set to: {}",
        string::hex(g_cloud_proxy_device_id)
    );

    if (auto no_wmpf_mode = std::getenv("MAISERVER_NO_WMPF")) {
        if (std::string_view(no_wmpf_mode) != "0") {
            g_no_wmpf_mode = true;
        }
    }

    HttpServer server;
    server.config.address =
        address != nullptr ? address : DEFAULT_LISTEN_ADDRESS;
    server.config.port                                = port;
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
