// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <asio/ip/address.hpp>

#include "api/basic.h"
#include "config.h"
#include "error.h"
#include "mem/function.h"
#include "mem/signal.h"
#include "util/base64.h"
#include "util/env.h"
#include "util/string.h"

using namespace mai;
using namespace mai::util;
using namespace mai::mem;
using namespace mai::api;

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
        device_id = base64::decode(*device_id);
        if (!device_id || device_id->size() != 32) {
            std::println(
                "Invalid device ID; it must be 32 bytes long and "
                "base64-encoded."
            );
        } else {
            config.device_id = *device_id;
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

    BasicAPI http_api(config.address, config.port);
    http_api.start_server();

    return origin(argc, argv);
};

HOOK(set_uin, uint32_t uin) {
    g_uin = uin;
    return origin(uin);
}

HOOK(load_wmpf_host_export) {
    auto ret = origin();
    on_lib_wmpf_host_export_ready();
    return ret;
}

static CTOR_PRIORITY_MIDDLE void init() {
    // Prevent LD_PRELOAD Injection into Child Processes.
    unsetenv("LD_PRELOAD");
}
