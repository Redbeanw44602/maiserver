// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#define PRIORITY_LOW    3000
#define PRIORITY_MIDDLE 2000
#define PRIORITY_HIGH   1000

#define CTOR_PRIORITY_LOW    __attribute__((constructor(PRIORITY_LOW)))
#define CTOR_PRIORITY_MIDDLE __attribute__((constructor(PRIORITY_MIDDLE)))
#define CTOR_PRIORITY_HIGH   __attribute__((constructor(PRIORITY_HIGH)))

#define DTOR_PRIORITY_LOW    __attribute__((destructor(PRIORITY_LOW)))
#define DTOR_PRIORITY_MIDDLE __attribute__((destructor(PRIORITY_MIDDLE)))
#define DTOR_PRIORITY_HIGH   __attribute__((destructor(PRIORITY_HIGH)))

#define INIT_PRIORITY_LOW    __attribute__((init_priority(PRIORITY_LOW)))
#define INIT_PRIORITY_MIDDLE __attribute__((init_priority(PRIORITY_MIDDLE)))
#define INIT_PRIORITY_HIGH   __attribute__((init_priority(PRIORITY_HIGH)))
