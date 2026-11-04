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

#include "service/maimai-dx/session.h" // IWYU pragma: export
#include "util/html_parser.h"          // IWYU pragma: export

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

#define UNEXPECTED_WEBPAGE_WITH_ERR(x)                                         \
    DBG("unexpected webpage! error: {}", to_string(x.error()));                \
    return std::unexpected(MaiError::FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE)

#define UNEXPECTED_WEBPAGE                                                     \
    DBG("unexpected webpage!");                                                \
    return std::unexpected(MaiError::FAILED_TO_PARSE_HTML_UNEXPECTED_WEBPAGE)

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
        UNEXPECTED_WEBPAGE;
    }
}

inline std::expected<ScoreKind, MaiError>
score_kind_from_url(std::string_view url) {
    if (url.ends_with("standard.png")) {
        return ScoreKind::STANDARD;
    } else if (url.ends_with("dx.png")) {
        return ScoreKind::DX;
    } else {
        UNEXPECTED_WEBPAGE;
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
        UNEXPECTED_WEBPAGE_WITH_ERR(level);
    }
    result.level_num = *level;
    return result;
}

inline std::expected<double, MaiError> score_from_str(std::string_view str) {
    str.remove_suffix(1); // %
    auto result = util::string::to_numeric<double>(str);
    if (!result) {
        UNEXPECTED_WEBPAGE_WITH_ERR(result);
    }
    return result;
}

inline std::expected<uint32_t, MaiError>
star_count_from_str(std::string_view str) {
    str.remove_prefix(2); // '×' utf8 (0xC3 0x97)
    auto result = util::string::to_numeric<uint32_t>(str);
    if (!result) {
        UNEXPECTED_WEBPAGE_WITH_ERR(result);
    }
    return result;
}

// Sources:
// https://github.com/FukutoTojido/Narukami-Picker/blob/837874cd3637fcd96f7075db883780c5c938f09b/src/json/teams_.json
// https://github.com/TrueRou/maimai.py/blob/f88d3a9c48103111c9c94c942b5f44fea736263e/tests/sample_data/friend.html
// https://github.com/team-carol/carol/blob/2e70afca2f958b8672e10957e7c78fcae3e9b400/src/rankImages.ts
// https://github.com/leomotors/chuumai-tools/blob/c1d699bc789001ed126d7f9b2ddccd8e8daa1eb8/apps/maimai-scraper/src/parser/__fixtures__/playerData/rate16.html
// https://github.com/bakapiano/maimai-score-hub/blob/c3421a33a7510f811da51ed86cae00d44970bfb1/backend/src/modules/stats-card/stats-card.spec.ts
//
// 初段   /maimai-mobile/img/course/course_rank_00T7GHJvGe.png
// 二段
// 三段
// 四段
// 五段
// 六段   /maimai-mobile/img/course/course_rank_06q7xiUXeF.png
// 七段   /maimai-mobile/img/course/course_rank_07zaVOj56m.png
// 八段   /maimai-mobile/img/course/course_rank_08paNFJX5Y.png
// 九段   /maimai-mobile/img/course/course_rank_09eKhcbFNK.png
// 十段   /maimai-mobile/img/course/course_rank_10hvsSHd90.png
// 真初段 /maimai-mobile/img/course/course_rank_12qWgEcuKi.png
// 真二段 /maimai-mobile/img/course/course_rank_13KOI1uBwE.png
// 真三段
// 真四段 /maimai-mobile/img/course/course_rank_15DlEMWlRz.png
// 真五段
// 真六段 /maimai-mobile/img/course/course_rank_17VkB7xyBD.png
// 真七段 /maimai-mobile/img/course/course_rank_18xKRpcFc3.png
// 真八段
// 真九段 /maimai-mobile/img/course/course_rank_20DkWvoJjc.png
// 真十段
// 真皆传
// 里皆传

inline std::expected<std::string, MaiError>
course_rank_from_url(std::string_view url) {
    static const std::unordered_map<std::string_view, std::string_view>
         mapping = {
             {"00", "初段"  },
             {"02", "二段"  },
             {"03", "三段"  },
             {"04", "四段"  },
             {"05", "五段"  },
             {"06", "六段"  },
             {"07", "七段"  },
             {"08", "八段"  },
             {"09", "九段"  },
             {"10", "十段"  },
             {"12", "真初段"},
             {"13", "真二段"},
             {"14", "真三段"},
             {"15", "真四段"},
             {"16", "真五段"},
             {"17", "真六段"},
             {"18", "真七段"},
             {"19", "真八段"},
             {"20", "真九段"},
             {"21", "真十段"},
             {"22", "真皆传"},
             {"23", "里皆传"},
    };
    auto str = util::string::right(url, "/course_rank_");
    if (str.size() < 2) {
        UNEXPECTED_WEBPAGE;
    }
    DBG("course_rank_url={}, crop: '{}'", url, str);
    if (auto it = mapping.find(str.substr(0, 2)); it != mapping.end()) {
        return static_cast<std::string>(it->second);
    }
    UNEXPECTED_WEBPAGE;
}

