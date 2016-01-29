/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=2 sw=2 et tw=78:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* smart pointer for strong references to objects through pointer-like
 * "handle" objects */

#include <algorithm>
#include "mozilla/Assertions.h"

#ifndef mozilla_HandleRefPtr_h
#define mozilla_HandleRefPtr_h

namespace mozilla {

/**
 * A class for holding strong references to handle-managed objects.
 *
 * This is intended for use with objects like StyleSheetHandle, where
 * the handle type is not a pointer but which can still have ->AddRef()
 * and ->Release() called on it.
 */
template<typename T>
class HandleRefPtr
{
public:
  HandleRefPtr() {}

  HandleRefPtr(HandleRefPtr<T>& aRhs)
  {
    assign(aRhs.mHandle);
  }

  HandleRefPtr(HandleRefPtr<T>&& aRhs)
  {
    mHandle = aRhs.mHandle;
    aRhs.mHandle = nullptr;
  }

  MOZ_IMPLICIT HandleRefPtr(T aRhs)
  {
    assign(aRhs);
  }

  HandleRefPtr<T>& operator=(HandleRefPtr<T>& aRhs)
  {
    assign(aRhs.mHandle);
    return *this;
  }

  HandleRefPtr<T>& operator=(T aRhs)
  {
    assign(aRhs);
    return *this;
  }

  ~HandleRefPtr() { assign(nullptr); }

  explicit operator bool() const { return !!mHandle; }
  bool operator!() const { return !mHandle; }

  operator T() const { return mHandle; }
  T operator->() const { return mHandle; }

  bool operator==(const HandleRefPtr<T>& aOther) const
  {
    return mHandle == aOther.mHandle;
  }

  void swap(HandleRefPtr<T>& aOther)
  {
    std::swap(mHandle, aOther.mHandle);
  }

private:
  void assign(T aPtr)
  {
    if (aPtr) {
      aPtr->AddRef();
    }
    if (mHandle) {
      mHandle->Release();
    }
    mHandle = aPtr;
  }

  T mHandle;
};

} // namespace mozilla

#endif // mozilla_HandleRefPtr_h
