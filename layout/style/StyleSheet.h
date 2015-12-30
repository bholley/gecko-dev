#ifndef mozilla_StyleSheet_h
#define mozilla_StyleSheet_h

#include "mozilla/net/ReferrerPolicy.h"
#include "mozilla/dom/SRIMetadata.h"
#include "mozilla/CORSMode.h"
#include "mozilla/StyleImplementation.h"
#include "nsICSSLoaderObserver.h"
#include "nsIURI.h"

namespace mozilla {
class CSSStyleSheet;
class ServoStyleSheet;
}
class nsIDocument;

namespace mozilla {

// b413b152-3e32-47bf-ab7a-1021e19c8393
#define STYLE_SHEET_IMPL_CID \
{ 0xb413b152, 0x3e32, 0x47bf, \
  { 0xab, 0x7a, 0x10, 0x21, 0xe1, 0x9c, 0x83, 0x93 } }

class StyleSheet : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(STYLE_SHEET_IMPL_CID)

  bool IsGecko() const { return Implementation() == StyleImplementation::Gecko; }
  bool IsServo() const { return Implementation() == StyleImplementation::Servo; }
  inline CSSStyleSheet* AsGecko();
  inline const CSSStyleSheet* AsGecko() const;
  inline ServoStyleSheet* AsServo();
  inline const ServoStyleSheet* AsServo() const;

  virtual nsIURI* GetSheetURI() const = 0;
  virtual nsIURI* GetOriginalURI() const = 0;
  virtual nsIURI* GetBaseURI() const = 0;
  virtual void SetURIs(nsIURI* aSheetURI, nsIURI* aOriginalSheetURI, nsIURI* aBaseURI) = 0;

  virtual bool IsApplicable() const = 0;
  virtual void SetComplete() = 0;
  virtual bool HasRules() const = 0;

  virtual nsIDocument* GetOwningDocument() const = 0;
  virtual void SetOwningDocument(nsIDocument* aDocument) = 0;

  virtual nsINode* GetOwningNode() const = 0;
  virtual void SetOwningNode(nsINode* aNode) = 0;

  virtual StyleSheet* GetParentSheet() const = 0;
  virtual void AppendStyleSheet(StyleSheet* aSheet) = 0;

  virtual nsIPrincipal* Principal() const = 0;
  virtual void SetPrincipal(nsIPrincipal* aPrincipal) = 0;

  virtual CORSMode GetCORSMode() const = 0;
  virtual net::ReferrerPolicy GetReferrerPolicy() const = 0;
  virtual dom::SRIMetadata GetIntegrity() const = 0;

  virtual nsICSSLoaderObserver* GetChildSheetLoadObserver() = 0;

protected:
  virtual ~StyleSheet() {}
  virtual StyleImplementation Implementation() const = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(StyleSheet, STYLE_SHEET_IMPL_CID)

} // namespace mozilla

#endif // mozilla_StyleSheet_h
