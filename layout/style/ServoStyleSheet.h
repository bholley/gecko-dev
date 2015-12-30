#ifndef mozilla_ServoStyleSheet_h
#define mozilla_ServoStyleSheet_h

#include "mozilla/StyleSheet.h"
#include "mozilla/css/Loader.h"
#include "mozilla/servo/ExternalAPI.h"

namespace mozilla {

class ServoStyleSheet final : public StyleSheet
{
public:
  ServoStyleSheet(CORSMode aCORSMode,
                  net::ReferrerPolicy aReferrerPolicy,
                  const dom::SRIMetadata& aIntegrity);

  NS_DECL_ISUPPORTS

  virtual StyleImplementation Implementation() const override { return StyleImplementation::Servo; }

  virtual nsIURI* GetSheetURI() const override;
  virtual nsIURI* GetOriginalURI() const override;
  virtual nsIURI* GetBaseURI() const override;
  virtual void SetURIs(nsIURI* aSheetURI, nsIURI* aOriginalSheetURI, nsIURI* aBaseURI) override;

  virtual bool IsApplicable() const override;
  virtual void SetComplete() override;
  virtual bool HasRules() const override;

  virtual nsIDocument* GetOwningDocument() const override;
  virtual void SetOwningDocument(nsIDocument* aDocument) override;

  virtual void SetOwningNode(nsINode* aOwningNode) override { mOwningNode = aOwningNode; /* Not ref counted */ }
  virtual nsINode* GetOwningNode() const override { return mOwningNode; }

  virtual StyleSheet* GetParentSheet() const override;
  virtual void AppendStyleSheet(StyleSheet* aSheet) override;

  virtual nsIPrincipal* Principal() const override;
  virtual void SetPrincipal(nsIPrincipal* aPrincipal) override;

  virtual CORSMode GetCORSMode() const override;
  virtual net::ReferrerPolicy GetReferrerPolicy() const override;
  virtual dom::SRIMetadata GetIntegrity() const override;

  virtual nsICSSLoaderObserver* GetChildSheetLoadObserver() override;

  void ParseSheet(const nsAString& aInput,
                  nsIURI* aSheetURI,
                  nsIURI* aBaseURI,
                  nsIPrincipal* aSheetPrincipal,
                  uint32_t aLineNumber,
                  mozilla::css::SheetParsingMode aParsingMode);

protected:
  virtual ~ServoStyleSheet() override;

private:
  void DropSheet();

  nsCOMPtr<nsIURI> mSheetURI;
  nsCOMPtr<nsIURI> mOriginalURI;
  nsCOMPtr<nsIURI> mBaseURI;
  nsCOMPtr<nsIPrincipal> mPrincipal;
  nsIDocument* mDocument; // weak ref; parents maintain this for their children
  nsINode* mOwningNode; // weak ref

  ServoArcStyleSheet* mSheet;

  CORSMode mCORSMode;
  net::ReferrerPolicy mReferrerPolicy;
  dom::SRIMetadata mIntegrity;

  bool mComplete;
  bool mDisabled;
#ifdef DEBUG
  bool mPrincipalSet;
#endif
};

inline ServoStyleSheet*
mozilla::StyleSheet::AsServo()
{
  MOZ_ASSERT(IsServo());
  return static_cast<ServoStyleSheet*>(this);
}

} // namespace mozilla

#endif // !mozilla_ServoStyleSheet_h
