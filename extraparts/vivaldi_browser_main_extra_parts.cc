// Copyright (c) 2015 Vivaldi Technologies AS. All rights reserved

#include "extraparts/vivaldi_browser_main_extra_parts.h"

#include <string>

#include "app/vivaldi_apptools.h"
#include "base/command_line.h"
#include "calendar/calendar_model_loaded_observer.h"
#include "calendar/calendar_service_factory.h"
#include "chrome/browser/net/url_info.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/chrome_switches.h"
#include "components/prefs/pref_service.h"
#include "components/security_state/core/switches.h"
#include "components/translate/core/browser/translate_pref_names.h"
#include "content/public/common/content_switches.h"
#include "notes/notes_factory.h"
#include "notes/notes_model.h"
#include "notes/notes_model_loaded_observer.h"
#include "notes/notesnode.h"
#include "prefs/vivaldi_pref_names.h"

#include "components/datasource/vivaldi_data_source_api.h"
#include "extensions/api/bookmarks/bookmarks_private_api.h"
#include "extensions/api/calendar/calendar_api.h"
#include "extensions/api/extension_action_utils/extension_action_utils_api.h"
#include "extensions/api/history/history_private_api.h"
#include "extensions/api/import_data/import_data_api.h"
#include "extensions/api/notes/notes_api.h"
#include "extensions/api/runtime/runtime_api.h"
#include "extensions/api/settings/settings_api.h"
#include "extensions/api/show_menu/show_menu_api.h"
#include "extensions/api/sync/sync_api.h"
#include "extensions/api/tabs/tabs_private_api.h"
#include "extensions/api/vivaldi_utilities/vivaldi_utilities_api.h"
#include "extensions/api/zoom/zoom_api.h"
#include "extensions/vivaldi_extensions_init.h"
#include "ui/devtools/devtools_connector.h"

VivaldiBrowserMainExtraParts::VivaldiBrowserMainExtraParts() {}

VivaldiBrowserMainExtraParts::~VivaldiBrowserMainExtraParts() {}

// Overridden from ChromeBrowserMainExtraParts:
void VivaldiBrowserMainExtraParts::PostEarlyInitialization() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (vivaldi::IsVivaldiRunning()) {
    // andre@vivaldi.com HACK while not having all the permission dialogs in
    // place.
    vivaldi::CommandLineAppendSwitchNoDup(command_line,
                                          switches::kAlwaysAuthorizePlugins);

    //vivaldi::CommandLineAppendSwitchNoDup(
    //    command_line, translate::switches::kDisableTranslate);

    // NOTE(arnar): Can be removed once ResizeObserver is stable.
    // https://www.chromestatus.com/feature/5705346022637568
    if (command_line->HasSwitch(switches::kEnableBlinkFeatures)) {
      std::string enabledBlinkFeatures =
          command_line->GetSwitchValueASCII(switches::kEnableBlinkFeatures);

      if (enabledBlinkFeatures.find("ResizeObserver") == std::string::npos) {
        std::string out = enabledBlinkFeatures.append(",ResizeObserver");
        command_line->AppendSwitchASCII(switches::kEnableBlinkFeatures, out);
      }
    } else {
      command_line->AppendSwitchASCII(switches::kEnableBlinkFeatures,
                                      "ResizeObserver");
    }
  }

#if defined(OS_MACOSX)
  PostEarlyInitializationMac();
#endif
#if defined(OS_WIN)
  PostEarlyInitializationWin();
#endif
#if defined(OS_LINUX)
  PostEarlyInitializationLinux();
#endif
}

void VivaldiBrowserMainExtraParts::
    EnsureBrowserContextKeyedServiceFactoriesBuilt() {
  extensions::CalendarAPI::GetFactoryInstance();
  extensions::VivaldiBookmarksAPI::GetFactoryInstance();
  extensions::DevtoolsConnectorAPI::GetFactoryInstance();
  extensions::ExtensionActionUtilFactory::GetInstance();
  extensions::ImportDataAPI::GetFactoryInstance();
  extensions::NotesAPI::GetFactoryInstance();
  extensions::TabsPrivateAPI::GetFactoryInstance();
  extensions::ShowMenuAPI::GetFactoryInstance();
  extensions::SyncAPI::GetFactoryInstance();
  extensions::VivaldiDataSourcesAPI::GetFactoryInstance();
  extensions::VivaldiExtensionInit::GetFactoryInstance();
  extensions::VivaldiRuntimeFeaturesFactory::GetInstance();
  extensions::VivaldiSettingsApiNotificationFactory::GetInstance();
  extensions::VivaldiUtilitiesAPI::GetFactoryInstance();
  extensions::ZoomAPI::GetFactoryInstance();
  extensions::HistoryPrivateAPI::GetFactoryInstance();
}

void VivaldiBrowserMainExtraParts::PreProfileInit() {
  EnsureBrowserContextKeyedServiceFactoriesBuilt();

#if defined(OS_MACOSX)
  PreProfileInitMac();
#endif
}

void VivaldiBrowserMainExtraParts::PostProfileInit() {
  Profile* profile = ProfileManager::GetActiveUserProfile();

  vivaldi::Notes_Model* notes_model =
      vivaldi::NotesModelFactory::GetForProfile(profile);
  notes_model->AddObserver(new vivaldi::NotesModelLoadedObserver(profile));

  calendar::CalendarService* calendar_service =
      calendar::CalendarServiceFactory::GetForProfile(profile);
  calendar_service->AddObserver(new calendar::CalendarModelLoadedObserver());

  if (!vivaldi::IsVivaldiRunning())
    return;

  PrefService* pref_service = profile->GetPrefs();
  pref_service->SetBoolean(prefs::kEnableTranslate, false);

  if (pref_service->GetBoolean(vivaldiprefs::kSmoothScrollingEnabled) ==
      false) {
    vivaldi::CommandLineAppendSwitchNoDup(
        base::CommandLine::ForCurrentProcess(),
        switches::kDisableSmoothScrolling);
  }
}
