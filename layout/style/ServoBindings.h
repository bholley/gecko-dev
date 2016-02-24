/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_ServoBindings_h
#define mozilla_ServoBindings_h

#include <stdint.h>

/*
 * API for Servo to access Gecko data structures. This file must compile as valid
 * C code in order for the binding generator to parse it.
 *
 * Functions beginning with Gecko_ are implemented in Gecko and invoked from Servo.
 * Functions beginning with Servo_ are implemented in Servo and invoked from Gecko.
 */

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
struct ServoStyleSetData;
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
struct ServoStyleSetData;
typedef struct ServoStyleSetData ServoStyleSetData;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// DOM Traversal.
uint32_t Gecko_ChildrenCount(RawGeckoNode* node);
int Gecko_NodeIsElement(RawGeckoNode* node);
RawGeckoNode* Gecko_GetParentNode(RawGeckoNode* node);
RawGeckoNode* Gecko_GetFirstChild(RawGeckoNode* node);
RawGeckoNode* Gecko_GetLastChild(RawGeckoNode* node);
RawGeckoNode* Gecko_GetPrevSibling(RawGeckoNode* node);
RawGeckoNode* Gecko_GetNextSibling(RawGeckoNode* node);
RawGeckoElement* Gecko_GetParentElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetFirstChildElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetLastChildElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetPrevSiblingElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetNextSiblingElement(RawGeckoElement* element);
RawGeckoElement* Gecko_GetDocumentElement(RawGeckoDocument* document);

// Selector Matching.
uint8_t Gecko_ElementState(RawGeckoElement* element);
int Gecko_IsHTMLElementInHTMLDocument(RawGeckoElement* element);
int Gecko_IsLink(RawGeckoElement* element);
int Gecko_IsTextNode(RawGeckoNode* node);
int Gecko_IsVisitedLink(RawGeckoElement* element);
int Gecko_IsUnvisitedLink(RawGeckoElement* element);
int Gecko_IsRootElement(RawGeckoElement* element);

// Node data.
ServoNodeData* Gecko_GetNodeData(RawGeckoNode* node);
void Gecko_SetNodeData(RawGeckoNode* node, ServoNodeData* data);
void Servo_DropNodeData(ServoNodeData* data);

// Stylesheet management.
ServoArcStyleSheet* Servo_StylesheetFromUTF8Bytes(const uint8_t* bytes, uint32_t length);
void Servo_DropStylesheet(ServoArcStyleSheet* sheet);
void Servo_AppendStyleSheet(ServoArcStyleSheet* sheet, ServoStyleSetData* data);
void Servo_PrependStyleSheet(ServoArcStyleSheet* sheet, ServoStyleSetData* data);
void Servo_RemoveStyleSheet(ServoArcStyleSheet* sheet, ServoStyleSetData* data);
int Servo_StyleSheetHasRules(ServoArcStyleSheet* sheet);
ServoStyleSetData* Servo_InitStyleSetData();
void Servo_DropStyleSetData(ServoStyleSetData* data);

// Temporary string-based APIs for attributes and element names.
//
// These won't ever land in mozilla-central, as we'll replace them
// with atom-based APIs instead.
const char* Gecko_GetAttrAsUTF8(RawGeckoElement* element, const uint8_t* ns,
                                const uint8_t* name, uint32_t* length);
const uint16_t* Gecko_LocalName(RawGeckoElement* element, uint32_t* length);
const uint16_t* Gecko_Namespace(RawGeckoElement* element, uint32_t* length);

// Servo API.
void Servo_RestyleDocument(RawGeckoDocument* doc, ServoStyleSetData* data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // mozilla_ServoBindings_h
