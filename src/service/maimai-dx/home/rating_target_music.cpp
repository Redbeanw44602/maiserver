// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "service/maimai-dx/home/rating_target_music.h"
#include "service/maimai-dx/helper.h"

using namespace mai::util;

namespace mai::service {

namespace {

std::expected<RatingDetail, MaiError> parse(std::string&& html) {
    auto xml_doc = parse_html(std::move(html));
    if (!xml_doc) {
        UNEXPECTED_WEBPAGE_WITH_ERR(xml_doc);
    }

    auto nodes = xml_doc->select_nodes(
        "//div[@class=\"wrapper main_wrapper t_c o_v\"]/div"
    );

    RatingDetail                      result;
    uint                              music_vec_num = 0;
    std::vector<RatingDetail::Music>* music_vec{};

    for (const auto& node_ : nodes) {
        auto node       = node_.node();
        auto class_name = std::string_view(node.attribute("class").as_string());
        if (class_name == "screw_block m_15 f_15 p_s") {
            music_vec_num += 1;
            if (music_vec_num == 1) { // 评分对象曲目（新曲）
                music_vec = &result.best_15;
            } else if (music_vec_num == 2) { // 评分对象曲目（最佳）
                music_vec = &result.best_35;
            } else if (music_vec_num == 3) { // 评分候选曲目（新曲）
                music_vec = &result.candidate_best_15;
            } else if (music_vec_num == 4) { // 评分候选曲目（最佳）
                music_vec = &result.candidate_best_35;
            } else {
                break;
            }
            continue;
        }
        if (music_vec && class_name.starts_with("music_")) {
            auto form = node.child("form");
            // clang-format off
            auto difficulty_img = std::string_view(form.find_child_by_attribute("class", "h_20 f_l").attribute("src").as_string());
            auto kind_img = std::string_view(form.find_child_by_attribute("class", "music_kind_icon f_r").attribute("src").as_string());
            auto level_str = std::string_view(form.find_child_by_attribute("class", "music_lv_block f_r t_c f_14").text().as_string());
            auto name = form.find_child_by_attribute("class", "music_name_block t_l f_13 break").text().as_string();
            auto score_str = string::strip(std::string_view(form.find_child_by_attribute("class", "music_score_block w_150 t_l f_r f_12 p_r").text().as_string()));
            auto id = std::string(form.child("input").attribute("value").as_string());
            // clang-format on
            auto difficulty = score_difficulty_from_url(difficulty_img);
            auto kind       = score_kind_from_url(kind_img);
            auto level      = score_level_from_str(level_str);
            auto score      = score_from_str(score_str);
            if (id.empty()) {
                UNEXPECTED_WEBPAGE;
            }
            if (auto error =
                    check_multiple_expected(difficulty, kind, level, score)) {
                return std::unexpected(*error);
            }
            music_vec->emplace_back(
                RatingDetail::Music{
                    std::move(id),
                    name,
                    *kind,
                    *difficulty,
                    *level,
                    *score
                }
            );
        }
    }

    if (result.best_15.size() != 15 || result.best_35.size() != 35
        || result.candidate_best_15.empty()
        || result.candidate_best_35.empty()) {
        UNEXPECTED_WEBPAGE;
    }

    return result;
}

} // namespace

std::expected<RatingDetail, MaiError> get_rating_detail() {
    auto webpage = session::simple_get(url::HOME_RATING_TARGET_MUSIC);
    if (!webpage) {
        UNEXPECTED_WEBPAGE_WITH_ERR(webpage);
    }
    return parse(std::move(*webpage));
}

} // namespace mai::service
