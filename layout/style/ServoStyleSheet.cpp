#include "mozilla/ServoStyleSheet.h"

#include "nsNullPrincipal.h"

using namespace mozilla;

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
  , mComplete(false)
  , mDisabled(false)
#ifdef DEBUG
  , mPrincipalSet(false)
#endif
{
  if (!mPrincipal) {
    NS_RUNTIMEABORT("nsNullPrincipal::Init failed");
  }
}

ServoStyleSheet::~ServoStyleSheet()
{
  DropSheet();
}

NS_IMPL_ISUPPORTS0(ServoStyleSheet)

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

bool
ServoStyleSheet::HasRules() const
{
  MOZ_CRASH("not implemented");
}

nsIDocument*
ServoStyleSheet::GetOwningDocument() const
{
  return mDocument;
}

void
ServoStyleSheet::SetOwningDocument(nsIDocument* aDocument)
{
  MOZ_CRASH("not implemented");
}

StyleSheet*
ServoStyleSheet::GetParentSheet() const
{
  MOZ_CRASH("not implemented");
}

void
ServoStyleSheet::AppendStyleSheet(StyleSheet* aSheet)
{
  MOZ_CRASH("not implemented");
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

dom::SRIMetadata
ServoStyleSheet::GetIntegrity() const
{
  return mIntegrity;
}

nsICSSLoaderObserver*
ServoStyleSheet::GetChildSheetLoadObserver()
{
  MOZ_CRASH("not implemented");
}

void
ServoStyleSheet::ParseSheet(const nsAString& aInput,
                            nsIURI* aSheetURI,
                            nsIURI* aBaseURI,
                            nsIPrincipal* aSheetPrincipal,
                            uint32_t aLineNumber,
                            mozilla::css::SheetParsingMode aParsingMode)
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
