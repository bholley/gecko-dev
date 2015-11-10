#ifndef mozilla_StyleSet_h
#define mozilla_StyleSet_h

#include "mozilla/RefPtr.h"
#include "mozilla/StyleImplementation.h"
#include "mozilla/SheetType.h"
#include "nsCSSPseudoElements.h"
#include "nsIAtom.h"
#include "nsTArray.h"

// TODO:
//   * change CSSStyleSheets to a generic StyleSheet type
//   * change nsStyleContext to a generic StyleContext type

namespace mozilla {
namespace dom {
class Element;
}
class CSSStyleSheet;
}
class nsStyleContext;
class nsPresContext;
struct TreeMatchContext;

namespace mozilla {

class StyleSet
{
public:
  virtual ~StyleSet() {}

  bool IsGecko() const { return Implementation() == StyleImplementation::Gecko; }
  bool IsServo() const { return Implementation() == StyleImplementation::Servo; }
  inline nsStyleSet* AsGecko();

  virtual void Init(nsPresContext* aPresContext) = 0;
  virtual void BeginShutdown() = 0;
  virtual void Shutdown() = 0;

  virtual bool GetAuthorStyleDisabled() = 0;
  virtual nsresult SetAuthorStyleDisabled(bool aStyleDisabled) = 0;

  virtual void BeginUpdate() = 0;
  virtual nsresult EndUpdate() = 0;

  // resolve a style context
  virtual already_AddRefed<nsStyleContext>
  ResolveStyleFor(mozilla::dom::Element* aElement,
                  nsStyleContext* aParentContext) = 0;

  virtual already_AddRefed<nsStyleContext>
  ResolveStyleFor(mozilla::dom::Element* aElement,
                  nsStyleContext* aParentContext,
                  TreeMatchContext& aTreeMatchContext) = 0;

  virtual already_AddRefed<nsStyleContext>
  ResolveStyleForNonElement(nsStyleContext* aParentContext) = 0;

  virtual already_AddRefed<nsStyleContext>
  ResolvePseudoElementStyle(mozilla::dom::Element* aParentElement,
                            nsCSSPseudoElements::Type aType,
                            nsStyleContext* aParentContext,
                            mozilla::dom::Element* aPseudoElement) = 0;

  // aFlags is an nsStyleSet flags bitfield
  virtual already_AddRefed<nsStyleContext>
  ResolveAnonymousBoxStyle(nsIAtom* aPseudoTag, nsStyleContext* aParentContext,
                           uint32_t aFlags = 0) = 0;

  // manage the set of style sheets in the style set
  virtual nsresult AppendStyleSheet(mozilla::SheetType aType,
                                    mozilla::CSSStyleSheet* aSheet) = 0;
  virtual nsresult PrependStyleSheet(mozilla::SheetType aType,
                                     mozilla::CSSStyleSheet* aSheet) = 0;
  virtual nsresult RemoveStyleSheet(mozilla::SheetType aType,
                                    mozilla::CSSStyleSheet* aSheet) = 0;
  virtual nsresult ReplaceSheets(mozilla::SheetType aType,
                                 const nsTArray<RefPtr<mozilla::CSSStyleSheet>>& aNewSheets) = 0;
  virtual nsresult InsertStyleSheetBefore(mozilla::SheetType aType,
                                          mozilla::CSSStyleSheet* aNewSheet,
                                          mozilla::CSSStyleSheet* aReferenceSheet) = 0;

  virtual int32_t SheetCount(mozilla::SheetType aType) const = 0;
  virtual mozilla::CSSStyleSheet* StyleSheetAt(mozilla::SheetType aType,
                                               int32_t aIndex) const = 0;

  virtual nsresult RemoveDocStyleSheet(mozilla::CSSStyleSheet* aSheet) = 0;
  virtual nsresult AddDocStyleSheet(mozilla::CSSStyleSheet* aSheet,
                                    nsIDocument* aDocument) = 0;

  // check whether there is ::before/::after style for an element
  virtual already_AddRefed<nsStyleContext>
  ProbePseudoElementStyle(mozilla::dom::Element* aParentElement,
                          nsCSSPseudoElements::Type aType,
                          nsStyleContext* aParentContext) = 0;

  virtual already_AddRefed<nsStyleContext>
  ProbePseudoElementStyle(mozilla::dom::Element* aParentElement,
                          nsCSSPseudoElements::Type aType,
                          nsStyleContext* aParentContext,
                          TreeMatchContext& aTreeMatchContext,
                          mozilla::dom::Element* aPseudoElement = nullptr) = 0;


protected:
  virtual StyleImplementation Implementation() const = 0;
};

} // namespace mozilla

#endif // mozilla_StyleSet_h
