// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string>
#include <vector>

namespace mai::util {

class CArgs {
public:
    explicit CArgs(const char* argv[]) {
        for (int i = 0; argv && argv[i] != nullptr; ++i) {
            args_.emplace_back(argv[i]);
        }
    }

    void for_each(std::invocable<std::string&> auto fn) {
        for (auto& arg : args_) {
            fn(arg);
        }
    }

    void replace(std::string_view old_arg, std::string_view new_arg) {
        for_each([=](std::string& arg) {
            if (arg == old_arg) arg = new_arg;
        });
    }

    void append(std::string arg) { args_.push_back(std::move(arg)); }

    auto convert_back_to_cargs() & {
        c_args_.reserve(args_.size() + 1);
        for (auto& arg : args_) {
            c_args_.push_back(arg.data());
        }
        c_args_.push_back(nullptr);
        return c_args_.data();
    }

private:
    std::vector<std::string> args_;
    std::vector<const char*> c_args_;
};

} // namespace mai::util
