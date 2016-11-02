/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/ServoRestyleManager.h"
#include "mozilla/ServoBindings.h"
#include "mozilla/ServoStyleSet.h"
#include "mozilla/dom/ChildIterator.h"
#include "nsContentUtils.h"
#include "nsPrintfCString.h"
#include "nsStyleChangeList.h"

using namespace mozilla::dom;

namespace mozilla {

ServoRestyleManager::ServoRestyleManager(nsPresContext* aPresContext)
  : RestyleManagerBase(aPresContext)
{
}

void
ServoRestyleManager::PostRestyleEvent(Element* aElement,
                                      nsRestyleHint aRestyleHint,
                                      nsChangeHint aMinChangeHint)
{
  if (MOZ_UNLIKELY(IsDisconnected()) ||
      MOZ_UNLIKELY(PresContext()->PresShell()->IsDestroying())) {
    return;
  }

  if (aRestyleHint == 0 && !aMinChangeHint && !HasPendingRestyles()) {
    return; // Nothing to do.
  }

  if (aRestyleHint || aMinChangeHint) {
    Servo_NoteExplicitHints(aElement, aRestyleHint, aMinChangeHint);
  }

  PostRestyleEventInternal(false);
}

void
ServoRestyleManager::PostRestyleEventForLazyConstruction()
{
  PostRestyleEventInternal(true);
}

void
ServoRestyleManager::RebuildAllStyleData(nsChangeHint aExtraHint,
                                         nsRestyleHint aRestyleHint)
{
  NS_WARNING("stylo: ServoRestyleManager::RebuildAllStyleData not implemented");
}

void
ServoRestyleManager::PostRebuildAllStyleDataEvent(nsChangeHint aExtraHint,
                                                  nsRestyleHint aRestyleHint)
{
  NS_WARNING("stylo: ServoRestyleManager::PostRebuildAllStyleDataEvent not implemented");
}

void
ServoRestyleManager::RecreateStyleContexts(Element* aElement,
                                           nsStyleContext* aParentContext,
                                           ServoStyleSet* aStyleSet,
                                           nsStyleChangeList& aChangeListToProcess)
{
  nsIFrame* primaryFrame = aElement->GetPrimaryFrame();

  // XXX: assert everything consumed and bits cleared.
  //
  // XXX: Add todo about fast reject.
  
  auto changeHint = nsChangeHint(0);
  if (Servo_ConsumeRestyle(aElement, &changeHint)) {

    // Add the new change hint to the list of elements to process if
    // we need to do any work.
    if (changeHint) {
      aChangeListToProcess.AppendChange(primaryFrame, aElement, changeHint);
    }

    if (primaryFrame) {
      RefPtr<ServoComputedValues> computedValues =
        Servo_ComputedValues_Get(aElement).Consume();
      MOZ_ASSERT(computedValues);

      // Hold the old style context alive, because it could become a dangling
      // pointer during the replacement. In practice it's not a huge deal (on
      // GetNextContinuationWithSameStyle the pointer is not dereferenced, only
      // compared), but better not playing with dangling pointers if not needed.
      RefPtr<nsStyleContext> oldStyleContext = primaryFrame->StyleContext();
      MOZ_ASSERT(oldStyleContext);

      RefPtr<nsStyleContext> newContext =
        aStyleSet->GetContext(computedValues.forget(), aParentContext, nullptr,
                              CSSPseudoElementType::NotPseudo);

      // XXX This could not always work as expected: there are kinds of content
      // with the first split and the last sharing style, but others not. We
      // should handle those properly.
      for (nsIFrame* f = primaryFrame; f;
           f = GetNextContinuationWithSameStyle(f, oldStyleContext)) {
        f->SetStyleContext(newContext);
      }

      // Update pseudo-elements state if appropriate.
      const static CSSPseudoElementType pseudosToRestyle[] = {
        CSSPseudoElementType::before,
        CSSPseudoElementType::after,
      };

      for (CSSPseudoElementType pseudoType : pseudosToRestyle) {
        nsIAtom* pseudoTag = nsCSSPseudoElements::GetPseudoAtom(pseudoType);

        if (nsIFrame* pseudoFrame = FrameForPseudoElement(aElement, pseudoTag)) {
          // TODO: we could maybe make this more performant via calling into
          // Servo just once to know which pseudo-elements we've got to restyle?
          RefPtr<nsStyleContext> pseudoContext =
            aStyleSet->ProbePseudoElementStyle(aElement, pseudoType, newContext);

          // If pseudoContext is null here, it means the frame is going away, so
          // our change hint computation should have already indicated we need
          // to reframe.
          MOZ_ASSERT_IF(!pseudoContext,
                        changeHint & nsChangeHint_ReconstructFrame);
          if (pseudoContext) {
            pseudoFrame->SetStyleContext(pseudoContext);

            // We only care restyling text nodes, since other type of nodes
            // (images), are still not supported. If that eventually changes, we
            // may have to write more code here... Or not, I don't think too
            // many inherited properties can affect those other frames.
            StyleChildrenIterator it(pseudoFrame->GetContent());
            for (nsIContent* n = it.GetNextChild(); n; n = it.GetNextChild()) {
              if (n->IsNodeOfType(nsINode::eTEXT)) {
                RefPtr<nsStyleContext> childContext =
                  aStyleSet->ResolveStyleForText(n, pseudoContext);
                MOZ_ASSERT(n->GetPrimaryFrame(),
                           "How? This node is created at FC time!");
                n->GetPrimaryFrame()->SetStyleContext(childContext);
              }
            }
          }
        }
      }
    }
  }

  if (aElement->HasRestyleDescendantsForServo()) {
    MOZ_ASSERT(primaryFrame,
               "Frame construction should be scheduled, and it takes the "
               "correct style for the children, so no need to be here.");
    StyleChildrenIterator it(aElement);
    for (nsIContent* n = it.GetNextChild(); n; n = it.GetNextChild()) {
      if (n->IsElement()) {
        RecreateStyleContexts(n->AsElement(), primaryFrame->StyleContext(),
                              aStyleSet, aChangeListToProcess);
      } else if (n->IsNodeOfType(nsINode::eTEXT)) {
        RecreateStyleContextsForText(n, primaryFrame->StyleContext(),
                                     aStyleSet);
      }
    }

    aElement->UnsetHasRestyleDescendantsForServo();
  }
}

void
ServoRestyleManager::RecreateStyleContextsForText(nsIContent* aTextNode,
                                                  nsStyleContext* aParentContext,
                                                  ServoStyleSet* aStyleSet)
{
  nsIFrame* primaryFrame = aTextNode->GetPrimaryFrame();
  if (primaryFrame) {
    RefPtr<nsStyleContext> oldStyleContext = primaryFrame->StyleContext();
    RefPtr<nsStyleContext> newContext =
      aStyleSet->ResolveStyleForText(aTextNode, aParentContext);

    for (nsIFrame* f = primaryFrame; f;
         f = GetNextContinuationWithSameStyle(f, oldStyleContext)) {
      f->SetStyleContext(newContext);
    }
  }
}

/* static */ nsIFrame*
ServoRestyleManager::FrameForPseudoElement(const nsIContent* aContent,
                                           nsIAtom* aPseudoTagOrNull)
{
  MOZ_ASSERT_IF(aPseudoTagOrNull, aContent->IsElement());
  nsIFrame* primaryFrame = aContent->GetPrimaryFrame();

  if (!aPseudoTagOrNull) {
    return primaryFrame;
  }

  if (!primaryFrame) {
    return nullptr;
  }

  // NOTE: we probably need to special-case display: contents here. Gecko's
  // RestyleManager passes the primary frame of the parent instead.
  if (aPseudoTagOrNull == nsCSSPseudoElements::before) {
    return nsLayoutUtils::GetBeforeFrameForContent(primaryFrame, aContent);
  }

  if (aPseudoTagOrNull == nsCSSPseudoElements::after) {
    return nsLayoutUtils::GetAfterFrameForContent(primaryFrame, aContent);
  }

  MOZ_CRASH("Unkown pseudo-element given to "
            "ServoRestyleManager::FrameForPseudoElement");
  return nullptr;
}

void
ServoRestyleManager::ProcessPendingRestyles()
{
  MOZ_ASSERT(PresContext()->Document(), "No document?  Pshaw!");
  MOZ_ASSERT(!nsContentUtils::IsSafeToRunScript(), "Missing a script blocker!");

  if (MOZ_UNLIKELY(!PresContext()->PresShell()->DidInitialize())) {
    // PresShell::FlushPendingNotifications doesn't early-return in the case
    // where the PreShell hasn't yet been initialized (and therefore we haven't
    // yet done the initial style traversal of the DOM tree). We should arguably
    // fix up the callers and assert against this case, but we just detect and
    // handle it for now.
    return;
  }

  if (!HasPendingRestyles()) {
    return;
  }

  ServoStyleSet* styleSet = StyleSet();
  nsIDocument* doc = PresContext()->Document();
  Element* root = doc->GetRootElement();
  if (HasPendingRestyles()) {
    MOZ_ASSERT(root);
    mInStyleRefresh = true;
    styleSet->StyleDocument();

    // First do any queued-up frame creation. (see bugs 827239 and 997506).
    //
    // XXXEmilio I'm calling this to avoid random behavior changes, since we
    // delay frame construction after styling we should re-check once our
    // model is more stable whether we can skip this call.
    //
    // Note this has to be *after* restyling, because otherwise frame
    // construction will find unstyled nodes, and that's not funny.
    PresContext()->FrameConstructor()->CreateNeededFrames();

    nsStyleChangeList changeList;
    RecreateStyleContexts(root, nullptr, styleSet, changeList);
    ProcessRestyledFrames(changeList);
    styleSet->AssertTreeIsClean();

    mInStyleRefresh = false;
  }

  IncrementRestyleGeneration();
}

void
ServoRestyleManager::RestyleForInsertOrChange(nsINode* aContainer,
                                              nsIContent* aChild)
{
  //
  // XXXbholley: We need the Gecko logic here to correctly restyle for things
  // like :empty and positional selectors (though we may not need to post
  // restyle events as agressively as the Gecko path does).
  //
  // Bug 1297899 tracks this work.
  //
}

void
ServoRestyleManager::ContentInserted(nsINode* aContainer, nsIContent* aChild)
{
  if (aContainer == aContainer->OwnerDoc()) {
    // If we're getting this notification for the insertion of a root element,
    // that means either:
    //   (a) We initialized the PresShell before the root element existed, or
    //   (b) The root element was removed and it or another root is being
    //       inserted.
    //
    // Either way the whole tree is dirty, so we should style the document.
    MOZ_ASSERT(aChild == aChild->OwnerDoc()->GetRootElement());
    StyleSet()->StyleDocument();
    return;
  }

  if (!aContainer->HasServoData()) {
    // This can happen with display:none. Bug 1297249 tracks more investigation
    // and assertions here.
    return;
  }

  // Style the new subtree because we will most likely need it during subsequent
  // frame construction. Bug 1298281 tracks deferring this work in the lazy
  // frame construction case.
  StyleSet()->StyleNewSubtree(aChild);

  RestyleForInsertOrChange(aContainer, aChild);
}

void
ServoRestyleManager::RestyleForAppend(nsIContent* aContainer,
                                      nsIContent* aFirstNewContent)
{
  //
  // XXXbholley: We need the Gecko logic here to correctly restyle for things
  // like :empty and positional selectors (though we may not need to post
  // restyle events as agressively as the Gecko path does).
  //
  // Bug 1297899 tracks this work.
  //
}

void
ServoRestyleManager::ContentAppended(nsIContent* aContainer,
                                     nsIContent* aFirstNewContent)
{
  if (!aContainer->HasServoData()) {
    // This can happen with display:none. Bug 1297249 tracks more investigation
    // and assertions here.
    return;
  }

  // Style the new subtree because we will most likely need it during subsequent
  // frame construction. Bug 1298281 tracks deferring this work in the lazy
  // frame construction case.
  if (aFirstNewContent->GetNextSibling()) {
    StyleSet()->StyleNewChildren(aContainer);
  } else {
    StyleSet()->StyleNewSubtree(aFirstNewContent);
  }

  RestyleForAppend(aContainer, aFirstNewContent);
}

void
ServoRestyleManager::ContentRemoved(nsINode* aContainer,
                                    nsIContent* aOldChild,
                                    nsIContent* aFollowingSibling)
{
  NS_WARNING("stylo: ServoRestyleManager::ContentRemoved not implemented");
}

nsresult
ServoRestyleManager::ContentStateChanged(nsIContent* aContent,
                                         EventStates aChangedBits)
{
  if (!aContent->IsElement()) {
    return NS_OK;
  }

  Element* aElement = aContent->AsElement();
  nsChangeHint changeHint;
  nsRestyleHint restyleHint;

  // NOTE: restyleHint here is effectively always 0, since that's what
  // ServoStyleSet::HasStateDependentStyle returns. Servo computes on
  // ProcessPendingRestyles using the ElementSnapshot, but in theory could
  // compute it sequentially easily.
  //
  // Determine what's the best way to do it, and how much work do we save
  // processing the restyle hint early (i.e., computing the style hint here
  // sequentially, potentially saving the snapshot), vs lazily (snapshot
  // approach).
  //
  // If we take the sequential approach we need to specialize Servo's restyle
  // hints system a bit more, and mesure whether we save something storing the
  // restyle hint in the table and deferring the dirtiness setting until
  // ProcessPendingRestyles (that's a requirement if we store snapshots though),
  // vs processing the restyle hint in-place, dirtying the nodes on
  // PostRestyleEvent.
  //
  // If we definitely take the snapshot approach, we should take rid of
  // HasStateDependentStyle, etc (though right now they're no-ops).
  ContentStateChangedInternal(aElement, aChangedBits, &changeHint,
                              &restyleHint);

  EventStates previousState = aElement->StyleState() ^ aChangedBits;
  ServoElementSnapshot* snapshot = Servo_SnapshotForElement(aElement);
  snapshot->AddState(previousState);

  PostRestyleEvent(aElement, restyleHint, changeHint);
  return NS_OK;
}

void
ServoRestyleManager::AttributeWillChange(Element* aElement,
                                         int32_t aNameSpaceID,
                                         nsIAtom* aAttribute, int32_t aModType,
                                         const nsAttrValue* aNewValue)
{
  ServoElementSnapshot* snapshot = Servo_SnapshotForElement(aElement);
  snapshot->AddAttrs(aElement);
}

nsresult
ServoRestyleManager::ReparentStyleContext(nsIFrame* aFrame)
{
  NS_WARNING("stylo: ServoRestyleManager::ReparentStyleContext not implemented");
  return NS_OK;
}

} // namespace mozilla
