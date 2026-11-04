// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include "error.h"
#include "service/maimai-dx/types.h"

namespace mai::service {

std::expected<PlayerInformation, MaiError> get_player_information();

}
