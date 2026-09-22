// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <base64.hpp>
#include <cpr/cpr.h>
#include <lz4.h>
#include <nlohmann/json.hpp>

#include "config.h"
#include "log.h"
#include "mem/function.h"
#include "mem/std_string.h"
#include "mem/wpkg_header.h"
#include "service/aimetoken.h"
#include "service/service.h"
#include "util/simple_channel.h"
#include "util/string.h"

#include "micromsg.pb.h"

using namespace std::chrono_literals;
using namespace mai;
using namespace mai::util;
using namespace mai::mem;

namespace mai::service {

static SimpleChannel channel;

std::expected<std::string, MaiError> get_cloud_proxy_session_info() {
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
    nlohmann::json cloud_auth_task{
        {"cloud_task_id",   MAGIC_TASK_ID                },
        {"long_polling",    false                        },
        {"req_body_encode", base64::to_base64(serialized)},
        {"timeout_ms",      10000                        }
    };

    send_cloud_proxy_auth_request(
        service,
        MAGIC_TASK_ID,
        LLVMString(cloud_auth_task.dump()),
        nullptr // callback
    );
    auto result = channel.wait_for_message(10s);
    if (!result) {
        return std::unexpected(MaiError::TIMEOUT);
    }
    auto json = nlohmann::json::parse(*result);
    if (!json.contains("client_auth_resp_encode") // todo: check err_code?
        || !json.at("client_auth_resp_encode").is_string()) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    auto proto_str = base64::from_base64(
        json.at("client_auth_resp_encode").get<std::string_view>()
    );

    CloudProxyAuthResponse response;
    if (!response.ParseFromString(proto_str)) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF);
    }
    if (!response.has_unk2() || !response.unk2().has_unk2()) {
        return std::unexpected(MaiError::MISSING_SESSION_INFO);
    }

    return response.unk2().unk2().session_info();
}

std::expected<std::vector<char>, MaiError> build_oauth_request(
    std::string_view url,
    std::string_view device_id,
    uint32_t         uin,
    bool             enable_compression
) {
    OAuthAuthorizeRequest request;
    request.set_url(url);
    request.set_scene(0);
    request.set_referrer_url("");
    auto base_request = request.mutable_base_request();
    base_request->set_key_type("sessionkey");
    base_request->set_uin(uin);
    base_request->set_device_id(device_id);
    /* The meaning of this field is currently unknown, but it may be related to
     * the version. Update it when adapting to the new version. */
    base_request->set_val1(-227271415);
    base_request->set_client_os("UnifiedPCLinux");
    base_request->set_val2(0);
    std::vector<char> serialized(request.ByteSizeLong());
    if (!request.SerializeToArray(serialized.data(), serialized.size())) {
        return std::unexpected(MaiError::FAILED_TO_SERIALIZE_REQUEST_PROTOBUF);
    }
    if (enable_compression) {
        std::vector<char> compressed(LZ4_compressBound(serialized.size()));
        auto              compressed_size = LZ4_compress_default(
            serialized.data(),
            compressed.data(),
            serialized.size(),
            compressed.size()
        );
        if (compressed_size == 0) {
            return std::unexpected(MaiError::FAILED_TO_COMPRESS_REQUEST);
        }
        compressed.resize(compressed_size);
        return compressed;
    }
    return serialized;
}

auto build_wpkg_header(uint32_t uin, std::string_view session_info) {
    WPKGHeader header;
    header.set_uint64<1>(1);
    header.set_uint64<2>(uin);
    header.set_uint64<3>(0);
    header.set_uint64<4>(0);
    /* The meaning of this field is currently unknown, but it may be related to
     * the version. Update it when adapting to the new version. */
    header.set_uint64<5>(726016);
    header.set_uint64<6>(15);
    header.set_uint64<7>(0);
    header.set_uint64<8>(0);
    header.set_uint64<9>(0);
    header.set_uint64<10>(1);
    header.set_uint64<11>(0);
    header.set_uint64<12>(0);
    header.set_uint64<13>(0);
    header.set_string<14>("");
    // 15 is not set
    // 16 is not set
    header.set_uint64<17>(0);
    header.set_uint64<18>(1);
    // 19 is not set
    header.set_uint64<20>(1506);
    header.set_uint64<21>(0);
    header.set_uint64<22>(uin);
    header.set_uint64<23>(0);
    header.set_string<24>("wechat");
    header.set_uint64<25>(0);
    header.set_uint64<26>(4);
    header.set_string<27>(session_info);
    header.set_uint64<28>(1);
    header.set_uint64<29>(1);
    header.set_uint64<30>(0);
    header.set_uint64<31>(0);
    std::vector<char> ret(10248);
    auto              size = header.serialize(ret);
    ret.resize(size);
    return ret;
}

