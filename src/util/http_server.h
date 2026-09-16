// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <server_http.hpp>
#include <server_https.hpp>

namespace mai::util {

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
                {"code",    0         },
                {"message", "Success."},
                {"data",    data      }
            }
        );
        // clang-format on
    }

private:
    Resp& response_;

    void write_response(
        SimpleWeb::StatusCode    status_code,
        nlohmann::ordered_json&& body
    ) {
        // clang-format off
        response_.write(
            status_code,
            body.dump(4),
            SimpleWeb::CaseInsensitiveMultimap{
                {"Content-Type", "application/json"}
            }
        );
        // clang-format on
    }
};

} // namespace mai::util
