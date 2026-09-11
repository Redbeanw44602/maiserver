#pragma once

#include <expected>

#include "config.h"
#include "error.h"
#include "util/simple_channel.h"

namespace mai::service {

namespace detail {

class HasError {
public:
    HasError(MaiError error) : error_(error) {}

    operator bool() { return error_ != MaiError::NO_ERROR; }
    operator MaiError() { return error_; }

    template <typename T>
    operator std::expected<T, MaiError>() {
        return std::unexpected(error_);
    }

private:
    MaiError error_;
};

} // namespace detail

inline detail::HasError check(const util::SimpleChannel& channel) {
    if (g_uin == 0) {
        return MaiError::WECHAT_NOT_LOGGED_IN;
    }
    if (channel.is_busy()) {
        return MaiError::BUSY;
    }
    return MaiError::NO_ERROR;
}

} // namespace mai::service
