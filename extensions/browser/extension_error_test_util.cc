// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/extension_error_test_util.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/url_constants.h"
#include "extensions/browser/extension_error.h"
#include "extensions/common/constants.h"
#include "extensions/common/stack_frame.h"
#include "url/gurl.h"

namespace extensions {
namespace error_test_util {

namespace {
const char kDefaultStackTrace[] = "function_name (https://url.com:1:1)";
}

scoped_ptr<ExtensionError> CreateNewRuntimeError(
    const std::string& extension_id,
    const std::string& message,
    bool from_incognito) {
  StackTrace stack_trace;
  scoped_ptr<StackFrame> frame =
      StackFrame::CreateFromText(base::UTF8ToUTF16(kDefaultStackTrace));
  CHECK(frame.get());
  stack_trace.push_back(*frame);

  base::string16 source =
      base::UTF8ToUTF16(std::string(kExtensionScheme) +
                            content::kStandardSchemeSeparator +
                            extension_id);

  return scoped_ptr<ExtensionError>(new RuntimeError(
      extension_id,
      from_incognito,
      source,
      base::UTF8ToUTF16(message),
      stack_trace,
      GURL::EmptyGURL(),  // no context url
      logging::LOG_INFO,
      0, 0 /* Render [View|Process] ID */ ));
}

scoped_ptr<ExtensionError> CreateNewRuntimeError(
    const std::string& extension_id, const std::string& message) {
  return CreateNewRuntimeError(extension_id, message, false);
}

scoped_ptr<ExtensionError> CreateNewManifestError(
    const std::string& extension_id, const std::string& message) {
  return scoped_ptr<ExtensionError>(
      new ManifestError(extension_id,
                        base::UTF8ToUTF16(message),
                        base::EmptyString16(),
                        base::EmptyString16()));
}

}  // namespace error_test_util
}  // namespace extensions
