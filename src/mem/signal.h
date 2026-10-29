// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <sigslot/signal.hpp>

#include "mem/priority.h"

namespace mai::mem {

inline sigslot::signal<> on_lib_wmpf_host_export_ready INIT_PRIORITY_HIGH;

}
