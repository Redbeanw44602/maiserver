// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <pugixml.hpp>

#include "error.h"

namespace mai::util {

std::expected<pugi::xml_document, MaiError> parse_html(std::string&& html);

} // namespace mai::util
