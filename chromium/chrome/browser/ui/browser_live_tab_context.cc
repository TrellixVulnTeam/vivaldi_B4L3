// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_live_tab_context.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_tabrestore.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/sessions/content/content_live_tab.h"
#include "components/sessions/content/content_platform_specific_tab_data.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/session_storage_namespace.h"

#if BUILDFLAG(ENABLE_SESSION_SERVICE)
#include "chrome/browser/sessions/tab_loader.h"
#endif

#include "app/vivaldi_apptools.h"

using content::NavigationController;
using content::SessionStorageNamespace;
using content::WebContents;

void BrowserLiveTabContext::ShowBrowserWindow() {
  browser_->window()->Show();
}

const SessionID& BrowserLiveTabContext::GetSessionID() const {
  return browser_->session_id();
}

int BrowserLiveTabContext::GetTabCount() const {
  return browser_->tab_strip_model()->count();
}

int BrowserLiveTabContext::GetSelectedIndex() const {
  return browser_->tab_strip_model()->active_index();
}

std::string BrowserLiveTabContext::GetAppName() const {
  return browser_->app_name();
}

std::string BrowserLiveTabContext::GetExtData() const {
  return browser_->ext_data();
}

sessions::LiveTab* BrowserLiveTabContext::GetLiveTabAt(int index) const {
  return sessions::ContentLiveTab::GetForWebContents(
      browser_->tab_strip_model()->GetWebContentsAt(index));
}

sessions::LiveTab* BrowserLiveTabContext::GetActiveLiveTab() const {
  return sessions::ContentLiveTab::GetForWebContents(
      browser_->tab_strip_model()->GetActiveWebContents());
}

bool BrowserLiveTabContext::IsTabPinned(int index) const {
  return browser_->tab_strip_model()->IsTabPinned(index);
}

sessions::LiveTab* BrowserLiveTabContext::AddRestoredTab(
    const std::vector<sessions::SerializedNavigationEntry>& navigations,
    int tab_index,
    int selected_navigation,
    const std::string& extension_app_id,
    bool select,
    bool pin,
    bool from_last_session,
    const sessions::PlatformSpecificTabData* tab_platform_data,
    const std::string& user_agent_override,
    const std::string& ext_data) {
  SessionStorageNamespace* storage_namespace =
      tab_platform_data
          ? static_cast<const sessions::ContentPlatformSpecificTabData*>(
                tab_platform_data)
                ->session_storage_namespace()
          : nullptr;

  WebContents* web_contents = chrome::AddRestoredTab(
      browser_, navigations, tab_index, selected_navigation, extension_app_id,
      select, pin, from_last_session, storage_namespace, user_agent_override,
      ext_data);

#if BUILDFLAG(ENABLE_SESSION_SERVICE)
  // The focused tab will be loaded by Browser, and TabLoader will load the
  // rest.
  if (!select) {
    // Regression check: make sure that the tab hasn't started to load
    // immediately.
    DCHECK(web_contents->GetController().NeedsReload());
    DCHECK(!web_contents->IsLoading());
  }
  std::vector<TabLoader::RestoredTab> restored_tabs;
  restored_tabs.emplace_back(web_contents, select, !extension_app_id.empty(),
                             pin);
  TabLoader::RestoreTabs(restored_tabs, base::TimeTicks::Now());
#else   // BUILDFLAG(ENABLE_SESSION_SERVICE)
  // Load the tab manually if there is no TabLoader.
  web_contents->GetController().LoadIfNecessary();
#endif  // BUILDFLAG(ENABLE_SESSION_SERVICE)

  return sessions::ContentLiveTab::GetForWebContents(web_contents);
}

sessions::LiveTab* BrowserLiveTabContext::ReplaceRestoredTab(
    const std::vector<sessions::SerializedNavigationEntry>& navigations,
    int selected_navigation,
    bool from_last_session,
    const std::string& extension_app_id,
    const sessions::PlatformSpecificTabData* tab_platform_data,
    const std::string& user_agent_override,
    const std::string& ext_data) {
  SessionStorageNamespace* storage_namespace =
      tab_platform_data
          ? static_cast<const sessions::ContentPlatformSpecificTabData*>(
                tab_platform_data)
                ->session_storage_namespace()
          : nullptr;

  WebContents* web_contents = chrome::ReplaceRestoredTab(
      browser_, navigations, selected_navigation, from_last_session,
      extension_app_id, storage_namespace, user_agent_override, ext_data);

  return sessions::ContentLiveTab::GetForWebContents(web_contents);
}

void BrowserLiveTabContext::CloseTab() {
  chrome::CloseTab(browser_);
}

// static
sessions::LiveTabContext* BrowserLiveTabContext::Create(
    Profile* profile,
    const std::string& app_name, const std::string& ext_data) {
  Browser* browser;
  if (app_name.empty()) {
    Browser::CreateParams params(profile, true);
    params.is_vivaldi = vivaldi::IsVivaldiRunning();
    params.ext_data = ext_data;
    browser = new Browser(params);
  } else {
    // Only trusted app popup windows should ever be restored.
    browser = new Browser(Browser::CreateParams::CreateForApp(
        app_name, true /* trusted_source */, gfx::Rect(), profile, true));
  }
  if (browser)
    return browser->live_tab_context();
  else
    return NULL;
}

// static
sessions::LiveTabContext* BrowserLiveTabContext::FindContextForWebContents(
    const WebContents* contents) {
  Browser* browser = chrome::FindBrowserWithWebContents(contents);
  return browser ? browser->live_tab_context() : nullptr;
}

// static
sessions::LiveTabContext* BrowserLiveTabContext::FindContextWithID(
    SessionID::id_type desired_id) {
  Browser* browser = chrome::FindBrowserWithID(desired_id);
  return browser ? browser->live_tab_context() : nullptr;
}
