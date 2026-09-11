// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <print>

#include <dobby.h>

#define SYM(sym) (DobbySymbolResolver(nullptr, sym))

#define CALLABLE(ret_t, sym, args_t...)                                        \
    CALLABLE_ADDR(ret_t, (uintptr_t)(SYM(#sym)), args_t)

#define CALLABLE_ADDR(ret_t, addr, args_t...)                                  \
    ((ret_t (*)(args_t))((void*)(addr)))

#define HOOK(ret_t, sym, args_t...)                                            \
    HOOK_ADDR(ret_t, sym, (uintptr_t)(SYM(#sym)), args_t)

#define HOOK_ADDR_BASE(ret_t, name, addr, args_t...)                           \
    class HookRegistrar_##name {                                               \
    public:                                                                    \
        explicit HookRegistrar_##name() {                                      \
            if (DobbyHook(                                                     \
                    (void*)(addr),                                             \
                    (dobby_dummy_func_t)detour,                                \
                    (dobby_dummy_func_t*)&origin                               \
                )                                                              \
                != 0) {                                                        \
                std::println("Failed to hook: {} ({:#x}).", #name, (addr));    \
            }                                                                  \
        }                                                                      \
        static ret_t (*origin)(args_t);                                        \
        static ret_t detour(args_t);                                           \
    };                                                                         \
    ret_t (*HookRegistrar_##name::origin)(args_t) = nullptr;

#define HOOK_ADDR_INSTALL(name) HookRegistrar_##name hookRegistrar_##name;
#define HOOK_ADDR_STATIC_INSTALL(name) static HOOK_ADDR_INSTALL(name)

#define HOOK_ADDR(ret_t, name, addr, args_t...)                                \
    HOOK_ADDR_BASE(ret_t, name, addr, args_t)                                  \
    HOOK_ADDR_STATIC_INSTALL(name)                                             \
    ret_t HookRegistrar_##name::detour(args_t)

#define HOOK_ADDR_LAZY(ret_t, name, addr, args_t...)                           \
    HOOK_ADDR_BASE(ret_t, name, addr, args_t)                                  \
    ret_t HookRegistrar_##name::detour(args_t)

#define MODULE(module_name)                                                    \
    namespace module_name {                                                    \
    inline uintptr_t base() { return get_module_base(#module_name); }          \
    inline uintptr_t rel(ptrdiff_t offset) { return base() + offset; }         \
    }

inline uintptr_t _get_module_base(std::string_view module_name) {
    auto* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, module_name.data())) {
            uintptr_t addr;
            sscanf(line, "%lx-", &addr);
            fclose(fp);
            return addr;
        }
    }

    fclose(fp);
    return 0;
}

inline uintptr_t get_module_base(std::string_view module_name) {
    auto ret = _get_module_base(module_name);
    if (!ret) {
        std::println("Failed to get base address of {}.", module_name);
        return 0;
    }
    return ret;
}
