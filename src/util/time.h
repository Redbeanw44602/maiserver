// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <chrono>
#include <format>
#include <sstream>

/* libc++ has not yet fully implemented P0355R7, see:
 * https://github.com/llvm/llvm-project/issues/99982 */
#include <date/tz.h>

namespace mai::util::time {

inline std::optional<int64_t> to_timestamp(std::string_view time_str) {
    using namespace date;

    auto tz = locate_zone("Asia/Shanghai");

    auto sys_now   = std::chrono::system_clock::now();
    auto local_now = tz->to_local(sys_now);

    auto ymd_now      = year_month_day{floor<days>(local_now)};
    auto current_year = static_cast<int>(ymd_now.year());

    auto full_time_str = std::format("{}/{}", current_year, time_str);

    local_seconds      target_local;
    std::istringstream iss{full_time_str};
    iss >> date::parse("%Y/%m/%d %H:%M", target_local);

    if (iss.fail()) {
        return std::nullopt;
    }

    if (target_local < local_now) {
        auto           dp = floor<days>(target_local);
        year_month_day ymd{dp};
        hh_mm_ss       time{target_local - dp};

        ymd = (ymd.year() + years{1}) / ymd.month() / ymd.day();

        target_local = local_days{ymd} + time.to_duration();
    }

    auto target_sys = tz->to_sys(target_local);

    return duration_cast<std::chrono::seconds>(target_sys.time_since_epoch())
        .count();
}

} // namespace mai::util::time
