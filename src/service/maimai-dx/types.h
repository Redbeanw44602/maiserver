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

// clang-format off
enum class ClassRank { 
    B5,   B4,   B3,   B2,   B1,
    A5,   A4,   A3,   A2,   A1,
    S5,   S4,   S3,   S2,   S1,
    SS5,  SS4,  SS3,  SS2,  SS1,
    SSS5, SSS4, SSS3, SSS2, SSS1,
    LEGEND
};
// clang-format on

struct ScoreLevel {
    uint32_t level_num;
    bool     is_plus;
    bool     is_unknown;
};

struct PlayerInformation {
    std::string avatar_url;
    std::string name;
    std::string trophy;
    uint32_t    dx_rating;
    uint32_t    star_count;
    std::string course_rank;
    ClassRank   class_rank;
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
