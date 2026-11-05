// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2026-present, RedbeanW.
 * This file is part of the Maibox open source project.
 */

#include <tidy.h>
#include <tidybuffio.h>

#include "log.h"
#include "util/html_parser.h"

namespace mai::util {

std::expected<pugi::xml_document, MaiError> parse_html(std::string&& html) {
    TidyDoc    tidy_doc = tidyCreate();
    TidyBuffer out_buf  = {};
    TidyBuffer err_buf  = {};
    tidyOptSetBool(tidy_doc, TidyXmlOut, yes);
    tidyOptSetBool(tidy_doc, TidyQuiet, yes);
    tidyOptSetBool(tidy_doc, TidyNumEntities, yes);
    tidyOptSetBool(tidy_doc, TidyShowWarnings, no);
    tidyOptSetInt(tidy_doc, TidyWrapLen, 0);
    tidySetErrorBuffer(tidy_doc, &err_buf);
    tidyParseString(tidy_doc, html.c_str());
    tidyCleanAndRepair(tidy_doc);
    tidySaveBuffer(tidy_doc, &out_buf);
    if (!out_buf.bp) {
        DBG("Tidy error:\n{}",
            std::string_view((char*)err_buf.bp, err_buf.size));
        if (out_buf.allocated) tidyBufFree(&out_buf);
        if (err_buf.allocated) tidyBufFree(&err_buf);
        tidyRelease(tidy_doc);
        return std::unexpected(MaiError::FAILED_TO_TIDY_HTML_DOCUMENT);
    }
    DBG("Tidy output:\n{}", std::string_view((char*)out_buf.bp, out_buf.size));
    pugi::xml_document xml_doc;
    auto               result = xml_doc.load_buffer(out_buf.bp, out_buf.size);
    if (out_buf.allocated) tidyBufFree(&out_buf);
    if (err_buf.allocated) tidyBufFree(&err_buf);
    tidyRelease(tidy_doc);
    if (!result) {
        return std::unexpected(MaiError::FAILED_TO_PARSE_XML_DOCUMENT);
    }
    return xml_doc;
}

} // namespace mai::util
