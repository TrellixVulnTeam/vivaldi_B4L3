// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/renderer_main_platform_delegate.h"

#include <dwrite.h>

#include <memory>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/win/scoped_comptr.h"
#include "base/win/win_util.h"
#include "base/win/windows_version.h"
#include "content/child/dwrite_font_proxy/dwrite_font_proxy_init_win.h"
#include "content/child/font_warmup_win.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/injection_test_win.h"
#include "content/public/renderer/render_thread.h"
#include "content/renderer/render_thread_impl.h"
#include "sandbox/win/src/sandbox.h"
#include "third_party/WebKit/public/platform/WebRuntimeFeatures.h"
#include "third_party/WebKit/public/web/win/WebFontRendering.h"
#include "third_party/icu/source/i18n/unicode/timezone.h"
#include "third_party/skia/include/ports/SkTypeface_win.h"
#include "ui/display/win/dpi.h"
#include "ui/gfx/win/direct_write.h"

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
#include "platform_media/common/win/mf_util.h"
#endif

namespace content {
void WarmUpMediaFoundation() {
#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
  media::LoadMFCommonLibraries();
  media::LoadMFAudioDecoderLibraries();
  media::LoadMFVideoDecoderLibraries();
#endif
}

RendererMainPlatformDelegate::RendererMainPlatformDelegate(
    const MainFunctionParams& parameters)
    : parameters_(parameters) {}

RendererMainPlatformDelegate::~RendererMainPlatformDelegate() {
}

void RendererMainPlatformDelegate::PlatformInitialize() {
  const base::CommandLine& command_line = parameters_.command_line;

  // Be mindful of what resources you acquire here. They can be used by
  // malicious code if the renderer gets compromised.
  bool no_sandbox = command_line.HasSwitch(switches::kNoSandbox);

  if (!no_sandbox) {
    // ICU DateFormat class (used in base/time_format.cc) needs to get the
    // Olson timezone ID by accessing the registry keys under
    // HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones.
    // After TimeZone::createDefault is called once here, the timezone ID is
    // cached and there's no more need to access the registry. If the sandbox
    // is disabled, we don't have to make this dummy call.
    std::unique_ptr<icu::TimeZone> zone(icu::TimeZone::createDefault());
    WarmUpMediaFoundation();
  }

  InitializeDWriteFontProxy();

  // TODO(robliao): This should use WebScreenInfo. See http://crbug.com/604555.
  blink::WebFontRendering::SetDeviceScaleFactor(display::win::GetDPIScale());
}

void RendererMainPlatformDelegate::PlatformUninitialize() {
  UninitializeDWriteFontProxy();
}

bool RendererMainPlatformDelegate::EnableSandbox() {
  sandbox::TargetServices* target_services =
      parameters_.sandbox_info->target_services;

  if (target_services) {
    // Cause advapi32 to load before the sandbox is turned on.
    unsigned int dummy_rand;
    rand_s(&dummy_rand);

    target_services->LowerToken();
    return true;
  }
  return false;
}

}  // namespace content
