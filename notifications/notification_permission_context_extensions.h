// Copyright (c) 2015 Vivaldi Technologies AS. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NOTIFICATIONS_NOTIFICATION_PERMISSION_CONTEXT_EXTENSIONS_H_
#define NOTIFICATIONS_NOTIFICATION_PERMISSION_CONTEXT_EXTENSIONS_H_

#include "base/callback_forward.h"
#include "base/macros.h"
#include "components/content_settings/core/common/content_settings.h"

namespace content {
class WebContents;
}

class GURL;
class PermissionRequestID;
class Profile;

// Chrome extensions specific portions of NotificationPermissionContext.
class NotificationPermissionContextExtensions {
 public:
  explicit NotificationPermissionContextExtensions(Profile* profile);
  ~NotificationPermissionContextExtensions();

  // Returns true if the permission request was handled. In which case,
  // |permission_set| will be set to true if the permission changed, and the
  // permission has been set to |new_permission|.
  bool DecidePermission(content::WebContents* web_contents,
                        const PermissionRequestID& request_id,
                        int bridge_id,
                        const GURL& requesting_frame,
                        bool user_gesture,
                        const base::Callback<void(ContentSetting)>& callback,
                        bool* permission_set,
                        bool* new_permission);

  // Returns true if the cancellation request was handled.
  bool CancelPermissionRequest(content::WebContents* web_contents,
                               int bridge_id);

 private:
  Profile* profile_;

  DISALLOW_COPY_AND_ASSIGN(NotificationPermissionContextExtensions);
};

#endif  // NOTIFICATIONS_NOTIFICATION_PERMISSION_CONTEXT_EXTENSIONS_H_
