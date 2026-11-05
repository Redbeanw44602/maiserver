// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "service/maimai-dx/record/music_detail.h"
#include "service/maimai-dx/helper.h"

using namespace mai::util;

namespace mai::service {

namespace {

std::expected<RecordDetail, MaiError> parse(std::string&& html) {
    auto xml_doc = parse_html(std::move(html));
    if (!xml_doc) {
        UNEXPECTED_WEBPAGE_WITH_ERR(xml_doc);
    }

    auto nodes = xml_doc->select_nodes(
        "/html/body/div[@class=\"wrapper main_wrapper t_c\"]/div"
    );

    RecordDetail result;
    auto         has_basic_block = false;

    for (const auto& node_ : nodes) {
        auto node       = node_.node();
        auto class_name = std::string_view(node.attribute("class").as_string());
        if (class_name == "basic_block m_15 m_t_0 p_5 t_l") {
            has_basic_block = true;
            // clang-format off
            result.name = node.select_node(".//div[@class=\"m_5 f_15 break\"]").node().text().as_string();
            result.description = node.select_node(".//div[@class=\"m_5 f_12 break\"]").node().text().as_string();
            result.cover_url = node.child("img").attribute("src").as_string();
            result.category = string::strip(node.select_node("//div[@class=\"m_10 m_t_5 t_r f_12 blue\"]").node().text().as_string());
            auto kind_img = node.select_node(".//img[@class=\"f_l h_20\"]").node().attribute("src").as_string();
            // clang-format on
            auto kind = score_kind_from_url(kind_img);
            if (!kind || result.cover_url.empty() || result.category.empty()) {
                UNEXPECTED_WEBPAGE;
            }
            result.kind = *kind;
            continue;
        }
        if (has_basic_block && class_name.starts_with("music_")) {
            // clang-format off
            auto difficulty_img = node.find_child_by_attribute("class", "h_20 f_l").attribute("src").as_string();
            auto level = std::string(node.find_child_by_attribute("class", "music_lv_back m_3 m_b_0 f_l t_c f_14").text().as_string());
            auto singleplayer_bonus_img = node.select_node(".//img[@class=\"h_45 v_t\"]").node().attribute("src").as_string();
            auto multiplayer_bonus_img = node.select_node(".//img[@class=\"h_45 m_r_10 v_t\"]").node().attribute("src").as_string();
            auto last_play_time = std::string(node.select_node(".//table[@class=\"collapse f_11\"]/tr[1]/td[2]").node().text().as_string());
            auto play_count_str = node.select_node(".//table[@class=\"collapse f_11\"]/tr[2]/td[2]").node().text().as_string();
            auto score_str = node.select_node(".//div[@class=\"music_score_block w_120 d_ib t_r f_12\"]").node().text().as_string();
            auto dx_score_str = std::string(string::strip(node.select_node(".//div[@class=\"music_score_block w_310 m_r_0 d_ib t_r f_12\"]").node().text().as_string()));
            auto dx_star_img = node.select_node("//img[@class=\"w_80 p_l_10 f_r\"]").node().attribute("src").as_string();
            // clang-format on
            DBG("last_play_time='{}',play_count_str='{}',score_str='{}',dx_"
                "score_str='{}'",
                last_play_time,
                play_count_str,
                score_str,
                dx_score_str);
            if (last_play_time.empty() || level.empty()) {
                UNEXPECTED_WEBPAGE;
            }
            auto difficulty = score_difficulty_from_url(difficulty_img);
            auto sp_bonus = singleplayer_bonus_from_url(singleplayer_bonus_img);
            auto mp_bonus = multiplayer_bonus_from_url(multiplayer_bonus_img);
            auto play_count = string::to_numeric<uint32_t>(play_count_str);
            auto score      = score_from_str(score_str);
            auto dx_score_t = dx_score_from_str(std::move(dx_score_str));
            auto dx_star    = dx_star_from_url(dx_star_img);
            if (auto error = check_multiple_expected(
                    difficulty,
                    sp_bonus,
                    mp_bonus,
                    play_count,
                    score,
                    dx_score_t,
                    dx_star
                )) {
                UNEXPECTED_WEBPAGE_WITH_ERR_R(*error);
            }
            auto [dx_score, dx_score_full] = *dx_score_t;
            result.levels.emplace_back(
                std::move(level),
                *score,
                dx_score,
                dx_score_full,
                *dx_star,
                *sp_bonus,
                *mp_bonus,
                last_play_time,
                *play_count
            );
            continue;
        }
    }

    if (!has_basic_block) {
        UNEXPECTED_WEBPAGE;
    }

    return result;
}

} // namespace

std::expected<RecordDetail, MaiError> get_record_detail(std::string id) {
    // clang-format off
    auto webpage = session::simple_get(
        url::HOME_RECORD_MUSIC_DETAIL,
        true,
        {{"idx", std::move(id)}}
    );
    // clang-format on
    if (!webpage) {
        UNEXPECTED_WEBPAGE_WITH_ERR(webpage);
    }
    return parse(std::move(*webpage));
}

} // namespace mai::service
