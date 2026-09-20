// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <base64.hpp>
#include <lz4.h>
#include <nlohmann/json.hpp>

#include "config.h"
#include "log.h"
#include "mem/function.h"
#include "mem/std_string.h"
#include "mem/wpkg_header.h"
#include "util/string.h"

#include "micromsg.pb.h"

using namespace mai;
using namespace mai::mem;
using namespace mai::util;

HOOK_DELAYED(
    send_cloud_proxy_transfer_request,
    void*             a1,
    uint32_t          task_id,
    const LLVMString& content,
    void*             callback
) {
    DBG("send_cloud_proxy_transfer_request, task_id = {}, content = {}",
        task_id,
        content.view());
    std::string               protobuf;
    CloudProxyTransferRequest request;
    std::span<const char>     req_span;
    WPKGHeader                header;
    std::string               decompressed_proto(1024, '\0');
    int                       decompressed_size;
    // clang-format off
    auto iterate_wpkg = []<uint32_t... Keys>(std::integer_sequence<uint32_t, Keys...>, const WPKGHeader& header) {
        ([&]() {
            constexpr auto Key = Keys + 1;
            if (auto uint64 = header.get_uint64<Key>()) {
                DBG("wpkg: {} -> {}", Key, *uint64);
            }
            if (auto string = header.get_string<Key>()) {
                DBG("wpkg: {} -> '{}' or '{}'", Key, *string, string::hex(*string));
            }
        }(), ...);
    };
    // clang-format on
    try {
        auto json = nlohmann::json::parse(content.view());
        if (json.contains("req_body_encode")) {
            protobuf = base64::from_base64(
                json.at("req_body_encode").get<std::string_view>()
            );
        }
    } catch (const nlohmann::json::exception& e) {
        DBG("failed to parse json: {}", e.what());
        goto RETURN;
    }
    if (!request.ParseFromString(protobuf)) {
        DBG("failed to parse request {}!", task_id);
        goto RETURN;
    }
    DBG("parsed request {}\n{}", task_id, request.DebugString());
    if (!request.has_cmd_id()
        || request.cmd_id() != CLOUD_PROXY_TRANSFER_CMD_ID) {
        goto RETURN;
    }
    req_span = std::span(request.payload());
    if (req_span.size() < CLOUD_PROXY_SHORTCONN_HEADER_SIZE) {
        DBG("illegal request?");
        goto RETURN;
    }
    DBG("parsed shortconn header {}",
        string::hex(req_span.first(CLOUD_PROXY_SHORTCONN_HEADER_SIZE)));
    req_span = req_span.subspan(CLOUD_PROXY_SHORTCONN_HEADER_SIZE);
    if (header.deserialize(req_span) != 0 || header.size() > req_span.size()) {
        DBG("failed to parse wpkg header.");
        goto RETURN;
    }
    DBG("parsed wpkg header {}", string::hex(req_span.first(header.size())));
    iterate_wpkg(std::make_integer_sequence<uint32_t, 64>{}, header);
    req_span          = req_span.subspan(header.size()); // LZ compressed proto
    decompressed_size = LZ4_decompress_safe(
        req_span.data(),
        decompressed_proto.data(),
        req_span.size(),
        decompressed_proto.size()
    );
    if (decompressed_size < 0) {
        DBG("lz4 decompress failed, result = {}", decompressed_size);
        goto RETURN;
    }
    decompressed_proto.resize(decompressed_size);
    DBG("parsed decompressed proto {}", string::hex(decompressed_proto));
RETURN:
    return origin(a1, task_id, content, callback);
}

HOOK_DELAYED(
    send_cloud_proxy_auth_request,
    void*             a1,
    uint32_t          task_id,
    const LLVMString& content,
    void*             callback
) {
    DBG("send_cloud_proxy_auth_request, task_id = {}, content = {}",
        task_id,
        content.view());
    return origin(a1, task_id, content, callback);
}
