// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#pragma once

#include <string_view>

namespace mai::service {

/*

# Partial WAF Rules for maimai DX NET (CN)

## Detection of Illegal Clients

If you make a request using a non-browser User-Agent (such as curl/8.23.0), you
will be immediately banned for 30 minutes.

## Rate Limiting

The rate-limiting rules for maimai.wahlap.com are primarily divided into two
windows: a long window and a short window. As long as the limit for either
window is triggered, the request will be rejected. Rate limiting is calculated
simply by the number of requests; for example, a 302 redirect to a 200 is also
counted as two requests.

### Long Window
- Valid for 60 seconds starting from the first request
- Frequency limit: 120 requests
- If the limit is exceeded, remaining requests within that window are rejected
- Shared across all pathnames

### Short Window
- Valid for 5 seconds starting from the first request
- Frequency limit: 10 requests
- If the limit is exceeded, remaining requests are rejected for the next 30
minutes
- Independent for each pathname

### Notes on Pathnames
- Pathnames are independent of one another even if they have a hierarchical
relationship. For example, '/music', '/music/detail', '/music.jpg', and
'/music_01.jpg' are four independent paths, each with its own short window.
- Pathnames do not include request parameters. For example, '/music/share?id=3'
and '/music/share?id=4' share the same short window.

## Blocked State

If you are blocked or your request is rejcted, you will receive a 567 status
code from the Tencent Cloud EdgeOne server. The block is enforced based on IP
address.

*/

namespace rate_limiter {

void acquire(std::string_view url);

}

} // namespace mai::service
