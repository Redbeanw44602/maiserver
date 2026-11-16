// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "log.h"
#include "service/maimai-dx/helper.h"
#include "service/maimai-dx/rate_limiter.h"
#include "util/string.h"

using namespace mai::util;

namespace mai::service::rate_limiter {

namespace {

using TimePoint = std::chrono::steady_clock::time_point;

struct Window {
    bool      activated;
    TimePoint activated_on;
    size_t    request_count;


    Window() { reset(); }

    void reset() {
        activated     = false;
        activated_on  = {};
        request_count = 0;
    }

    void activate(TimePoint now) {
        activated     = true;
        activated_on  = now;
        request_count = 0;
    }

    template <typename Duration>
    TimePoint live_until(Duration lifetime) {
        return activated_on + lifetime;
    }

    bool is_blocked(size_t request_limit) const {
        return activated && request_count >= request_limit;
    }
};

struct PathState {
    Window short_window;
};

constexpr auto LONG_WINDOW_LIFETIME  = std::chrono::seconds(60);
constexpr auto SHORT_WINDOW_LIFETIME = std::chrono::seconds(5);

constexpr size_t LONG_WINDOW_LIMIT  = 120;
constexpr size_t SHORT_WINDOW_LIMIT = 10;

static Window                                     long_window_;
static std::unordered_map<std::string, PathState> paths_;
static std::mutex                                 mutex_;

template <typename Duration>
void check_window_expired(Window& window, TimePoint now, Duration lifetime) {
    if (window.activated && now >= window.live_until(lifetime)) {
        window.reset();
    }
}

std::string_view get_pathname_from_url(std::string_view url) {
    auto result = string::remove_prefix_r(url, url::ORIGIN);
    auto pos    = result.find_first_of("?#");

    if (pos != std::string_view::npos) {
        result = result.substr(0, pos);
    }

    return result;
}

} // namespace

void acquire(std::string_view url) {
    auto pathname = get_pathname_from_url(url);

    std::unique_lock lock(mutex_);

    while (true) {
        DBG("'{}': acquired mutex", pathname);

        auto now = std::chrono::steady_clock::now();

        auto& long_window  = long_window_;
        auto& short_window = paths_[std::string(pathname)].short_window;

        check_window_expired(long_window, now, LONG_WINDOW_LIFETIME);
        check_window_expired(short_window, now, SHORT_WINDOW_LIFETIME);

        const auto long_blocked  = long_window.is_blocked(LONG_WINDOW_LIMIT);
        const auto short_blocked = short_window.is_blocked(SHORT_WINDOW_LIMIT);

        if (!long_blocked && !short_blocked) {
            DBG("'{}': clearance, l: {}/{} s: {}/{}",
                pathname,
                long_window.request_count,
                LONG_WINDOW_LIMIT,
                short_window.request_count,
                SHORT_WINDOW_LIMIT);

            if (!long_window.activated) long_window.activate(now);
            if (!short_window.activated) short_window.activate(now);

            ++long_window.request_count;
            ++short_window.request_count;

            return;
        }

        TimePoint wake_at;

        if (long_blocked && short_blocked) {
            wake_at = std::max(
                long_window_.live_until(LONG_WINDOW_LIFETIME),
                short_window.live_until(SHORT_WINDOW_LIFETIME)
            );
        } else if (long_blocked) {
            wake_at = long_window_.live_until(LONG_WINDOW_LIFETIME);
        } else {
            wake_at = short_window.live_until(SHORT_WINDOW_LIFETIME);
        }

        DBG("'{}': rate limit exceeced (l={}, s={}), wait {}",
            pathname,
            long_blocked,
            short_blocked,
            std::chrono::duration_cast<std::chrono::seconds>(wake_at - now));

        // std::unique_lock is an RAII wrapper, so don't forget to unlock it!
        lock.unlock();

        std::this_thread::sleep_until(wake_at);

        lock.lock();
    }
}

} // namespace mai::service::rate_limiter