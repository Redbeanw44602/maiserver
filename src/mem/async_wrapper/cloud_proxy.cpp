// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <future>

#include <nlohmann/json.hpp>

#include "config.h"
#include "log.h"
#include "mem/async_wrapper/cloud_proxy.h"
#include "mem/function.h"
#include "mem/signal.h"
#include "util/base64.h"

#include "micromsg.pb.h"

using namespace mai;
using namespace mai::mem;
using namespace mai::util;

namespace mai::mem::async_wrapper::cloud_proxy {

namespace {

static std::mutex                                              mutex_;
static std::unordered_map<uint32_t, std::promise<std::string>> pending_;
static std::atomic<uint32_t> magic_task_id_ = 114514'1919;

std::future<std::string>
send_request(void* service, auto&& send_func, std::string&& payload) {
    std::promise<std::string> promise;
    auto                      future = promise.get_future();

    auto task_id = magic_task_id_.fetch_add(1, std::memory_order_relaxed);
    auto task    = nlohmann::json{
        {"cloud_task_id",   task_id                },
        {"long_polling",    false                  },
        {"req_body_encode", base64::encode(payload)},
        {"timeout_ms",      10000                  }
    };

    {
        std::lock_guard lock(mutex_);
        pending_.emplace(task_id, std::move(promise));
    }

    send_func(
        service,
        task_id,
        LLVMString(task.dump()),
        nullptr // callback
    );

    return future;
};

} // namespace

SessionInfoOrErr auth() {
    auto service = get_browsing_service();
    if (!service) {
        return std::unexpected(MaiError::FAILED_TO_GET_BROWSING_SERVICE);
    }

    CloudProxyAuthRequest auth_request;
    auto                  base_request = auth_request.mutable_request();
    base_request->set_device_id(g_cloud_proxy_device_id);
    std::string serialized;
    if (!auth_request.SerializeToString(&serialized)) {
        return std::unexpected(MaiError::FAILED_TO_SERIALIZE_REQUEST_PROTOBUF);
    }

    auto result = send_request(
        service,
        send_cloud_proxy_auth_request,
        std::move(serialized)
    );
    auto json = nlohmann::json::parse(result.get());
    if (!json.contains("client_auth_resp_encode") // todo: check err_code?
        || !json.at("client_auth_resp_encode").is_string()) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    auto proto_str = base64::decode(
        json.at("client_auth_resp_encode").get<std::string_view>()
    );
    if (!proto_str) {
        return proto_str;
    }

    CloudProxyAuthResponse response;
    if (!response.ParseFromString(*proto_str)) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF);
    }
    if (!response.has_unk2() || !response.unk2().has_unk2()) {
        return std::unexpected(MaiError::MISSING_SESSION_INFO);
    }

    return response.unk2().unk2().session_info();
}

PayloadOrErr transfer(
    uint32_t    cmd_id,
    std::string cmd_host,
    std::string cmd_url,
    std::string payload
) {
    auto service = get_browsing_service();
    if (!service) {
        return std::unexpected(MaiError::FAILED_TO_GET_BROWSING_SERVICE);
    }

    CloudProxyTransferRequest request;
    request.set_val1(1901);
    request.set_cmd_id(cmd_id);
    request.set_cmd_host(std::move(cmd_host));
    request.set_cmd_url(std::move(cmd_url));
    request.set_payload(std::move(payload));

    DBG("start to serialize cloud proxy transfer request");
    std::string serialized;
    if (!request.SerializeToString(&serialized)) {
        return std::unexpected(MaiError::FAILED_TO_SERIALIZE_REQUEST_PROTOBUF);
    }

    auto result = send_request(
        service,
        send_cloud_proxy_transfer_request,
        std::move(serialized)
    );
    auto json = nlohmann::json::parse(result.get());
    if (!json.contains("ilink_response_encode") // todo: check err_code?
        || !json.at("ilink_response_encode").is_string()) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    auto proto_str = base64::decode(
        json.at("ilink_response_encode").get<std::string_view>()
    );
    if (!proto_str) {
        return proto_str;
    }
    DBG("got valid ilink response, now deserialize");

    CloudProxyTransferResponse response;
    if (!response.ParseFromString(*proto_str)) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF);
    }
    if (!response.has_payload()
        || response.payload().size() < CLOUD_PROXY_SHORTCONN_HEADER_SIZE) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    DBG("got cloud proxy transfer response {}",
        string::hex(
            std::span(response.payload())
                .subspan(CLOUD_PROXY_SHORTCONN_HEADER_SIZE)
        ));
    return std::move(response.payload())
        .substr(CLOUD_PROXY_SHORTCONN_HEADER_SIZE);
}

} // namespace mai::mem::async_wrapper::cloud_proxy

HOOK_DELAYED(
    run_cloud_proxy_callback,
    void*             a1,
    uint32_t          task_id,
    const LLVMString& content
) {
    using namespace async_wrapper::cloud_proxy;
    DBG("run_cloud_proxy_callback, task_id = {}, content = {}",
        task_id,
        content.view());
    {
        std::lock_guard lock(mutex_);
        if (auto it = pending_.find(task_id); it != pending_.end()) {
            it->second.set_value(std::string(content.view()));
            pending_.erase(it);
        }
    }
    origin(a1, task_id, content);
}

HOOK_DELAYED(
    insert_cloud_proxy_callback,
    void*    a1,
    uint32_t task_id,
    void*    a3
) {
    using namespace async_wrapper::cloud_proxy;
    bool contains;
    {
        std::lock_guard lock(mutex_);
        contains = pending_.contains(task_id);
    }
    return contains ? nullptr : origin(a1, task_id, a3);
}

static CTOR_PRIORITY_MIDDLE void init() {
    on_lib_wmpf_host_export_ready.connect([]() {
        HOOK_INSTALL(run_cloud_proxy_callback);
        HOOK_INSTALL(insert_cloud_proxy_callback);
    });
}
