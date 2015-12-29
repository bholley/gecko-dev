#ifndef mozilla_Restyler
#define mozilla_Restyler

#include "mozilla/StyleBackendType.h"
#include "nsChangeHint.h"
#include "prenv.h"

namespace mozilla {
namespace dom {
class Element;
}
class RestyleManager;
}
class nsIAtom;
class nsIContent;
class nsIFrame;
class nsStyleChangeList;

namespace mozilla {

class Restyler
{
public:
  static bool StyloEnabled()
  {
    static bool enabled = PR_GetEnv("MOZ_STYLO");
    return enabled;
  }

  virtual StyleBackendType Implementation() const = 0;
  bool IsGecko() const { return Implementation() == StyleBackendType::Gecko; }
  bool IsServo() const { return Implementation() == StyleBackendType::Servo; }
  inline RestyleManager* AsGecko();

  NS_INLINE_DECL_REFCOUNTING(mozilla::Restyler)

  virtual void Disconnect() = 0;

  virtual void PostRestyleEvent(mozilla::dom::Element* aElement,
                                nsRestyleHint aRestyleHint,
                                nsChangeHint aMinChangeHint) = 0;
  virtual void PostRestyleEventForLazyConstruction() = 0;
  virtual void RebuildAllStyleData(nsChangeHint aExtraHint,
                                   nsRestyleHint aRestyleHint) = 0;
  virtual void PostRebuildAllStyleDataEvent(nsChangeHint aExtraHint,
                                            nsRestyleHint aRestyleHint) = 0;
  virtual void ProcessPendingRestyles() = 0;

  virtual void RestyleForInsertOrChange(mozilla::dom::Element* aContainer, nsIContent* aChild) = 0;
  virtual void RestyleForAppend(mozilla::dom::Element* aContainer, nsIContent* aFirstNewContent) = 0;
  virtual void RestyleForRemove(mozilla::dom::Element* aContainer,
                                nsIContent* aOldChild,
                                nsIContent* aFollowingSibling) = 0;
  virtual nsresult ContentStateChanged(nsIContent*   aContent,
                                       EventStates aStateMask) = 0;
  virtual void AttributeWillChange(mozilla::dom::Element* aElement,
                                   int32_t  aNameSpaceID,
                                   nsIAtom* aAttribute,
                                   int32_t  aModType,
                                   const nsAttrValue* aNewValue) = 0;
  virtual void AttributeChanged(mozilla::dom::Element* aElement,
                                int32_t  aNameSpaceID,
                                nsIAtom* aAttribute,
                                int32_t  aModType,
                                const nsAttrValue* aOldValue) = 0;

  virtual nsresult ProcessFrameReconstructions(nsStyleChangeList& aChangeList) = 0;
  virtual nsresult ReparentStyleContext(nsIFrame* aFrame) = 0;

protected:
  virtual ~Restyler() {}
};

}

#endif // mozilla_Restyler