auto build_shortconn_header(uint32_t packet_size, uint32_t cmd_id) {
    uint32_t fields[] =
        {packet_size, /*magic=*/0x1110076d, cmd_id, /*reserved=*/0};
    static_assert(sizeof(fields) == CLOUD_PROXY_SHORTCONN_HEADER_SIZE);

    if constexpr (std::endian::native == std::endian::little) {
        for (auto& val : fields) {
            val = std::byteswap(val);
        }
    }

    std::array<char, sizeof(fields)> ret;
    std::memcpy(ret.data(), fields, sizeof(fields));

    return ret;
}

std::expected<std::string, MaiError>
get_oauth_callback_url(std::string_view url) {
    auto service = get_browsing_service();
    if (!service) {
        return std::unexpected(MaiError::FAILED_TO_GET_BROWSING_SERVICE);
    }
    DBG("get_oauth_callback_url called, start oauth process...");

    // Another device ID, related to the MAC address, which is different from
    // `g_cloud_proxy_device_id`.
    auto device_id = get_device_id();
    DBG("got another device id {}", device_id->view());

    auto oauth_request_or_err =
        build_oauth_request(url, device_id->view(), g_uin, true);
    if (!oauth_request_or_err) {
        DBG("failed to build oauth request!");
        return std::unexpected(oauth_request_or_err.error());
    }

    auto oauth_request = *oauth_request_or_err;
    auto wpkg_header   = build_wpkg_header(g_uin, g_cloud_proxy_session_info);
    auto total_size    = oauth_request.size() + wpkg_header.size()
                       + CLOUD_PROXY_SHORTCONN_HEADER_SIZE;
    auto shortconn_header =
        build_shortconn_header(total_size, CLOUD_PROXY_TRANSFER_CMD_ID);
    DBG("built oauth request ({}) {}",
        oauth_request_or_err->size(),
        string::hex(*oauth_request_or_err));
    DBG("built wpkg header ({}) {}",
        wpkg_header.size(),
        string::hex(wpkg_header));
    DBG("built shortconn header ({}) {}",
        shortconn_header.size(),
        string::hex(shortconn_header));

    CloudProxyTransferRequest request;
    request.set_val1(1901);
    request.set_cmd_id(CLOUD_PROXY_TRANSFER_CMD_ID);
    request.set_cmd_host(CLOUD_PROXY_TRANSFER_CMD_HOST);
    request.set_cmd_url(CLOUD_PROXY_TRANSFER_CMD_URL);
    auto payload = request.mutable_payload();
    payload->reserve(total_size);
    payload->append_range(shortconn_header);
    payload->append_range(wpkg_header);
    payload->append_range(oauth_request);

    DBG("start to serialize cloud proxy transfer request");
    std::string serialized;
    if (!request.SerializeToString(&serialized)) {
        return std::unexpected(MaiError::FAILED_TO_SERIALIZE_REQUEST_PROTOBUF);
    }

    nlohmann::json cloud_transfer_task{
        {"cloud_task_id",   MAGIC_TASK_ID                },
        {"long_polling",    false                        },
        {"req_body_encode", base64::to_base64(serialized)},
        {"timeout_ms",      30000                        }
    };
    DBG("send cloud proxy transfer task {}", cloud_transfer_task.dump());

    send_cloud_proxy_transfer_request(
        service,
        MAGIC_TASK_ID,
        LLVMString(cloud_transfer_task.dump()),
        nullptr // callback
    );
    auto result = channel.wait_for_message(30s);
    if (!result) {
        return std::unexpected(MaiError::TIMEOUT);
    }
    auto json = nlohmann::json::parse(*result);
    if (!json.contains("ilink_response_encode") // todo: check err_code?
        || !json.at("ilink_response_encode").is_string()) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    auto proto_str = base64::from_base64(
        json.at("ilink_response_encode").get<std::string_view>()
    );
    DBG("got valid ilink response, now deserialize");

    CloudProxyTransferResponse response;
    if (!response.ParseFromString(proto_str)) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF);
    }
    if (!response.has_payload()
        || response.payload().size() < CLOUD_PROXY_SHORTCONN_HEADER_SIZE) {
        return std::unexpected(MaiError::ILLEGAL_RESPONSE);
    }
    auto resp_span = std::span(response.payload())
                         .subspan(CLOUD_PROXY_SHORTCONN_HEADER_SIZE);
    DBG("got cloud proxy transfer response {}", string::hex(resp_span));

    WPKGHeader header;
    if (header.deserialize(resp_span) != 0
        || header.size() > resp_span.size()) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_WPKG);
    }
    DBG("parsed wpkg header size {}", header.size());

    auto        compressed_proto = resp_span.subspan(header.size());
    std::string decompressed_proto(1024, '\0');
    auto        decompressed_size = LZ4_decompress_safe(
        compressed_proto.data(),
        decompressed_proto.data(),
        compressed_proto.size(),
        decompressed_proto.size()
    );
    if (decompressed_size >= 0) {
        decompressed_proto.resize(decompressed_size);
    } else {
        DBG("lz4 decompress failed, result = {}", decompressed_size);
        return std::unexpected(MaiError::FAILED_TO_DECOMPRESS_RESPONSE);
    }
    DBG("decompressed oauth response {}", string::hex(decompressed_proto));

    OAuthAuthorizeResponse oauth_response;
    if (!oauth_response.ParseFromString(decompressed_proto)) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF);
    }
    if (!oauth_response.has_callback_url()) {
        return std::unexpected(MaiError::MISSING_OAUTH_CALLBACK_URL);
    }

    return oauth_response.callback_url();
}

