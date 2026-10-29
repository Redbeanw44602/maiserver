// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <cpr/cpr.h>
#include <lz4.h>
#include <nlohmann/json.hpp>

#include "config.h"
#include "log.h"
#include "mem/function.h"
#include "mem/std_string.h"
#include "mem/wpkg_header.h"
#include "service/aimetoken.h"
#include "util/string.h"

#include "micromsg.pb.h"

using namespace mai;
using namespace mai::util;
using namespace mai::mem;
using namespace mai::mem::async_wrapper;

namespace mai::service {

namespace {

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
    base_request->set_val1(-227271398);
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

    std::string request;
    request.reserve(total_size);
    request.append_range(shortconn_header);
    request.append_range(wpkg_header);
    request.append_range(oauth_request);
    auto response = cloud_proxy::transfer(
        CLOUD_PROXY_TRANSFER_CMD_ID,
        CLOUD_PROXY_TRANSFER_CMD_HOST,
        CLOUD_PROXY_TRANSFER_CMD_URL,
        std::move(request)
    );
    if (!response) {
        return response;
    }
    auto resp_span = std::span(*response);

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

} // namespace
    DBG("called aimetoken(), start oauth process...");
    auto oauth_url = cpr::Get(
        cpr::Url{std::format("{}/{}", TGK_WCAIME_OAUTH_URL, service_name)},
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
        if (auto session_info = cloud_proxy::auth()) {
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
