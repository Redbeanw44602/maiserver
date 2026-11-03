// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <expected> // IWYU pragma: export
#include <optional>
#include <string_view>

namespace mai {

enum class MaiError {
    NO_ERROR,
    TIMEOUT,
    ILLEGAL_REQUEST,
    ILLEGAL_RESPONSE,
    EXPECT_REDIRECT,
    EXPECT_OK,
    WECHAT_NOT_LOGGED_IN,
    MISSING_SESSION_INFO,
    MISSING_OAUTH_CALLBACK_URL,
    COMMAND_NOT_FOUND,
    ENVIRONMENT_VARIABLE_NOT_FOUND,
    FAILED_TO_GET_BROWSING_SERVICE,
    FAILED_TO_TIDY_HTML_DOCUMENT,
    FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE,
    FAILED_TO_PARSE_XML_DOCUMENT,
    FAILED_TO_PARSE_XML_URL_NOT_FOUND,
    FAILED_TO_PARSE_XML_DESCRIPTION_NOT_FOUND,
    FAILED_TO_PARSE_XML_EXPIRY_TIME_NOT_FOUND,
    FAILED_TO_PARSE_XML_MAID_NOT_FOUND,
    FAILED_TO_SERIALIZE_REQUEST_PROTOBUF,
    FAILED_TO_PARSE_RESPONSE_PROTOBUF,
    FAILED_TO_PARSE_RESPONSE_WPKG,
    FAILED_TO_COMPRESS_REQUEST,
    FAILED_TO_DECOMPRESS_RESPONSE,
    FAILED_TO_CONVERSION_INVALID_STR,
    FAILED_TO_CONVERSION_OUT_OF_RANGE,
    FAILED_TO_DECODE_BASE64_INVALID_INPUT,
    FAILED_TO_DECODE_BASE64_DECODER_NOT_FOUND,
    FAILED_TO_LOGIN_TO_MAIMAI_DX_NET,
};

inline std::string_view to_string(MaiError error) {
    switch (error) {
    case MaiError::TIMEOUT:
        return "The request timed out.";
    case MaiError::ILLEGAL_REQUEST:
        return "Illegal request.";
    case MaiError::ILLEGAL_RESPONSE:
        return "Unexpected response.";
    case MaiError::EXPECT_REDIRECT:
        return "Expected a redirect, but didn't get location.";
    case MaiError::EXPECT_OK:
        return "An unexpected HTTP code was returned; the request failed.";
    case MaiError::WECHAT_NOT_LOGGED_IN:
        return "Please log in to WeChat first.";
    case MaiError::MISSING_SESSION_INFO:
        return "Session info is missing in the response.";
    case MaiError::COMMAND_NOT_FOUND:
        return "Command not found.";
    case MaiError::ENVIRONMENT_VARIABLE_NOT_FOUND:
        return "Environment variable not found.";
    case MaiError::MISSING_OAUTH_CALLBACK_URL:
        return "OAuth callback url is missing in the response";
    case MaiError::FAILED_TO_GET_BROWSING_SERVICE:
        return "Unable to get the browsing service.";
    case MaiError::FAILED_TO_TIDY_HTML_DOCUMENT:
        return "Failed to tidy the HTML document.";
    case MaiError::FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE:
        return "Failed to parse HTML; Unexpected webpage.";
    case MaiError::FAILED_TO_PARSE_XML_DOCUMENT:
        return "Failed to parse the XML document.";
    case MaiError::FAILED_TO_PARSE_XML_URL_NOT_FOUND:
        return "Failed to parse the url from the XML document.";
    case MaiError::FAILED_TO_PARSE_XML_DESCRIPTION_NOT_FOUND:
        return "Failed to parse the description from the XML document.";
    case MaiError::FAILED_TO_PARSE_XML_EXPIRY_TIME_NOT_FOUND:
        return "Failed to parse the expiry time from the XML document.";
    case MaiError::FAILED_TO_PARSE_XML_MAID_NOT_FOUND:
        return "Failed to parse the MAID from the XML document.";
    case MaiError::FAILED_TO_SERIALIZE_REQUEST_PROTOBUF:
        return "Failed to serialize the request. (Protobuf)";
    case MaiError::FAILED_TO_PARSE_RESPONSE_PROTOBUF:
        return "Failed to deserialize the response. (Protobuf)";
    case MaiError::FAILED_TO_PARSE_RESPONSE_WPKG:
        return "Failed to deserialize the response. (Wpkg)";
    case MaiError::FAILED_TO_COMPRESS_REQUEST:
        return "Failed to compress the request.";
    case MaiError::FAILED_TO_DECOMPRESS_RESPONSE:
        return "Failed to decompress the response.";
    case MaiError::FAILED_TO_CONVERSION_INVALID_STR:
        return "Invalid string; cannot be converted to an integer.";
    case MaiError::FAILED_TO_CONVERSION_OUT_OF_RANGE:
        return "An integer overflow has been detected.";
    case MaiError::FAILED_TO_LOGIN_TO_MAIMAI_DX_NET:
        return "Failed to login to maimai DX NET.";
    default:
        return "Unknown error.";
    }
}

template <typename E, typename... Ts>
std::optional<E> check_multiple_expected(std::expected<Ts, E>&... args) {
    E* first_error = nullptr;

    auto check = [&first_error](auto& arg) -> bool {
        if (!arg) {
            first_error = std::addressof(arg.error());
            return true;
        }
        return false;
    };

    (void)(check(args) || ...);

    if (first_error) {
        return std::make_optional(std::move(*first_error));
    }

    return {};
}

} // namespace mai
