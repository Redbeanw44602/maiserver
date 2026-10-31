// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>
#include <sigslot/signal.hpp>

#include "error.h"

namespace mai::service {

struct MaiQRCode {
    std::string url;
    std::string description;
    std::string maid;
    int64_t     expires_in;
};

using MaiQRCodeOrErr = std::expected<MaiQRCode, MaiError>;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    MaiQRCode,
    url,
    description,
    maid,
    expires_in
);

inline sigslot::signal<MaiQRCodeOrErr> on_qrcode_received;

/* (Recommended) Retrieve the QR code asynchronously. After executing this
 * method, receive the response via the `on_qrcode_received` signal. This method
 * has a 15-second timeout; any new executions during this period will be
 * ignored. */
std::expected<void, MaiError> request_qrcode_async();

/* Retrieves a QR code synchronously. This method is a wrapper around the
 * `request_qrcode` method and therefore shares the global timeout timer. If a
 * response is generated before the timeout expires, it will be returned
 * immediately. This method cannot handle scenarios with multiple responses and
 * is therefore not recommended. */
MaiQRCodeOrErr request_qrcode();

} // namespace mai::service
