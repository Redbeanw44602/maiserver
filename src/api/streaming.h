// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <nlohmann/json_fwd.hpp>
#include <sigslot/signal.hpp>

#include "util/simple_web.h"

namespace mai::api {

class StreamingAPI {
public:
    StreamingAPI();

    WsServer& get_server();

    void broadcast(std::string_view sub_type, nlohmann::ordered_json&& data);

private:
    WsServer server_;

    sigslot::scoped_connection on_qrcode_received_connection;
};

} // namespace mai::api
