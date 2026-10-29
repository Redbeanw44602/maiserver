// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "util/simple_web.h"

namespace mai::api {

class BasicAPI {
public:
    BasicAPI(std::string address, uint16_t port);
    ~BasicAPI();

    void start_server();

    void stop_server();

    void upgradable(WsServer& ws_server);

private:
    std::mutex  mutex_;
    std::thread thread_;
    HttpServer  server_;

    auto& GET(std::string regex) {
        return server_.resource[std::move(regex)]["GET"];
    }
};

} // namespace mai::api