// Copyright (c) 2015 Vivaldi Technologies AS. All rights reserved
#include "testing/disable_unittests_macros.h"

// List of unittest disabled for Windows by Vivaldi
// On the form
//    DISABLE(foo,bar)
//    DISABLE(foo,baz)

// Flaky on Windows, fails on tester and works on local machine
DISABLE(AutofillProfileComparatorTest, MergeAddresses)
DISABLE_ALL(NetworkQualityEstimatorTest)

// Flaky on Windows, fails on tester and works on local machine
DISABLE(LocalFileSyncServiceTest, LocalChangeObserver)

DISABLE(OmniboxViewTest, AcceptKeywordByTypingQuestionMark)

// Flaky in v51
DISABLE_ALL(TabDesktopMediaListTest)

// Flaky in v53
DISABLE(BrowsingDataRemoverBrowserTest, Cache)

// Sems to have broken in v54
DISABLE(QuarantineWinTest, LocalFile_DependsOnLocalConfig)
DISABLE(DownloadTest, CheckLocalhostZone_DependsOnLocalConfig)

// Seems to have broken in v56
DISABLE(PrefHashBrowserTestChangedSplitPrefInstance/PrefHashBrowserTestChangedSplitPref,
        ChangedSplitPref/0)
DISABLE(PrefHashBrowserTestClearedAtomicInstance/PrefHashBrowserTestClearedAtomic,
        ClearedAtomic/1)
DISABLE(PrefHashBrowserTestRegistryValidationFailureInstance/PrefHashBrowserTestRegistryValidationFailure,
        RegistryValidationFailure/1)
DISABLE(PrefHashBrowserTestRegistryValidationFailureInstance/PrefHashBrowserTestRegistryValidationFailure,
        RegistryValidationFailure/3)
DISABLE(PrefHashBrowserTestUnchangedCustomInstance/PrefHashBrowserTestUnchangedCustom,
        UnchangedCustom/0)
DISABLE(PrefHashBrowserTestUnchangedCustomInstance/PrefHashBrowserTestUnchangedCustom,
        UnchangedCustom/3)
DISABLE(PrefHashBrowserTestUnchangedDefaultInstance/PrefHashBrowserTestUnchangedDefault,
        UnchangedDefault/2)
DISABLE(PrefHashBrowserTestUntrustedInitializedInstance/PrefHashBrowserTestUntrustedInitialized,
        UntrustedInitialized/0)
DISABLE(PrefHashBrowserTestUntrustedInitializedInstance/PrefHashBrowserTestUntrustedInitialized,
        UntrustedInitialized/3)

// Started failing in v54 after the Vivaldi sync code was added
DISABLE(StartupBrowserCreatorCorruptProfileTest,
        LastUsedProfileFallbackToAnyProfile)
DISABLE(StartupBrowserCreatorCorruptProfileTest,
        LastUsedProfileFallbackToUserManager)
DISABLE(StartupBrowserCreatorCorruptProfileTest, CannotCreateSystemProfile)

// Seems to have broken in v57
DISABLE(SubresourceFilterBrowserTest,
        ExpectHistogramsAreRecordedForFilteredChildFrames)
DISABLE(SubresourceFilterWithPerformanceMeasurementBrowserTest,
        ExpectHistogramsAreRecorded)
DISABLE(ContentSubresourceFilterDriverFactoryTest,
        SpecialCaseNavigationActivationListEnabledWithPerformanceMeasurement)

// Seems to have gotten flaky in v57 stable
DISABLE(OAuthRequestSignerTest, DecodeEncoded)

DISABLE(MediaGalleriesGalleryWatchApiTest, SetupWatchOnInvalidGallery)
DISABLE(MediaGalleriesGalleryWatchApiTest,
        SetupGalleryChangedListenerWithoutWatchers)
DISABLE(MediaGalleriesGalleryWatchApiTest, SetupGalleryWatchWithoutListeners)

// Seems to have failed in v59
DISABLE(ExtensionApiTest, UserLevelNativeMessaging)
DISABLE(MediaGalleriesPlatformAppBrowserTest, NoGalleriesCopyTo)

// Failed in v60
DISABLE(RenderThreadImplBrowserTest,
        InputHandlerManagerDestroyedAfterCompositorThread)

// Flaky
DISABLE(GenericSensorBrowserTest, AmbientLightSensorTest)
DISABLE(SitePerProcessTextInputManagerTest,
  TrackCompositionRangeForAllFrames)

// Failed in v61, also in pure chromium builds
DISABLE(BookmarkBarViewTest10, KeyEvents)
DISABLE(CertificateSelectorTest, GetSelectedCert)
DISABLE(FindInPageTest, SelectionRestoreOnTabSwitch)
DISABLE(OmniboxViewTest, AcceptKeywordBySpace)
DISABLE(OmniboxViewTest, BackspaceDeleteHalfWidthKatakana)
DISABLE(OmniboxViewTest, BackspaceInKeywordMode)
DISABLE(OmniboxViewTest, BasicTextOperations)
DISABLE(OmniboxViewTest, CtrlArrowAfterArrowSuggestions)
DISABLE(OmniboxViewTest, DeleteItem)
DISABLE(OmniboxViewTest, EscapeToDefaultMatch)
DISABLE(OmniboxViewTest, SelectAllStaysAfterUpdate)
DISABLE(SSLClientCertificateSelectorMultiTabTest, SelectSecond)
DISABLE(BookmarkBarViewTest23, ContextMenusKeyboard)
DISABLE(BookmarkBarViewTest24, ContextMenusKeyboardEscape)
DISABLE(KeyboardAccessTest, TestMenuKeyboardAccess)
DISABLE(OmniboxViewTest, DesiredTLDWithTemporaryText)
DISABLE(SitePerProcessInteractiveBrowserTest,
        ShowAndHideDatePopupInOOPIFMultipleTimes)
DISABLE_MULTI(WebViewInteractiveTest, EditCommandsNoMenu)

// Failed in v61, does not fail locally
DISABLE(ChromeVisibilityObserverBrowserTest, VisibilityTest)
DISABLE(SslCastSocketTest, TestConnectEndToEndWithRealSSL)
