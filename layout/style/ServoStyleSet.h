#ifndef mozilla_ServoStyleSet_h
#define mozilla_ServoStyleSet_h

#include "mozilla/ServoStyleSheet.h"
#include "mozilla/StyleSet.h"
#include "mozilla/EnumeratedArray.h"

namespace mozilla {

class ServoStyleSet : public StyleSet
{
public:
  ServoStyleSet();

  virtual StyleImplementation Implementation() const override { return StyleImplementation::Servo; }

  virtual void Init(nsPresContext* aPresContext) override;
  virtual void BeginShutdown() override;
  virtual void Shutdown() override;

  virtual bool GetAuthorStyleDisabled() override;
  virtual nsresult SetAuthorStyleDisabled(bool aStyleDisabled) override;

  virtual void BeginUpdate() override;
  virtual nsresult EndUpdate() override;

  // resolve a style context
  virtual already_AddRefed<nsStyleContext>
  ResolveStyleFor(mozilla::dom::Element* aElement,
                  nsStyleContext* aParentContext) override;

  virtual already_AddRefed<nsStyleContext>
  ResolveStyleFor(mozilla::dom::Element* aElement,
                  nsStyleContext* aParentContext,
                  TreeMatchContext& aTreeMatchContext) override;

  virtual already_AddRefed<nsStyleContext>
  ResolveStyleForNonElement(nsStyleContext* aParentContext) override;

  virtual already_AddRefed<nsStyleContext>
  ResolvePseudoElementStyle(mozilla::dom::Element* aParentElement,
                            nsCSSPseudoElements::Type aType,
                            nsStyleContext* aParentContext,
                            mozilla::dom::Element* aPseudoElement) override;

  // aFlags is an nsStyleSet flags bitfield
  virtual already_AddRefed<nsStyleContext>
  ResolveAnonymousBoxStyle(nsIAtom* aPseudoTag, nsStyleContext* aParentContext,
                           uint32_t aFlags = 0) override;

  // manage the set of style sheets in the style set
  virtual nsresult AppendStyleSheet(mozilla::SheetType aType,
                                    mozilla::StyleSheet* aSheet) override;
  virtual nsresult PrependStyleSheet(mozilla::SheetType aType,
                                     mozilla::StyleSheet* aSheet) override;
  virtual nsresult RemoveStyleSheet(mozilla::SheetType aType,
                                    mozilla::StyleSheet* aSheet) override;
  virtual nsresult ReplaceSheets(mozilla::SheetType aType,
                                 const nsTArray<RefPtr<mozilla::StyleSheet>>& aNewSheets) override;
  virtual nsresult InsertStyleSheetBefore(mozilla::SheetType aType,
                                          mozilla::StyleSheet* aNewSheet,
                                          mozilla::StyleSheet* aReferenceSheet) override;

  virtual int32_t SheetCount(mozilla::SheetType aType) const override;
  virtual mozilla::StyleSheet* StyleSheetAt(mozilla::SheetType aType,
                                               int32_t aIndex) const override;

  virtual nsresult RemoveDocStyleSheet(mozilla::StyleSheet* aSheet) override;
  virtual nsresult AddDocStyleSheet(mozilla::StyleSheet* aSheet,
                                    nsIDocument* aDocument) override;

  // check whether there is ::before/::after style for an element
  virtual already_AddRefed<nsStyleContext>
  ProbePseudoElementStyle(mozilla::dom::Element* aParentElement,
                          nsCSSPseudoElements::Type aType,
                          nsStyleContext* aParentContext) override;

  virtual already_AddRefed<nsStyleContext>
  ProbePseudoElementStyle(mozilla::dom::Element* aParentElement,
                          nsCSSPseudoElements::Type aType,
                          nsStyleContext* aParentContext,
                          TreeMatchContext& aTreeMatchContext,
                          mozilla::dom::Element* aPseudoElement = nullptr) override;

private:
  mozilla::EnumeratedArray<mozilla::SheetType, mozilla::SheetType::Count,
                           nsTArray<RefPtr<mozilla::ServoStyleSheet>>> mSheets;
  int32_t mBatching;
};

inline ServoStyleSet*
mozilla::StyleSet::AsServo()
{
  MOZ_ASSERT(IsServo());
  return static_cast<ServoStyleSet*>(this);
}

} // namespace mozilla

#endif // mozilla_ServoStyleSet_h
