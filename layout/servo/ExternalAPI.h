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
typedef nsINode GeckoNode;
namespace mozilla { namespace dom { class Element; } }
using mozilla::dom::Element;
typedef mozilla::dom::Element GeckoElement;
class nsIDocument;
typedef nsIDocument GeckoDocument;
struct ServoLayoutData;
#else
struct GeckoNode;
typedef struct GeckoNode GeckoNode;
struct GeckoElement;
typedef struct GeckoElement GeckoElement;
struct GeckoDocument;
typedef struct GeckoDocument GeckoDocument;
struct ServoLayoutData;
typedef struct ServoLayoutData ServoLayoutData;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// These functions are implemented in Gecko.
uint8_t Gecko_ElementState(GeckoElement* element);
const char* Gecko_GetAttrAsUTF8(GeckoElement* element, const uint8_t* ns, const uint8_t* name, uint32_t* length);
uint32_t Gecko_ChildrenCount(GeckoNode* node);
GeckoElement* Gecko_GetDocumentElement(GeckoDocument* document);

GeckoNode* Gecko_GetFirstChild(GeckoNode* node);
GeckoNode* Gecko_GetLastChild(GeckoNode* node);
GeckoNode* Gecko_GetPrevSibling(GeckoNode* node);
GeckoNode* Gecko_GetNextSibling(GeckoNode* node);

ServoLayoutData* Gecko_GetLayoutData(GeckoNode* node);
GeckoNode* Gecko_GetParentNode(GeckoNode* node);
const uint16_t* Gecko_LocalName(GeckoElement* element, uint32_t* length);
int Gecko_IsHTMLElementInHTMLDocument(GeckoElement* element);
int Gecko_IsLink(GeckoElement* element);
int Gecko_IsTextNode(GeckoNode* node);
int Gecko_IsVisitedLink(GeckoElement* element);
int Gecko_IsUnvisitedLink(GeckoElement* element);
const uint16_t* Gecko_Namespace(GeckoElement* element, uint32_t* length);
int Gecko_NodeIsElement(GeckoNode* node);
void Gecko_SetLayoutData(GeckoNode* node, ServoLayoutData* data);

// These functions are implemented in Servo.
void Servo_RestyleDocument(GeckoDocument* aDoc);
void Servo_DropLayoutData(ServoLayoutData* data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // mozilla_servo_ExternalAPI_h
