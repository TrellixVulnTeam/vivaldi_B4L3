// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/history/core/browser/history_database_params.h"

namespace history {

HistoryDatabaseParams::HistoryDatabaseParams()
    : history_dir(),
      download_interrupt_reason_none(0),
      download_interrupt_reason_crash(0),
      number_of_days_to_keep_visits(90) {
}

HistoryDatabaseParams::HistoryDatabaseParams(
    const base::FilePath& history_dir,
    DownloadInterruptReason download_interrupt_reason_none,
    DownloadInterruptReason download_interrupt_reason_crash)
    : history_dir(history_dir),
      download_interrupt_reason_none(download_interrupt_reason_none),
      download_interrupt_reason_crash(download_interrupt_reason_crash) {
}

HistoryDatabaseParams::~HistoryDatabaseParams() {
}

}  // namespace history
