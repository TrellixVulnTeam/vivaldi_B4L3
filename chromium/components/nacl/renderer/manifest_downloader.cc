// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/nacl/renderer/manifest_downloader.h"

#include <utility>

#include "base/callback.h"
#include "components/nacl/renderer/histogram.h"
#include "components/nacl/renderer/nexe_load_manager.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLLoader.h"
#include "third_party/WebKit/public/platform/WebURLResponse.h"
#include "third_party/WebKit/public/web/WebAssociatedURLLoader.h"

namespace nacl {

ManifestDownloader::ManifestDownloader(
    std::unique_ptr<blink::WebAssociatedURLLoader> url_loader,
    bool is_installed,
    Callback cb)
    : url_loader_(std::move(url_loader)),
      is_installed_(is_installed),
      cb_(cb),
      status_code_(-1),
      pp_nacl_error_(PP_NACL_ERROR_LOAD_SUCCESS) {
  CHECK(!cb.is_null());
}

ManifestDownloader::~ManifestDownloader() { }

void ManifestDownloader::Load(const blink::WebURLRequest& request) {
  url_loader_->LoadAsynchronously(request, this);
}

void ManifestDownloader::DidReceiveResponse(
    const blink::WebURLResponse& response) {
  if (response.HttpStatusCode() != 200)
    pp_nacl_error_ = PP_NACL_ERROR_MANIFEST_LOAD_URL;
  status_code_ = response.HttpStatusCode();
}

void ManifestDownloader::DidReceiveData(const char* data, int data_length) {
  if (buffer_.size() + data_length > kNaClManifestMaxFileBytes) {
    pp_nacl_error_ = PP_NACL_ERROR_MANIFEST_TOO_LARGE;
    buffer_.clear();
  }

  if (pp_nacl_error_ == PP_NACL_ERROR_LOAD_SUCCESS)
    buffer_.append(data, data_length);
}

void ManifestDownloader::Close() {
  // We log the status code here instead of in didReceiveResponse so that we
  // always log a histogram value, even when we never receive a status code.
  HistogramHTTPStatusCode(
      is_installed_ ? "NaCl.HttpStatusCodeClass.Manifest.InstalledApp" :
                      "NaCl.HttpStatusCodeClass.Manifest.NotInstalledApp",
      status_code_);

  cb_.Run(pp_nacl_error_, buffer_);
  delete this;
}

void ManifestDownloader::DidFinishLoading(double finish_time) {
  Close();
}

void ManifestDownloader::DidFail(const blink::WebURLError& error) {
  // TODO(teravest): Find a place to share this code with PepperURLLoaderHost.
  pp_nacl_error_ = PP_NACL_ERROR_MANIFEST_LOAD_URL;
  if (error.domain.Equals(blink::WebString::FromUTF8(net::kErrorDomain))) {
    switch (error.reason) {
      case net::ERR_ACCESS_DENIED:
      case net::ERR_NETWORK_ACCESS_DENIED:
        pp_nacl_error_ = PP_NACL_ERROR_MANIFEST_NOACCESS_URL;
        break;
    }
  }

  if (error.is_web_security_violation)
    pp_nacl_error_ = PP_NACL_ERROR_MANIFEST_NOACCESS_URL;

  Close();
}

}  // namespace nacl
