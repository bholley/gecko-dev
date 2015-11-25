/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/servo/ExternalAPI.h"

#include "nsIDOMNode.h"
#include "nsIDocument.h"
#include "nsINode.h"
#include "nsNameSpaceManager.h"
#include "nsString.h"

#include "mozilla/EventStates.h"
#include "mozilla/dom/Element.h"

uint8_t
Gecko_ElementState(GeckoElement* aElement)
{
  return aElement->State().GetInternalValue() & ((1 << (NS_EVENT_STATE_HIGHEST_SERVO_BIT + 1)) - 1);
}

const char*
Gecko_GetAttrAsUTF8(GeckoElement* aElement, const uint8_t* aNS, const uint8_t* aName, uint32_t* aLength)
{
  MOZ_ASSERT(aNS[0] == '\0', "Can't handle namespaces yet");
  const nsAttrValue* val = aElement->GetParsedAttr(NS_ConvertUTF8toUTF16(nsDependentCString(reinterpret_cast<const char*>(aName))));
  if (!val) {
    return nullptr;
  }
  *aLength = val->UTF8String().Length();
  return val->UTF8String().get();
}

uint32_t
Gecko_ChildrenCount(GeckoNode* aNode)
{
  return aNode->GetChildCount();
}

GeckoElement*
Gecko_GetDocumentElement(GeckoDocument* aDoc)
{
  return aDoc->GetDocumentElement();
}

GeckoNode*
Gecko_GetFirstChild(GeckoNode* aNode)
{
  return aNode->GetFirstChild();
}

GeckoNode*
Gecko_GetLastChild(GeckoNode* aNode)
{
  return aNode->GetLastChild();
}

GeckoNode*
Gecko_GetPrevSibling(GeckoNode* aNode)
{
  return aNode->GetPreviousSibling();
}

GeckoNode*
Gecko_GetNextSibling(GeckoNode* aNode)
{
  return aNode->GetNextSibling();
}

ServoLayoutData*
Gecko_GetLayoutData(GeckoNode* aNode)
{
  return aNode->mServoLayoutData;
}


GeckoNode*
Gecko_GetParentNode(GeckoNode* aNode)
{
  return aNode->GetParentNode();
}

const uint16_t*
Gecko_LocalName(GeckoElement* aElement, uint32_t* aLength)
{
  static_assert(sizeof(char16_t) == sizeof(uint16_t), "Servo doesn't know what a char16_t is");
  *aLength = aElement->LocalName().Length();
  return reinterpret_cast<const uint16_t*>(aElement->LocalName().get());
}

int
Gecko_IsHTMLElementInHTMLDocument(GeckoElement* aElement)
{
  return aElement->IsHTMLElement() && aElement->OwnerDoc()->IsHTMLDocument();
}

int
Gecko_IsLink(GeckoElement* aElement)
{
  return aElement->State().HasAtLeastOneOfStates(NS_EVENT_STATE_VISITED | NS_EVENT_STATE_UNVISITED);
}

int Gecko_IsTextNode(GeckoNode* aNode)
{
  return aNode->NodeInfo()->NodeType() == nsIDOMNode::TEXT_NODE;
}

int
Gecko_IsVisitedLink(GeckoElement* aElement)
{
  return aElement->State().HasState(NS_EVENT_STATE_VISITED);
}


int
Gecko_IsUnvisitedLink(GeckoElement* aElement)
{
  return aElement->State().HasState(NS_EVENT_STATE_UNVISITED);
}

const uint16_t*
Gecko_Namespace(GeckoElement* aElement, uint32_t* aLength)
{
  static_assert(sizeof(char16_t) == sizeof(uint16_t), "Servo doesn't know what a char16_t is");
  nsNameSpaceManager* manager = nsContentUtils::NameSpaceManager();
  const nsString& str = manager->NameSpaceURIRef(aElement->NodeInfo()->NamespaceID());
  *aLength = str.Length();
  return reinterpret_cast<const uint16_t*>(str.get());
}

int
Gecko_NodeIsElement(GeckoNode* aNode)
{
  return aNode->IsElement();
}

void
Gecko_SetLayoutData(GeckoNode* aNode, ServoLayoutData* aData)
{
  MOZ_ASSERT(!aNode->mServoLayoutData);
  aNode->mServoLayoutData = aData;
}

static_assert(ELEMENT_NODE == nsIDOMNode::ELEMENT_NODE, "Mismatched enum");
static_assert(ATTRIBUTE_NODE == nsIDOMNode::ATTRIBUTE_NODE, "Mismatched enum");
static_assert(TEXT_NODE == nsIDOMNode::TEXT_NODE, "Mismatched enum");
static_assert(CDATA_SECTION_NODE == nsIDOMNode::CDATA_SECTION_NODE, "Mismatched enum");
static_assert(ENTITY_REFERENCE_NODE == nsIDOMNode::ENTITY_REFERENCE_NODE, "Mismatched enum");
static_assert(ENTITY_NODE == nsIDOMNode::ENTITY_NODE, "Mismatched enum");
static_assert(PROCESSING_INSTRUCTION_NODE == nsIDOMNode::PROCESSING_INSTRUCTION_NODE, "Mismatched enum");
static_assert(COMMENT_NODE == nsIDOMNode::COMMENT_NODE, "Mismatched enum");
static_assert(DOCUMENT_NODE == nsIDOMNode::DOCUMENT_NODE, "Mismatched enum");
static_assert(DOCUMENT_TYPE_NODE == nsIDOMNode::DOCUMENT_TYPE_NODE, "Mismatched enum");
static_assert(DOCUMENT_FRAGMENT_NODE == nsIDOMNode::DOCUMENT_FRAGMENT_NODE, "Mismatched enum");
static_assert(NOTATION_NODE == nsIDOMNode::NOTATION_NODE, "Mismatched enum");

