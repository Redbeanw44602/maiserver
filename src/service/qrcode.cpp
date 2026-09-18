// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <algorithm>

#include <pugixml.hpp>

#include "mem/function.h"
#include "mem/struct.h"
#include "service/qrcode.h"
#include "service/service.h"
#include "util/string.h"
#include "util/time.h"

using namespace std::chrono_literals;
using namespace mai::util;
using namespace mai::mem;

namespace mai::service {

static SimpleChannel channel;

std::expected<MaiQRCode, MaiError> qrcode() {
    if (auto fail = check(channel)) {
        return fail;
    }
    send_biz_menu_event(
        nullptr,
        1,
        LLVMStringNA(SERVICE_ACCOUNT_GH_USERNAME),
        SERVICE_ACCOUNT_MENU_ID,
        LLVMStringNA(SERVICE_ACCOUNT_MENU_KEY)
    );
    auto event = channel.wait_for_message(15s);
    if (!event) {
        return std::unexpected(MaiError::TIMEOUT);
    }

    pugi::xml_document xml;
    if (!xml.load_string(event->c_str())) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_XML_DOCUMENT);
    }

    std::string url, description;
    if (auto appmsg_node = xml.select_node("/msg/appmsg")) {
        url         = appmsg_node.node().child("url").text().as_string();
        description = appmsg_node.node().child("des").text().as_string();
    }
    if (url.empty()) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_XML_URL_NOT_FOUND);
    }
    if (description.empty()) {
        return std::unexpected(
            MaiError::FAILED_TO_PARSE_XML_DESCRIPTION_NOT_FOUND
        );
    }

    auto maid = string::middle(url, "/qrcode/req/", ".html");
    if (maid.empty()) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_XML_MAID_NOT_FOUND);
    }

    auto expires_in = time::to_timestamp(string::right(description, " : "));
    if (!expires_in) {
        return std::unexpected(
            MaiError::FAILED_TO_PARSE_XML_EXPIRY_TIME_NOT_FOUND
        );
    }

    return MaiQRCode{url, description, std::string(maid), *expires_in};
}

} // namespace mai::service

HOOK(
    parse_newsync_messages,
    /* Since the std::vector structure in libc++/stdc++ is extremely similar, it
       is used directly here; perhaps a std_vector.h will be added in the
       future. */
    const std::vector<MsgItem>& items,
    void*                       a2,
    void*                       a3
) {
    using PStr = ProtoString;

    origin(items, a2, a3);

    if (auto item = std::ranges::find_if(
            items,
            [](auto item) {
                return PStr::Read(item.sender_username)
                        == mai::SERVICE_ACCOUNT_GH_USERNAME
                    && PStr::Read(item.content).contains("qrcode/req/MAID");
            }
        );
        item != items.end()) {
        mai::service::channel.post_message(
            std::string(PStr::Read(item->content))
        );
    }
}
