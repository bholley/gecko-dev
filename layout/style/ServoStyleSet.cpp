#include "mozilla/ServoStyleSet.h"

#include "nsCSSAnonBoxes.h"

using namespace mozilla;
using namespace mozilla::dom;

ServoStyleSet::ServoStyleSet()
  : mBatching(0)
{
}

void
ServoStyleSet::Init(nsPresContext* aPresContext)
{
}

void
ServoStyleSet::BeginShutdown()
{
}

void
ServoStyleSet::Shutdown()
{
}

bool
ServoStyleSet::GetAuthorStyleDisabled()
{
  return false;
}

nsresult
ServoStyleSet::SetAuthorStyleDisabled(bool aStyleDisabled)
{
  MOZ_CRASH("not implemented");
}

void
ServoStyleSet::BeginUpdate()
{
  ++mBatching;
}

nsresult
ServoStyleSet::EndUpdate()
{
  MOZ_ASSERT(mBatching > 0);
  if (--mBatching > 0) {
    return NS_OK;
  }

  // ... do something ...

  return NS_OK;
}

// resolve a style context
already_AddRefed<nsStyleContext>
ServoStyleSet::ResolveStyleFor(Element* aElement,
                               nsStyleContext* aParentContext)
{
  MOZ_CRASH("not implemented");
}

already_AddRefed<nsStyleContext>
ServoStyleSet::ResolveStyleFor(Element* aElement,
                               nsStyleContext* aParentContext,
                               TreeMatchContext& aTreeMatchContext)
{
  MOZ_CRASH("not implemented");
}

already_AddRefed<nsStyleContext>
ServoStyleSet::ResolveStyleForNonElement(nsStyleContext* aParentContext)
{
  MOZ_CRASH("not implemented");
}

already_AddRefed<nsStyleContext>
ServoStyleSet::ResolvePseudoElementStyle(Element* aParentElement,
                                         nsCSSPseudoElements::Type aType,
                                         nsStyleContext* aParentContext,
                                         Element* aPseudoElement)
{
  MOZ_CRASH("not implemented");
}

// aFlags is an nsStyleSet flags bitfield
already_AddRefed<nsStyleContext>
ServoStyleSet::ResolveAnonymousBoxStyle(nsIAtom* aPseudoTag,
                                        nsStyleContext* aParentContext,
                                        uint32_t aFlags)
{
  MOZ_ASSERT(nsCSSAnonBoxes::IsAnonBox(aPseudoTag));

  // FIXME(heycam): Do something with eSkipParentDisplayBasedStyleFixup,
  // which is the only value of aFlags that can be passed in.
  MOZ_ASSERT(aFlags == 0 ||
             aFlags == nsStyleSet::eSkipParentDisplayBasedStyleFixup);

  MOZ_CRASH("not implemented");
}

// manage the set of style sheets in the style set
nsresult
ServoStyleSet::AppendStyleSheet(SheetType aType,
                                StyleSheet* aSheet)
{
  MOZ_ASSERT(aSheet);
  MOZ_ASSERT(aSheet->IsApplicable());
  MOZ_ASSERT(IsCSSSheetType(aType));

  ServoStyleSheet* sheet = aSheet->AsServo();

  mSheets[aType].RemoveElement(sheet);
  mSheets[aType].AppendElement(sheet);

  return NS_OK;
}

nsresult
ServoStyleSet::PrependStyleSheet(SheetType aType,
                                 StyleSheet* aSheet)
{
  MOZ_ASSERT(aSheet);
  MOZ_ASSERT(aSheet->IsApplicable());
  MOZ_ASSERT(IsCSSSheetType(aType));

  ServoStyleSheet* sheet = aSheet->AsServo();

  mSheets[aType].RemoveElement(sheet);
  mSheets[aType].InsertElementAt(0, sheet);

  return NS_OK;
}

nsresult
ServoStyleSet::RemoveStyleSheet(SheetType aType,
                                StyleSheet* aSheet)
{
  MOZ_CRASH("not implemented");
}

nsresult
ServoStyleSet::ReplaceSheets(SheetType aType,
                             const nsTArray<RefPtr<StyleSheet>>& aNewSheets)
{
  MOZ_CRASH("not implemented");
}

nsresult
ServoStyleSet::InsertStyleSheetBefore(SheetType aType,
                                      StyleSheet* aNewSheet,
                                      StyleSheet* aReferenceSheet)
{
  MOZ_CRASH("not implemented");
}

int32_t
ServoStyleSet::SheetCount(SheetType aType) const
{
  MOZ_ASSERT(IsCSSSheetType(aType));
  return mSheets[aType].Length();
}

StyleSheet*
ServoStyleSet::StyleSheetAt(SheetType aType,
                            int32_t aIndex) const
{
  MOZ_ASSERT(IsCSSSheetType(aType));
  return mSheets[aType][aIndex];
}

nsresult
ServoStyleSet::RemoveDocStyleSheet(StyleSheet* aSheet)
{
  MOZ_CRASH("not implemented");
}

nsresult
ServoStyleSet::AddDocStyleSheet(StyleSheet* aSheet,
                                nsIDocument* aDocument)
{
  MOZ_CRASH("not implemented");
}

already_AddRefed<nsStyleContext>
ServoStyleSet::ProbePseudoElementStyle(Element* aParentElement,
                                       nsCSSPseudoElements::Type aType,
                                       nsStyleContext* aParentContext)
{
  MOZ_CRASH("not implemented");
}

already_AddRefed<nsStyleContext>
ServoStyleSet::ProbePseudoElementStyle(Element* aParentElement,
                                       nsCSSPseudoElements::Type aType,
                                       nsStyleContext* aParentContext,
                                       TreeMatchContext& aTreeMatchContext,
                                       Element* aPseudoElement)
{
  MOZ_CRASH("not implemented");
}
