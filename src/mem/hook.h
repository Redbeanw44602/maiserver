// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <print>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#include <dobby.h>
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#include "frida-gum.h"
#pragma GCC diagnostic pop

#include "util/string.h"

inline __attribute__((constructor(1000))) void gum_ctor() {
    gum_init_embedded();
}

inline __attribute__((destructor(1000))) void gum_dtor() {
    gum_deinit_embedded();
}

namespace mai::mem {

namespace detail {

template <typename FunctionResolver, typename Sig>
struct Function;

template <typename FunctionResolver, typename Ret, typename... Args>
struct Function<FunctionResolver, Ret(Args...)> {
    using return_type        = Ret;
    using function_signature = Ret(Args...);

    static uintptr_t address() { return FunctionResolver::address(); }

    template <typename... CallArgs>
    static Ret operator()(CallArgs&&... args) {
        return reinterpret_cast<function_signature*>(address())(
            std::forward<CallArgs>(args)...
        );
    }
};

template <util::string::Fixed Module>
struct ModuleBase {
    static uintptr_t get() {
        static auto cached = find_base();
        return cached;
    }

private:
    static uintptr_t find_base() {
        auto* fp = fopen("/proc/self/maps", "r");
        if (!fp) return 0;

        char line[1024];

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, Module.c_str())) {
                uintptr_t addr;
                sscanf(line, "%lx-", &addr);
                fclose(fp);
                return addr;
            }
        }

        std::println("Failed to get base address of {}!", Module.c_str());

        fclose(fp);
        return 0;
    }
};

template <util::string::Fixed Module, uintptr_t Offset>
struct ModuleOffsetResolver {
    static uintptr_t address() {
        static auto cached = ModuleBase<Module>::get() + Offset;
        return cached;
    }
};

template <util::string::Fixed Symbol>
struct SymbolResolver {
    static uintptr_t address() {
        static auto cached = reinterpret_cast<uintptr_t>(
            DobbySymbolResolver(nullptr, Symbol.c_str())
        );
        return cached;
    }
};

} // namespace detail

template <util::string::Fixed Module, uintptr_t Offset, typename Signature>
using DefineFunction =
    detail::Function<detail::ModuleOffsetResolver<Module, Offset>, Signature>;

template <util::string::Fixed Symbol, typename Signature>
using ResolveFunction =
    detail::Function<detail::SymbolResolver<Symbol>, Signature>;

} // namespace mai::mem

#define HOOK_NS(x)             namespace x
#define HOOK_NOREF_TYPE(x)     std::remove_reference_t<decltype(x)>
#define HOOK_NESTED_TYPE(x, t) typename HOOK_NOREF_TYPE(x)::t

#define HOOK_AUTOGEN    mai::mem::autogen
#define HOOK_AUTOGEN_NS HOOK_NS(HOOK_AUTOGEN)

#define HOOK_REGISTRAR(function) HOOK_AUTOGEN::_##function::Registrar

#define HOOK_DEFINE(function, ...)                                             \
    HOOK_AUTOGEN_NS {                                                          \
        HOOK_NS(_##function) {                                                 \
            using function_signature =                                         \
                HOOK_NESTED_TYPE(function, function_signature);                \
            struct Registrar {                                                 \
                Registrar() {                                                  \
                    auto interceptor = gum_interceptor_obtain();               \
                    gum_interceptor_begin_transaction(interceptor);            \
                    auto address = HOOK_NOREF_TYPE(function)::address();       \
                    if (gum_interceptor_replace(                               \
                            interceptor,                                       \
                            GSIZE_TO_POINTER(address),                         \
                            GSIZE_TO_POINTER(detour),                          \
                            (gpointer*)&origin,                                \
                            NULL                                               \
                        )                                                      \
                        != GUM_REPLACE_OK) {                                   \
                        std::println(                                          \
                            "Failed to hook: {} ({:#x}).",                     \
                            #function,                                         \
                            address                                            \
                        );                                                     \
                    }                                                          \
                    gum_interceptor_end_transaction(interceptor);              \
                }                                                              \
                static function_signature* origin;                             \
                static function_signature  detour;                             \
            };                                                                 \
            function_signature* Registrar::origin = nullptr;                   \
        }                                                                      \
    }

#define HOOK_DETOUR(function, ...)                                             \
    HOOK_NESTED_TYPE(function, return_type)                                    \
    HOOK_REGISTRAR(function)::detour(__VA_ARGS__)

#define HOOK_AUTO_INSTALL(function)                                            \
    HOOK_AUTOGEN_NS {                                                          \
        HOOK_NS(_##function) {                                                 \
            HOOK_REGISTRAR(function)                                           \
            installed __attribute__((init_priority(2000)));                    \
        }                                                                      \
    }

#define HOOK_INSTALL(function) HOOK_REGISTRAR(function) installed_##function;

#define HOOK(function, ...)                                                    \
    HOOK_DEFINE(function, __VA_ARGS__)                                         \
    HOOK_AUTO_INSTALL(function)                                                \
    HOOK_DETOUR(function, __VA_ARGS__)

#define HOOK_DELAYED(function, ...)                                            \
    HOOK_DEFINE(function, __VA_ARGS__)                                         \
    HOOK_DETOUR(function, __VA_ARGS__)
