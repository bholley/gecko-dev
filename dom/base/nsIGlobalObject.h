/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsIGlobalObject_h__
#define nsIGlobalObject_h__

#include "nsISupports.h"
#include "js/TypeDecls.h"

#define NS_IGLOBALOBJECT_IID \
{ 0x1dc72dd3, 0xfdc3, 0x4393, \
{ 0x8d, 0x41, 0xd0, 0x85, 0xfc, 0x92, 0xd8, 0x7a } }

class nsIPrincipal;
class TraceCallbacks;

class nsIGlobalObject : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IGLOBALOBJECT_IID)

  virtual JSObject* GetGlobalJSObject() = 0;

  // In general, nsIGlobalObject implementations do not hold their global alive.
  // This method allows holders of an nsIGlobalObject pointer to explicitly
  // trace the JS global and thus keep it alive.
  virtual void TraceGlobalJSObject(const TraceCallbacks& aCallbacks, void* aClosure) = 0;

  // This method is not meant to be overridden.
  nsIPrincipal* PrincipalOrNull();
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIGlobalObject,
                              NS_IGLOBALOBJECT_IID)

#endif // nsIGlobalObject_h__
