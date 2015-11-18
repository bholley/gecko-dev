/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsLayoutStylesheetCache_h__
#define nsLayoutStylesheetCache_h__

#include "nsIMemoryReporter.h"
#include "nsIObserver.h"
#include "nsAutoPtr.h"
#include "mozilla/Attributes.h"
#include "mozilla/MemoryReporting.h"
#include "mozilla/StaticPtr.h"
#include "mozilla/css/Loader.h"

class nsIFile;
class nsIURI;

namespace mozilla {
class StyleSheet;
} // namespace mozilla

class nsLayoutStylesheetCache final
 : public nsIObserver
 , public nsIMemoryReporter
{
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIMEMORYREPORTER

  static mozilla::StyleSheet* ScrollbarsSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* FormsSheet(mozilla::StyleImplementation aImpl);
  // This function is expected to return nullptr when the dom.forms.number
  // pref is disabled.
  static mozilla::StyleSheet* NumberControlSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* UserContentSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* UserChromeSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* UASheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* HTMLSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* MinimalXULSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* XULSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* QuirkSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* SVGSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* MathMLSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* CounterStylesSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* NoScriptSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* NoFramesSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* ChromePreferenceSheet(mozilla::StyleImplementation aImpl, nsPresContext* aPresContext);
  static mozilla::StyleSheet* ContentPreferenceSheet(mozilla::StyleImplementation aImpl, nsPresContext* aPresContext);
  static mozilla::StyleSheet* ContentEditableSheet(mozilla::StyleImplementation aImpl);
  static mozilla::StyleSheet* DesignModeSheet(mozilla::StyleImplementation aImpl);

  static void InvalidatePreferenceSheets();

  static void Shutdown();

  size_t SizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf) const;

private:
  nsLayoutStylesheetCache(mozilla::StyleImplementation aImpl);
  ~nsLayoutStylesheetCache();

  static nsLayoutStylesheetCache* EnsureGlobal(mozilla::StyleImplementation aImpl);
  void InitFromProfile();
  void InitMemoryReporter();
  void LoadSheetURL(const char* aURL,
                    RefPtr<mozilla::StyleSheet>& aSheet,
                    mozilla::css::SheetParsingMode aParsingMode);
  void LoadSheetFile(nsIFile* aFile,
                     RefPtr<mozilla::StyleSheet>& aSheet,
                     mozilla::css::SheetParsingMode aParsingMode);
  void LoadSheet(nsIURI* aURI, RefPtr<mozilla::StyleSheet>& aSheet,
                 mozilla::css::SheetParsingMode aParsingMode);
  static void InvalidateSheet(RefPtr<mozilla::StyleSheet>& aSheet);
  static void DependentPrefChanged(const char* aPref, void* aData);
  void BuildPreferenceSheet(RefPtr<mozilla::StyleSheet>& aSheet,
                            nsPresContext* aPresContext);

  static mozilla::StaticRefPtr<nsLayoutStylesheetCache> gStyleCache_Gecko;
  static mozilla::StaticRefPtr<nsLayoutStylesheetCache> gStyleCache_Servo;
  static mozilla::css::Loader* gCSSLoader_Gecko;
  static mozilla::css::Loader* gCSSLoader_Servo;
  RefPtr<mozilla::StyleSheet> mChromePreferenceSheet;
  RefPtr<mozilla::StyleSheet> mContentEditableSheet;
  RefPtr<mozilla::StyleSheet> mContentPreferenceSheet;
  RefPtr<mozilla::StyleSheet> mCounterStylesSheet;
  RefPtr<mozilla::StyleSheet> mDesignModeSheet;
  RefPtr<mozilla::StyleSheet> mFormsSheet;
  RefPtr<mozilla::StyleSheet> mHTMLSheet;
  RefPtr<mozilla::StyleSheet> mMathMLSheet;
  RefPtr<mozilla::StyleSheet> mMinimalXULSheet;
  RefPtr<mozilla::StyleSheet> mNoFramesSheet;
  RefPtr<mozilla::StyleSheet> mNoScriptSheet;
  RefPtr<mozilla::StyleSheet> mNumberControlSheet;
  RefPtr<mozilla::StyleSheet> mQuirkSheet;
  RefPtr<mozilla::StyleSheet> mSVGSheet;
  RefPtr<mozilla::StyleSheet> mScrollbarsSheet;
  RefPtr<mozilla::StyleSheet> mUASheet;
  RefPtr<mozilla::StyleSheet> mUserChromeSheet;
  RefPtr<mozilla::StyleSheet> mUserContentSheet;
  RefPtr<mozilla::StyleSheet> mXULSheet;

  mozilla::StyleImplementation mImplementation;
};

#endif
