// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include "service/maimai-dx/home/home.h"
#include "service/maimai-dx/helper.h"

using namespace mai::util;

namespace mai::service {

namespace {

std::expected<PlayerInformation, MaiError> parse(std::string&& html) {
    auto xml_doc = parse_html(std::move(html));
    if (!xml_doc) {
        UNEXPECTED_WEBPAGE_WITH_ERR(xml_doc);
    }

    auto basic_node =
        xml_doc->select_node("//div[@class=\"basic_block p_10 f_0\"]").node();

    // clang-format off
    auto avatar_url = std::string(basic_node.select_node(".//img[@class=\"w_112 f_l\"]").node().attribute("src").as_string());
    auto name = std::string(basic_node.select_node(".//div[@class=\"name_block f_l f_16\"]").node().text().as_string());
    auto trophy = std::string(basic_node.select_node(".//div[@class=\"trophy_inner_block f_13\"]").node().child("span").text().as_string());
    auto dx_rating_str = std::string_view(basic_node.select_node(".//div[@class=\"rating_block\"]").node().text().as_string());
    auto star_count_str = std::string_view(basic_node.select_node(".//div[@class=\"p_l_10 f_l f_14\"]").node().text().as_string());
    auto course_rank_url = std::string_view(basic_node.select_node(".//img[@class=\"h_35 f_l\"]").node().attribute("src").as_string());
    auto class_rank_url = std::string_view(basic_node.select_node(".//img[@class=\"p_l_10 h_35 f_l\"]").node().attribute("src").as_string());
    // clang-format on

    DBG("name='{}',trophy='{}',dx_rating_str='{}',star_count_str='{}'",
        name,
        trophy,
        dx_rating_str,
        star_count_str);

    auto dx_rating   = string::to_numeric<uint32_t>(dx_rating_str);
    auto star_count  = star_count_from_str(star_count_str);
    auto course_rank = course_rank_from_url(course_rank_url);
    auto class_rank  = class_rank_from_url(class_rank_url);

    if (avatar_url.empty()) {
        UNEXPECTED_WEBPAGE;
    }
    if (auto error = check_multiple_expected(
            dx_rating,
            star_count,
            course_rank,
            class_rank
        )) {
        return std::unexpected(*error);
    }

    return PlayerInformation{
        std::move(avatar_url),
        std::move(name),
        std::move(trophy),
        *dx_rating,
        *star_count,
        *course_rank,
        *class_rank
    };
}

} // namespace

std::expected<PlayerInformation, MaiError> get_player_information() {
    auto webpage = session::simple_get(url::HOME);
    if (!webpage) {
        UNEXPECTED_WEBPAGE_WITH_ERR(webpage);
    }
    return parse(std::move(*webpage));
}

} // namespace mai::service
