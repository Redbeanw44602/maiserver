// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <chrono>
#include <condition_variable>

namespace mai::util {

class SimpleChannel {
public:
    struct BusyGuard {
        bool& flag_;
        BusyGuard(bool& flag) : flag_(flag) { flag_ = true; }
        ~BusyGuard() { flag_ = false; }
    };

    void post_message(std::string msg) {
        {
            std::lock_guard lock(mtx_);
            message_ = std::move(msg);
        }
        cv_.notify_one();
    }

    template <typename Rep, typename Period>
    std::optional<std::string>
    wait_for_message(std::chrono::duration<Rep, Period> timeout) {
        std::unique_lock lock(mtx_);

        if (is_busy_) {
            // todo, print warning?
            return std::nullopt;
        }
        BusyGuard guard(is_busy_);

        message_.reset();
        auto received = cv_.wait_for(lock, timeout, [this] {
            return message_.has_value();
        });

        if (received) {
            return std::move(message_);
        }
        return std::nullopt;
    }

    bool is_busy() const {
        std::lock_guard lock(mtx_);
        return is_busy_;
    }

private:
    mutable std::mutex         mtx_;
    std::condition_variable    cv_;
    std::optional<std::string> message_;
    bool                       is_busy_{false};
};

} // namespace mai::util