// Sources:
// https://github.com/FukutoTojido/Narukami-Picker/blob/837874cd3637fcd96f7075db883780c5c938f09b/src/json/teams_.json
// https://github.com/michioxd/betterDXnet/blob/19daa8a4ddf284e17ca370d034a2a6d9d85e7d27/src/api/me.ts
// https://github.com/otohime-site/parser/blob/7deafa8b7d23d2ee3fde8b24f884132412605306/tests/dx_intl.test.ts
// https://github.com/graycat0305/MaimaiB50Extension/blob/a4e7820c9e32559694a270bbd1af8f3d53760fab/example_b50_page.html
// https://github.com/team-carol/carol/blob/2e70afca2f958b8672e10957e7c78fcae3e9b400/src/rankImages.ts
// https://github.com/leomotors/chuumai-tools/blob/c1d699bc789001ed126d7f9b2ddccd8e8daa1eb8/apps/maimai-scraper/src/parser/__fixtures__/playerData/rate16.html
//
// B5     /maimai-mobile/img/class/class_rank_s_00ZqZmdpb8.png
// B4     /maimai-mobile/img/class/class_rank_s_01VFe8gl5z.png
// B3     /maimai-mobile/img/class/class_rank_s_02LdzHISU6.png
// B2
// B1
// A5
// A4     /maimai-mobile/img/class/class_rank_s_06IjZSkULI.png
// A3     /maimai-mobile/img/class/class_rank_s_07kRW9Mzgf.png
// A2     /maimai-mobile/img/class/class_rank_s_08R8oyTJTT.png
// A1     /maimai-mobile/img/class/class_rank_s_09XFJ50NEJ.png
// S5     /maimai-mobile/img/class/class_rank_s_10FODgtQo4.png
// S4     /maimai-mobile/img/class/class_rank_s_11vX3qUYdA.png
// S3     /maimai-mobile/img/class/class_rank_s_12ruJTrWoU.png
// S2     /maimai-mobile/img/class/class_rank_s_13UwBeQ8fU.png
// S1     /maimai-mobile/img/class/class_rank_s_14brd4ch6F.png
// SS5    /maimai-mobile/img/class/class_rank_s_15D7JMvMhJ.png
// SS4
// SS3
// SS2
// SS1
// SSS5   /maimai-mobile/img/class/class_rank_s_20OmGGNvGh.png
// SSS4
// SSS3   /maimai-mobile/img/class/class_rank_s_22enfEgsYk.png
// SSS2
// SSS1
// LEGEND

inline std::expected<ClassRank, MaiError>
class_rank_from_url(std::string_view url) {
    static const std::unordered_map<std::string_view, ClassRank> mapping = {
        {"00", ClassRank::B5    },
        {"01", ClassRank::B4    },
        {"02", ClassRank::B3    },
        {"03", ClassRank::B2    },
        {"04", ClassRank::B1    },
        {"05", ClassRank::A5    },
        {"06", ClassRank::A4    },
        {"07", ClassRank::A3    },
        {"08", ClassRank::A2    },
        {"09", ClassRank::A1    },
        {"10", ClassRank::S5    },
        {"11", ClassRank::S4    },
        {"12", ClassRank::S3    },
        {"13", ClassRank::S2    },
        {"14", ClassRank::S1    },
        {"15", ClassRank::SS5   },
        {"16", ClassRank::SS4   },
        {"17", ClassRank::SS3   },
        {"18", ClassRank::SS2   },
        {"19", ClassRank::SS1   },
        {"20", ClassRank::SSS5  },
        {"21", ClassRank::SSS4  },
        {"22", ClassRank::SSS3  },
        {"23", ClassRank::SSS2  },
        {"24", ClassRank::SSS1  },
        {"25", ClassRank::LEGEND},
    };
    auto str = util::string::right(url, "/class_rank_s_");
    if (str.size() < 2) {
        UNEXPECTED_WEBPAGE;
    }
    if (auto it = mapping.find(str.substr(0, 2)); it != mapping.end()) {
        return it->second;
    }
    UNEXPECTED_WEBPAGE;
}

} // namespace mai::service
