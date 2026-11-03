// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mai::service {

enum class ScoreDifficulty { BASIC, ADVANCED, EXPERT, MASTER, RE_MASTER };

enum class ScoreKind { STANDARD, DX };

struct ScoreLevel {
    uint32_t level_num;
    bool     is_plus;
    bool     is_unknown;
};

struct RatingDetail {
    struct Music {
        std::string     id;
        std::string     name;
        ScoreKind       kind;
        ScoreDifficulty difficulty;
        ScoreLevel      level;
        double          score;
    };
    std::vector<Music> best_15;
    std::vector<Music> best_35;
    std::vector<Music> candidate_best_15;
    std::vector<Music> candidate_best_35;
};

} // namespace mai::service
