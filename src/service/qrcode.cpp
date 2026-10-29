// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <algorithm>
#include <future>

#include <pugixml.hpp>

#include "config.h"
#include "log.h"
#include "mem/function.h"
#include "mem/struct.h"
#include "service/qrcode.h"
#include "util/string.h"
#include "util/time.h"

using namespace mai::util;
using namespace mai::mem;

namespace mai::service {

namespace {

struct Task {
    std::promise<MaiQRCodeOrErr>          promise;
    std::shared_future<MaiQRCodeOrErr>    future = promise.get_future().share();
    std::chrono::steady_clock::time_point deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(15);
};

static std::shared_ptr<Task> shared_task_;
static std::mutex            mutex_;

std::expected<void, MaiError> send_request() {
    if (g_uin == 0) {
        return std::unexpected(MaiError::WECHAT_NOT_LOGGED_IN);
    }
    send_biz_menu_event(
        nullptr,
        1,
        LLVMStringNA(SERVICE_ACCOUNT_GH_USERNAME),
        SERVICE_ACCOUNT_MENU_ID,
        LLVMStringNA(SERVICE_ACCOUNT_MENU_KEY)
    );
    DBG("biz menu event sent");
    return {};
}

MaiQRCodeOrErr parse_xml_message(std::string xml_str) {
    pugi::xml_document xml;
    if (!xml.load_string(xml_str.c_str())) {
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

} // namespace

std::expected<void, MaiError> request_qrcode_async() {
    DBG("called request_qrcode() - async, start process...");
    std::lock_guard lock(mutex_);
    if (!shared_task_) {
        DBG("initializing global shared task...");
        auto result = send_request();
        if (!result) {
            return result;
        }
        shared_task_ = std::make_shared<Task>();
    }
    DBG("success, just return...");
    return {};
}

MaiQRCodeOrErr request_qrcode() {
    DBG("called request_qrcode_sync(), start process...");
    std::shared_ptr<Task> task;

    {
        std::lock_guard lock(mutex_);
        if (!shared_task_) {
            DBG("initializing global shared task...");
            auto result = send_request();
            if (!result) {
                return std::unexpected(result.error());
            }
            shared_task_ = std::make_shared<Task>();
        }
        DBG("copying task ptr");
        task = shared_task_;
    }

    DBG("start waiting for response, or timeout");
    if (task->future.wait_until(task->deadline)
        == std::future_status::timeout) {
        DBG("timeout exceeded!");
        std::lock_guard lock(mutex_);
        if (shared_task_ == task) {
            DBG("set value = TIMEOUT and reset global shared task...");
            /* Only the thread that times out (wakes up) first and successfully
             * acquires the mutex will reach this point. */
            task->promise.set_value(std::unexpected(MaiError::TIMEOUT));
            shared_task_.reset();
        }
    }

    DBG("get future!");
    return task->future.get();
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
    using namespace mai;
    using namespace mai::service;

    origin(items, a2, a3);

    if (auto item = std::ranges::find_if(
            items,
            [](auto item) {
                return PStr::Read(item.sender_username)
                        == SERVICE_ACCOUNT_GH_USERNAME
                    && PStr::Read(item.content).contains("qrcode/req/MAID");
            }
        );
        item != items.end()) {
        DBG("matched message received, parsing xml...");
        auto result = parse_xml_message(std::string(PStr::Read(item->content)));
        DBG("emit on_qrcode_received");
        on_qrcode_received(result);
        {
            std::lock_guard lock(mutex_);
            DBG("mutex acquired");
            if (shared_task_) {
                DBG("set result and reset global shared task...");
                shared_task_->promise.set_value(result);
                shared_task_.reset();
            }
        }
    }
}