std::expected<AimeToken, MaiError> aimetoken() {
    if (auto fail = check(channel)) {
        return fail;
    }
    DBG("called aimetoken(), start oauth process...");
    auto oauth_url = cpr::Get(
        cpr::Url{WEIXIN_WAHLAP_AIME_URL},
        cpr::Redirect{false},
        cpr::HttpVersion{cpr::HttpVersionCode::VERSION_2_0}
#if MAI_DEBUG
        ,
        cpr::Verbose{true}
#endif
    );
    DBG("got oauth url from wahlap, status = {}, text = {}",
        oauth_url.status_code,
        oauth_url.text);
    if (oauth_url.status_code == 0) {
        DBG("curl error: {}", oauth_url.error.message);
    }

    if (!oauth_url.header.contains("location")) {
        DBG("walhap response does not contains location header!");
        return std::unexpected(MaiError::EXPECT_REDIRECT);
    }

    DBG("ok, start getting cloud_proxy_session_info");
    if (g_cloud_proxy_session_info.empty()) {
        if (auto session_info = get_cloud_proxy_session_info()) {
            DBG("got valid session info {}", string::hex(*session_info));
            g_cloud_proxy_session_info = *session_info;
        } else {
            return std::unexpected(session_info.error());
        }
    }

    DBG("now start getting oauth callback url...");
    auto url_or_err = get_oauth_callback_url(oauth_url.header.at("location"));
    if (!url_or_err) {
        return std::unexpected(url_or_err.error());
    }

    DBG("server responses valid callback_url = {}, now send request",
        *url_or_err);
    auto callback_url = cpr::Get(
        cpr::Url{*url_or_err},
        cpr::Redirect{false},
        cpr::HttpVersion{cpr::HttpVersionCode::VERSION_2_0}
    );
    DBG("wahlap responses, status = {}, text = {}",
        callback_url.status_code,
        callback_url.text);
    if (!callback_url.header.contains("location")) {
        return std::unexpected(MaiError::EXPECT_REDIRECT);
    }

    DBG("success!");
    return AimeToken{callback_url.header.at("location")};
}

} // namespace mai::service

HOOK_DELAYED(
    run_cloud_proxy_callback,
    void*             a1,
    uint32_t          task_id,
    const LLVMString& content
) {
    DBG("run_cloud_proxy_callback, task_id = {}, content = {}",
        task_id,
        content.view());
    if (task_id == MAGIC_TASK_ID) {
        service::channel.post_message(std::string(content.view()));
        return;
    }
    origin(a1, task_id, content);
}

HOOK_DELAYED(
    insert_cloud_proxy_callback,
    void*    a1,
    uint32_t task_id,
    void*    a3
) {
    return task_id == MAGIC_TASK_ID ? nullptr : origin(a1, task_id, a3);
}

/* frida-gum does not allow the same address to be hooked multiple times; we
 * plan to address this issue in the future */
#include "maiserver_slim.inc"

#if MAI_DEBUG
#include "maidebug_helper.inc"
#endif

HOOK(load_wmpf_host_export) {
    auto ret = origin();
    HOOK_INSTALL(run_cloud_proxy_callback);
    HOOK_INSTALL(insert_cloud_proxy_callback);
#if MAI_DEBUG
    HOOK_INSTALL(send_cloud_proxy_transfer_request);
    HOOK_INSTALL(send_cloud_proxy_auth_request);
#endif
    HOOK_INSTALL(init_browser);
    return ret;
}
