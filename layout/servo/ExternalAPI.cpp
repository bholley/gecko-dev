/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/servo/ExternalAPI.h"

#include "nsContentUtils.h"
#include "nsIDOMNode.h"
#include "nsIDocument.h"
#include "nsINode.h"
#include "nsNameSpaceManager.h"
#include "nsString.h"

#include "mozilla/EventStates.h"
#include "mozilla/dom/Element.h"

uint8_t
Gecko_ElementState(RawGeckoElement* aElement)
{
  return aElement->State().GetInternalValue() & ((1 << (NS_EVENT_STATE_HIGHEST_SERVO_BIT + 1)) - 1);
}

const char*
Gecko_GetAttrAsUTF8(RawGeckoElement* aElement, const uint8_t* aNS, const uint8_t* aName, uint32_t* aLength)
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
Gecko_ChildrenCount(RawGeckoNode* aNode)
{
  return aNode->GetChildCount();
}

RawGeckoElement*
Gecko_GetDocumentElement(RawGeckoDocument* aDoc)
{
  return aDoc->GetDocumentElement();
}

RawGeckoNode*
Gecko_GetFirstChild(RawGeckoNode* aNode)
{
  return aNode->GetFirstChild();
}

RawGeckoNode*
Gecko_GetLastChild(RawGeckoNode* aNode)
{
  return aNode->GetLastChild();
}

RawGeckoNode*
Gecko_GetPrevSibling(RawGeckoNode* aNode)
{
  return aNode->GetPreviousSibling();
}

RawGeckoNode*
Gecko_GetNextSibling(RawGeckoNode* aNode)
{
  return aNode->GetNextSibling();
}

ServoNodeData*
Gecko_GetNodeData(RawGeckoNode* aNode)
{
  return aNode->mServoNodeData;
}


RawGeckoNode*
Gecko_GetParentNode(RawGeckoNode* aNode)
{
  return aNode->GetParentNode();
}

const uint16_t*
Gecko_LocalName(RawGeckoElement* aElement, uint32_t* aLength)
{
  static_assert(sizeof(char16_t) == sizeof(uint16_t), "Servo doesn't know what a char16_t is");
  *aLength = aElement->LocalName().Length();
  return reinterpret_cast<const uint16_t*>(aElement->LocalName().get());
}

int
Gecko_IsHTMLElementInHTMLDocument(RawGeckoElement* aElement)
{
  return aElement->IsHTMLElement() && aElement->OwnerDoc()->IsHTMLDocument();
}

int
Gecko_IsLink(RawGeckoElement* aElement)
{
  return aElement->State().HasAtLeastOneOfStates(NS_EVENT_STATE_VISITED | NS_EVENT_STATE_UNVISITED);
}

int Gecko_IsTextNode(RawGeckoNode* aNode)
{
  return aNode->NodeInfo()->NodeType() == nsIDOMNode::TEXT_NODE;
}

int
Gecko_IsVisitedLink(RawGeckoElement* aElement)
{
  return aElement->State().HasState(NS_EVENT_STATE_VISITED);
}


int
Gecko_IsUnvisitedLink(RawGeckoElement* aElement)
{
  return aElement->State().HasState(NS_EVENT_STATE_UNVISITED);
}

const uint16_t*
Gecko_Namespace(RawGeckoElement* aElement, uint32_t* aLength)
{
  static_assert(sizeof(char16_t) == sizeof(uint16_t), "Servo doesn't know what a char16_t is");
  nsNameSpaceManager* manager = nsContentUtils::NameSpaceManager();
  const nsString& str = manager->NameSpaceURIRef(aElement->NodeInfo()->NamespaceID());
  *aLength = str.Length();
  return reinterpret_cast<const uint16_t*>(str.get());
}

int
Gecko_NodeIsElement(RawGeckoNode* aNode)
{
  return aNode->IsElement();
}

void
Gecko_SetNodeData(RawGeckoNode* aNode, ServoNodeData* aData)
{
  MOZ_ASSERT(!aNode->mServoNodeData);
  aNode->mServoNodeData = aData;
}

static_assert((unsigned) ELEMENT_NODE == nsIDOMNode::ELEMENT_NODE, "Mismatched enum");
static_assert((unsigned) ATTRIBUTE_NODE == nsIDOMNode::ATTRIBUTE_NODE, "Mismatched enum");
static_assert((unsigned) TEXT_NODE == nsIDOMNode::TEXT_NODE, "Mismatched enum");
static_assert((unsigned) CDATA_SECTION_NODE == nsIDOMNode::CDATA_SECTION_NODE, "Mismatched enum");
static_assert((unsigned) ENTITY_REFERENCE_NODE == nsIDOMNode::ENTITY_REFERENCE_NODE, "Mismatched enum");
static_assert((unsigned) ENTITY_NODE == nsIDOMNode::ENTITY_NODE, "Mismatched enum");
static_assert((unsigned) PROCESSING_INSTRUCTION_NODE == nsIDOMNode::PROCESSING_INSTRUCTION_NODE, "Mismatched enum");
static_assert((unsigned) COMMENT_NODE == nsIDOMNode::COMMENT_NODE, "Mismatched enum");
static_assert((unsigned) DOCUMENT_NODE == nsIDOMNode::DOCUMENT_NODE, "Mismatched enum");
static_assert((unsigned) DOCUMENT_TYPE_NODE == nsIDOMNode::DOCUMENT_TYPE_NODE, "Mismatched enum");
static_assert((unsigned) DOCUMENT_FRAGMENT_NODE == nsIDOMNode::DOCUMENT_FRAGMENT_NODE, "Mismatched enum");
static_assert((unsigned) NOTATION_NODE == nsIDOMNode::NOTATION_NODE, "Mismatched enum");

