// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "error.h"
#include "log.h"
#include "service/maimai-dx/types.h"
#include "util/string.h"

namespace mai::service {

constexpr auto BROWSER_USER_AGENT =
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/144.0.0.0 Safari/537.36 NetType/WIFI "
    "MicroMessenger/7.0.20.1781(0x6700143B) WindowsWechat(0x63090a13) "
    "UnifiedPCLinuxWechat(0xf2741d1a) XWEB/25806 Flue";

// clang-format off
namespace url {
constexpr auto HOME = "https://maimai.wahlap.com/maimai-mobile/home/";
constexpr auto HOME_RATING_TARGET_MUSIC = "https://maimai.wahlap.com/maimai-mobile/home/ratingTargetMusic/";
}
// clang-format on

inline std::expected<ScoreDifficulty, MaiError>
score_difficulty_from_url(std::string_view url) {
    if (url.ends_with("diff_basic.png")) {
        return ScoreDifficulty::BASIC;
    } else if (url.ends_with("diff_advanced.png")) {
        return ScoreDifficulty::ADVANCED;
    } else if (url.ends_with("diff_expert.png")) {
        return ScoreDifficulty::EXPERT;
    } else if (url.ends_with("diff_master.png")) {
        return ScoreDifficulty::MASTER;
    } else if (url.ends_with("diff_remaster.png")) {
        return ScoreDifficulty::RE_MASTER;
    } else {
        DBG("unexpected webpage at {}", __FUNCTION__);
        return std::unexpected(
            MaiError::FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE
        );
    }
}

inline std::expected<ScoreKind, MaiError>
score_kind_from_url(std::string_view url) {
    if (url.ends_with("standard.png")) {
        return ScoreKind::STANDARD;
    } else if (url.ends_with("dx.png")) {
        return ScoreKind::DX;
    } else {
        DBG("unexpected webpage at {}", __FUNCTION__);
        return std::unexpected(
            MaiError::FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE
        );
    }
}

inline std::expected<ScoreLevel, MaiError>
score_level_from_str(std::string_view str) { // 12, 12+, 12+?
    ScoreLevel result{};
    if (str.ends_with("?")) {
        result.is_unknown = true;
        str.remove_suffix(1);
    }
    if (str.ends_with("+")) {
        result.is_plus = true;
        str.remove_suffix(1);
    }
    auto level = util::string::to_numeric<uint32_t>(str);
    if (!level) {
        return std::unexpected(level.error());
    }
    result.level_num = *level;
    return result;
}

inline std::expected<double, MaiError> score_from_str(std::string_view str) {
    str.remove_suffix(1); // %
    return util::string::to_numeric<double>(str);
}

} // namespace mai::service
