/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* internal interface for observing CSS style sheet loads */

#ifndef nsICSSLoaderObserver_h___
#define nsICSSLoaderObserver_h___

#include "nsISupports.h"

// 82f0c200-501d-4cd8-b63a-3433aa198fbe
#define NS_ICSSLOADEROBSERVER_IID \
{ 0x82f0c200, 0x501d, 0x4cd8, \
  { 0xb6, 0x3a, 0x34, 0x33, 0xaa, 0x19, 0x8f, 0xbe } }

namespace mozilla {
class StyleSheet;
} // namespace mozilla

class nsICSSLoaderObserver : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICSSLOADEROBSERVER_IID)

  /**
   * StyleSheetLoaded is called after aSheet is marked complete and before any
   * load events associated with aSheet are fired.
   * @param aSheet the sheet that was loaded. Guaranteed to always be
   *        non-null, even if aStatus indicates failure.
   * @param aWasAlternate whether the sheet was an alternate.  This will always
   *        match the value LoadStyleLink or LoadInlineStyle returned in
   *        aIsAlternate if one of those methods were used to load the sheet,
   *        and will always be false otherwise.
   * @param aStatus is a success code if the sheet loaded successfully and a
   *        failure code otherwise.  Note that successful load of aSheet
   *        doesn't indicate anything about whether the data actually parsed
   *        as CSS, and doesn't indicate anything about the status of any child
   *        sheets of aSheet.
   */
  NS_IMETHOD StyleSheetLoaded(mozilla::StyleSheet* aSheet,
                              bool aWasAlternate,
                              nsresult aStatus) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsICSSLoaderObserver, NS_ICSSLOADEROBSERVER_IID)

#endif // nsICSSLoaderObserver_h___
