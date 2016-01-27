/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/ServoStyleSheet.h"

#include "nsNullPrincipal.h"

using namespace mozilla::dom;

namespace mozilla {

ServoStyleSheet::ServoStyleSheet(CORSMode aCORSMode,
                                 net::ReferrerPolicy aReferrerPolicy,
                                 const dom::SRIMetadata& aIntegrity)
  : mPrincipal(nsNullPrincipal::Create())
  , mDocument(nullptr)
  , mOwningNode(nullptr)
  , mSheet(nullptr)
  , mCORSMode(aCORSMode)
  , mReferrerPolicy(aReferrerPolicy)
  , mIntegrity(aIntegrity)
  , mParsingMode(css::eUserSheetFeatures)
  , mComplete(false)
  , mDisabled(false)
  , mPrincipalSet(false)
{
  if (!mPrincipal) {
    NS_RUNTIMEABORT("nsNullPrincipal::Init failed");
  }
}

ServoStyleSheet::~ServoStyleSheet()
{
  DropSheet();
}

nsIURI*
ServoStyleSheet::GetSheetURI() const
{
  return mSheetURI;
}

nsIURI*
ServoStyleSheet::GetOriginalURI() const
{
  return mOriginalURI;
}

nsIURI*
ServoStyleSheet::GetBaseURI() const
{
  return mBaseURI;
}

void
ServoStyleSheet::SetURIs(nsIURI* aSheetURI, nsIURI* aOriginalSheetURI, nsIURI* aBaseURI)
{
  MOZ_ASSERT(aSheetURI);
  MOZ_ASSERT(aBaseURI);

 mSheetURI = aSheetURI;
  mOriginalURI = aOriginalSheetURI;
  mBaseURI = aBaseURI;
}

bool
ServoStyleSheet::IsApplicable() const
{
  return !mDisabled && mComplete;
}

void
ServoStyleSheet::SetComplete()
{
  MOZ_ASSERT(!mComplete);

  mComplete = true;

  // XXX Do the things that CSSStyleSheet::SetComplete does on mDocument
  // and mOwningNode.

  if (mDocument) {
    MOZ_CRASH("not implemented");
  }

  if (mOwningNode /* ... */) {
    MOZ_CRASH("not implemented");
  }
}

void
ServoStyleSheet::SetParsingMode(css::SheetParsingMode aMode)
{
  mParsingMode = aMode;
}

bool
ServoStyleSheet::HasRules() const
{
  MOZ_CRASH("stylo: not implemented");
}

nsIDocument*
ServoStyleSheet::GetOwningDocument() const
{
  return mDocument;
}

void
ServoStyleSheet::SetOwningDocument(nsIDocument* aDocument)
{
  // Not refcounted.
  mDocument = aDocument;

  // XXXbholley: May need to account for child sheets when we support them.
}

void
ServoStyleSheet::SetOwningNode(nsINode* aOwningNode)
{
  // Not refcounted.
  mOwningNode = aOwningNode;
}

nsINode*
ServoStyleSheet::GetOwnerNode() const
{
  return mOwningNode;
}

StyleSheetHandle
ServoStyleSheet::GetParentSheet() const
{
  MOZ_CRASH("stylo: not implemented");
}

void
ServoStyleSheet::AppendStyleSheet(StyleSheetHandle aSheet)
{
  MOZ_CRASH("stylo: not implemented");
}

nsIPrincipal*
ServoStyleSheet::Principal() const
{
  return mPrincipal;
}

void
ServoStyleSheet::SetPrincipal(nsIPrincipal* aPrincipal)
{
  MOZ_ASSERT(!mPrincipalSet);

  if (aPrincipal) {
    mPrincipal = aPrincipal;
#ifdef DEBUG
    mPrincipalSet = true;
#endif
  }
}

CORSMode
ServoStyleSheet::GetCORSMode() const
{
  return mCORSMode;
}

net::ReferrerPolicy
ServoStyleSheet::GetReferrerPolicy() const
{
  return mReferrerPolicy;
}

void
ServoStyleSheet::GetIntegrity(dom::SRIMetadata& aResult) const
{
  aResult = mIntegrity;
}

void
ServoStyleSheet::ParseSheet(const nsAString& aInput,
                            nsIURI* aSheetURI,
                            nsIURI* aBaseURI,
                            nsIPrincipal* aSheetPrincipal,
                            uint32_t aLineNumber,
                            css::SheetParsingMode aParsingMode)
{
  DropSheet();

  NS_ConvertUTF16toUTF8 input(aInput);
  mSheet = Servo_StylesheetFromUTF8Bytes(
      reinterpret_cast<const uint8_t*>(input.get()), input.Length());
}

void
ServoStyleSheet::DropSheet()
{
  if (mSheet) {
    Servo_DropStylesheet(mSheet);
    mSheet = nullptr;
  }
}

size_t
ServoStyleSheet::SizeOfIncludingThis(MallocSizeOf aMallocSizeOf) const
{
  MOZ_CRASH("stylo: not implemented");
}

#ifdef DEBUG
void
ServoStyleSheet::List(FILE* aOut, int32_t aIndex) const
{
  MOZ_CRASH("stylo: not implemented");
}
#endif

} // namespace mozilla
