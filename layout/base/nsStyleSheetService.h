/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* implementation of interface for managing user and user-agent style sheets */

#ifndef nsStyleSheetService_h_
#define nsStyleSheetService_h_

#include "nsCOMArray.h"
#include "nsCOMPtr.h"
#include "nsIMemoryReporter.h"
#include "nsIStyleSheetService.h"
#include "mozilla/Attributes.h"
#include "mozilla/StyleImplementation.h"
#include "mozilla/MemoryReporting.h"

namespace mozilla {
class StyleSheet;
}
class nsICategoryManager;
class nsIMemoryReporter;
class nsISimpleEnumerator;

// 98d6c191-d285-414d-b646-e9198b0f584c
#define NS_STYLESHEETSERVICE_CID \
{ 0x98d6c191, 0xd285, 0x414d, \
  { 0xb6, 0x46, 0xe9, 0x19, 0x8b, 0x0f, 0x58, 0x4c } }

#define NS_STYLESHEETSERVICE_CONTRACTID \
  "@mozilla.org/content/style-sheet-service;1"

class nsStyleSheetService final
  : public nsIStyleSheetService
  , public nsIMemoryReporter
{
 public:
  nsStyleSheetService();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISTYLESHEETSERVICE
  NS_DECL_NSIMEMORYREPORTER

  nsresult Init();

  nsTArray<RefPtr<mozilla::StyleSheet>>* AgentStyleSheets(mozilla::StyleImplementation aImpl)
  {
    return &Sheets(aImpl)[AGENT_SHEET];
  }
  nsTArray<RefPtr<mozilla::StyleSheet>>* UserStyleSheets(mozilla::StyleImplementation aImpl)
  {
    return &Sheets(aImpl)[USER_SHEET];
  }
  nsTArray<RefPtr<mozilla::StyleSheet>>* AuthorStyleSheets(mozilla::StyleImplementation aImpl)
  {
    return &Sheets(aImpl)[AUTHOR_SHEET];
  }

  size_t SizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf) const;

  static nsStyleSheetService *GetInstance();
  static nsStyleSheetService *gInstance;

private:
  ~nsStyleSheetService();

  nsTArray<RefPtr<mozilla::StyleSheet>> (& Sheets(mozilla::StyleImplementation aImpl))[3]
  {
    return aImpl == mozilla::StyleImplementation::Gecko ? mSheets_Gecko : mSheets_Servo;
  }

  void RegisterFromEnumerator(nsICategoryManager  *aManager,
                                          const char          *aCategory,
                                          nsISimpleEnumerator *aEnumerator,
                                          uint32_t             aSheetType);

  int32_t FindSheetByURI(const nsTArray<RefPtr<mozilla::StyleSheet>>& aSheets,
                         nsIURI* aSheetURI);

  // Like LoadAndRegisterSheet, but doesn't notify.  If successful, the
  // new sheet will be the last sheet in mSheets[aSheetType].
  nsresult LoadAndRegisterSheetInternal(nsIURI *aSheetURI,
                                        uint32_t aSheetType);

  nsTArray<RefPtr<mozilla::StyleSheet>> mSheets_Gecko[3];
  nsTArray<RefPtr<mozilla::StyleSheet>> mSheets_Servo[3];
};

#endif
