// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string_view>

namespace mai {

enum class MaiError {
    NO_ERROR,
    BUSY,
    TIMEOUT,
    ILLEGAL_RESPONSE,
    EXPECT_REDIRECT,
    WECHAT_NOT_LOGGED_IN,
    MISSING_SESSION_INFO,
    MISSING_OAUTH_CALLBACK_URL,
    FAILED_TO_GET_BROWSING_SERVICE,
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
};

inline std::string_view to_string(MaiError error) {
    switch (error) {
    case MaiError::BUSY:
        return "Currently busy; there is another request waiting.";
    case MaiError::TIMEOUT:
        return "The request timed out.";
    case MaiError::ILLEGAL_RESPONSE:
        return "Unexpected response.";
    case MaiError::EXPECT_REDIRECT:
        return "Expected a redirect, but didn't get location.";
    case MaiError::WECHAT_NOT_LOGGED_IN:
        return "Please log in to WeChat first.";
    case MaiError::MISSING_SESSION_INFO:
        return "Session info is missing in the response.";
    case MaiError::MISSING_OAUTH_CALLBACK_URL:
        return "OAuth callback url is missing in the response";
    case MaiError::FAILED_TO_GET_BROWSING_SERVICE:
        return "Unable to get the browsing service.";
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
    default:
        return "Unknown error.";
    }
}

} // namespace mai
