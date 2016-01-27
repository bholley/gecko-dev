/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_servo_ExternalAPI_h
#define mozilla_servo_ExternalAPI_h

#include <stdint.h>

/*
 * API for Servo to access Gecko data structures. This file must compile as valid
 * C code in order for the binding generator to parse it.
 */

// It's unfortunate that we need to duplicate this from nsIDOMNode.idl. We
// statically assert their equivalency in the cpp file.
enum NodeType {
  ELEMENT_NODE = 1U,
  ATTRIBUTE_NODE = 2U,
  TEXT_NODE = 3U,
  CDATA_SECTION_NODE = 4U,
  ENTITY_REFERENCE_NODE = 5U,
  ENTITY_NODE = 6U,
  PROCESSING_INSTRUCTION_NODE = 7U,
  COMMENT_NODE = 8U,
  DOCUMENT_NODE = 9U,
  DOCUMENT_TYPE_NODE = 10U,
  DOCUMENT_FRAGMENT_NODE = 11U,
  NOTATION_NODE = 12U
};

#ifdef __cplusplus
class nsINode;
typedef nsINode RawGeckoNode;
namespace mozilla { namespace dom { class Element; } }
using mozilla::dom::Element;
typedef mozilla::dom::Element RawGeckoElement;
class nsIDocument;
typedef nsIDocument RawGeckoDocument;
struct ServoNodeData;
struct ServoArcStyleSheet;
#else
struct RawGeckoNode;
typedef struct RawGeckoNode RawGeckoNode;
struct RawGeckoElement;
typedef struct RawGeckoElement RawGeckoElement;
struct RawGeckoDocument;
typedef struct RawGeckoDocument RawGeckoDocument;
struct ServoNodeData;
typedef struct ServoNodeData ServoNodeData;
struct ServoArcStyleSheet;
typedef struct ServoArcStyleSheet ServoArcStyleSheet;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// These functions are implemented in Gecko.
uint8_t Gecko_ElementState(RawGeckoElement* element);
const char* Gecko_GetAttrAsUTF8(RawGeckoElement* element, const uint8_t* ns, const uint8_t* name, uint32_t* length);
uint32_t Gecko_ChildrenCount(RawGeckoNode* node);
RawGeckoElement* Gecko_GetDocumentElement(RawGeckoDocument* document);

RawGeckoNode* Gecko_GetFirstChild(RawGeckoNode* node);
RawGeckoNode* Gecko_GetLastChild(RawGeckoNode* node);
RawGeckoNode* Gecko_GetPrevSibling(RawGeckoNode* node);
RawGeckoNode* Gecko_GetNextSibling(RawGeckoNode* node);

RawGeckoElement* Gecko_GetParentElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetFirstChildElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetLastChildElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetPrevSiblingElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetNextSiblingElement(RawGeckoElement* element);

ServoNodeData* Gecko_GetNodeData(RawGeckoNode* node);
RawGeckoNode* Gecko_GetParentNode(RawGeckoNode* node);
const uint16_t* Gecko_LocalName(RawGeckoElement* element, uint32_t* length);
int Gecko_IsHTMLElementInHTMLDocument(RawGeckoElement* element);
int Gecko_IsLink(RawGeckoElement* element);
int Gecko_IsTextNode(RawGeckoNode* node);
int Gecko_IsVisitedLink(RawGeckoElement* element);
int Gecko_IsUnvisitedLink(RawGeckoElement* element);
int Gecko_IsRootElement(RawGeckoElement* element);
const uint16_t* Gecko_Namespace(RawGeckoElement* element, uint32_t* length);
int Gecko_NodeIsElement(RawGeckoNode* node);
void Gecko_SetNodeData(RawGeckoNode* node, ServoNodeData* data);

// These functions are implemented in Servo.
void Servo_RestyleDocument(RawGeckoDocument* aDoc);
void Servo_DropNodeData(ServoNodeData* data);
ServoArcStyleSheet* Servo_StylesheetFromUTF8Bytes(const uint8_t* bytes, uint32_t length);
void Servo_DropStylesheet(ServoArcStyleSheet* sheet);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // mozilla_servo_ExternalAPI_h
