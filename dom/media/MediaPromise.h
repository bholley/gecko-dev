/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(MediaPromise_h_)
#define MediaPromise_h_

#include "prlog.h"

#include "mozilla/Maybe.h"
#include "mozilla/Mutex.h"

#include "MediaTaskQueue.h"
#include "nsIEventTarget.h"

namespace mozilla {

extern PRLogModuleInfo* gMediaPromiseLog;
void EnsureMediaPromiseLog();

#define PROMISE_LOG(x, ...) \
  MOZ_ASSERT(gMediaPromiseLog); \
  PR_LOG(gMediaPromiseLog, PR_LOG_DEBUG, (x, ##__VA_ARGS__))

/*
 * A promise manages an asynchronous request that may or may not be able to be
 * fulfilled immediately. When an API returns a promise, the consumer may attach
 * callbacks to be invoked (asynchronously, on a specified thread) when the
 * request is either completed (resolved) or cannot be completed (rejected).
 *
 * By default, resolve and reject callbacks are always invoked on the same thread
 * where Then() was invoked.
 */
template<typename ResolveValueT, typename RejectValueT>
class MediaPromise
{
public:
  typedef ResolveValueT ResolveValueType;
  typedef RejectValueT RejectValueType;

  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(MediaPromise)
  MediaPromise(const char* aCreationSite)
    : mCreationSite(aCreationSite)
    , mMutex("MediaPromise Mutex")
  {
    MOZ_COUNT_CTOR(MediaPromise);
    PROMISE_LOG("%s creating MediaPromise (%p)", mCreationSite, this);
  }

protected:

  /*
   * A ThenValue tracks a single consumer waiting on the promise. When a consumer
   * invokes promise->Then(...), a ThenValue is created. Once the Promise is
   * resolved or rejected, a ThenValueRunnable is dispatched, which invokes the
   * resolve/reject method and then deletes the ThenValue.
   */
  class ThenValueBase
  {
  public:
    class ThenValueRunnable : public nsRunnable
    {
    public:
      ThenValueRunnable(ThenValueBase* aThenValue, ResolveValueType aResolveValue)
        : mThenValue(aThenValue)
      {
        MOZ_COUNT_CTOR(ThenValueRunnable);
        mResolveValue.emplace(aResolveValue);
      }

      ThenValueRunnable(ThenValueBase* aThenValue, RejectValueType aRejectValue)
        : mThenValue(aThenValue)
      {
        MOZ_COUNT_CTOR(ThenValueRunnable);
        mRejectValue.emplace(aRejectValue);
      }

      ~ThenValueRunnable()
      {
        MOZ_COUNT_DTOR(ThenValueRunnable);
        MOZ_ASSERT(!mThenValue);
      }

      NS_IMETHODIMP Run()
      {
        PROMISE_LOG("ThenValueRunnable::Run() [this=%p]", this);
        if (mResolveValue.isSome()) {
          mThenValue->DoResolve(mResolveValue.ref());
        } else {
          mThenValue->DoReject(mRejectValue.ref());
        }

        delete mThenValue;
        mThenValue = nullptr;
        return NS_OK;
      }

    private:
      ThenValueBase* mThenValue;
      Maybe<ResolveValueType> mResolveValue;
      Maybe<RejectValueType> mRejectValue;
    };

    ThenValueBase(const char* aCallSite) : mCallSite(aCallSite)
    {
      MOZ_COUNT_CTOR(ThenValueBase);
    }

    virtual void Dispatch(MediaPromise *aPromise) = 0;

  protected:
    // This may only be deleted by ThenValueRunnable::Run.
    virtual ~ThenValueBase() { MOZ_COUNT_DTOR(ThenValueBase); }

    virtual void DoResolve(ResolveValueType aResolveValue) = 0;
    virtual void DoReject(RejectValueType aRejectValue) = 0;

    const char* mCallSite;
  };

  template<typename TargetType, typename ThisType,
           typename ResolveMethodType, typename RejectMethodType>
  class ThenValue : public ThenValueBase
  {
  public:
    ThenValue(TargetType* aResponseTarget, ThisType* aThisVal,
              ResolveMethodType aResolveMethod, RejectMethodType aRejectMethod,
              const char* aCallSite)
      : ThenValueBase(aCallSite)
      , mResponseTarget(aResponseTarget)
      , mThisVal(aThisVal)
      , mResolveMethod(aResolveMethod)
      , mRejectMethod(aRejectMethod) {}

    void Dispatch(MediaPromise *aPromise) MOZ_OVERRIDE
    {
      aPromise->mMutex.AssertCurrentThreadOwns();
      MOZ_ASSERT(!aPromise->IsPending());
      bool resolved = aPromise->mResolveValue.isSome();
      auto runnable = resolved ? new (typename ThenValueBase::ThenValueRunnable)(this, aPromise->mResolveValue.ref())
                               : new (typename ThenValueBase::ThenValueRunnable)(this, aPromise->mRejectValue.ref());
      PROMISE_LOG("%s Then() call made from %s [Runnable=%p, Promise=%p, ThenValue=%p]",
                  resolved ? "Resolving" : "Rejecting", ThenValueBase::mCallSite,
                  runnable, aPromise, this);
      nsresult rv = DoDispatch(mResponseTarget, runnable);
      MOZ_ASSERT(NS_SUCCEEDED(rv));
    }

  protected:
    virtual void DoResolve(ResolveValueType aResolveValue)
    {
      ((*mThisVal).*mResolveMethod)(aResolveValue);
    }

    virtual void DoReject(RejectValueType aRejectValue)
    {
      ((*mThisVal).*mRejectMethod)(aRejectValue);
    }

    static nsresult DoDispatch(MediaTaskQueue* aTaskQueue, nsIRunnable* aRunnable)
    {
      return aTaskQueue->ForceDispatch(aRunnable);
    }

    static nsresult DoDispatch(nsIEventTarget* aEventTarget, nsIRunnable* aRunnable)
    {
      return aEventTarget->Dispatch(aRunnable, NS_DISPATCH_NORMAL);
    }

    virtual ~ThenValue() {}

  private:
    nsRefPtr<TargetType> mResponseTarget;
    nsRefPtr<ThisType> mThisVal;
    ResolveMethodType mResolveMethod;
    RejectMethodType mRejectMethod;
  };
public:

  template<typename TargetType, typename ThisType,
           typename ResolveMethodType, typename RejectMethodType>
  void Then(TargetType* aResponseTarget, ThisType* aThisVal,
            ResolveMethodType aResolveMethod, RejectMethodType aRejectMethod,
            const char* aCallSite)
  {
    MutexAutoLock lock(mMutex);
    ThenValueBase* thenValue = new ThenValue<TargetType, ThisType, ResolveMethodType,
                                             RejectMethodType>(aResponseTarget, aThisVal,
                                                               aResolveMethod, aRejectMethod,
                                                               aCallSite);
    PROMISE_LOG("%s invoking Then() [this=%p, thenValue=%p, aThisVal=%p, isPending=%d]",
                aCallSite, this, thenValue, aThisVal, (int) IsPending());
    if (!IsPending()) {
      thenValue->Dispatch(this);
    } else {
      mThenValues.AppendElement(thenValue);
    }
  }

  void Resolve(ResolveValueType aResolveValue, const char* aResolveSite)
  {
    MutexAutoLock lock(mMutex);
    MOZ_ASSERT(IsPending());
    PROMISE_LOG("%s resolving MediaPromise (%p created at %s)", aResolveSite, this, mCreationSite);
    mResolveValue.emplace(aResolveValue);
    DispatchAll();
  }

  void Reject(RejectValueType aRejectValue, const char* aRejectSite)
  {
    MutexAutoLock lock(mMutex);
    MOZ_ASSERT(IsPending());
    PROMISE_LOG("%s rejecting MediaPromise (%p created at %s)", aRejectSite, this, mCreationSite);
    mRejectValue.emplace(aRejectValue);
    DispatchAll();
  }

protected:
  bool IsPending() { return mResolveValue.isNothing() && mRejectValue.isNothing(); }
  void DispatchAll()
  {
    mMutex.AssertCurrentThreadOwns();
    for (size_t i = 0; i < mThenValues.Length(); ++i)
      mThenValues[i]->Dispatch(this);
    mThenValues.Clear();
  }

  ~MediaPromise()
  {
    MOZ_COUNT_DTOR(MediaPromise);
    PROMISE_LOG("MediaPromise::~MediaPromise [this=%p]", this);
  };

  const char* mCreationSite; // For logging
  Mutex mMutex;
  Maybe<ResolveValueType> mResolveValue;
  Maybe<RejectValueType> mRejectValue;
  nsTArray<ThenValueBase*> mThenValues;
};

/*
 * Base class for RAII manipulation of promise members.
 * Do not instantiate directly.
 */
template<typename PromiseType>
class MOZ_STACK_CLASS AutoManagePromiseMember
{
public:
  explicit AutoManagePromiseMember(nsRefPtr<PromiseType>* aPromiseMember,
                                   const char* aMethodName)
    : mPromiseMember(aPromiseMember)
    , mMethodName(aMethodName)
    , mResponded(false) {}

  ~AutoManagePromiseMember()
  {
    // If the promise was resolved in stack scope, we forget about it.
    // Otherwise, it needs to stick around until it gets resolved.
    if (mResponded) {
      *mPromiseMember = nullptr;
    }
  }

  void Resolve(typename PromiseType::ResolveValueType aResolveValue)
  {
    (*mPromiseMember)->Resolve(aResolveValue, mMethodName);
    mResponded = true;
  }

  void Reject(typename PromiseType::RejectValueType aRejectValue)
  {
    (*mPromiseMember)->Reject(aRejectValue, mMethodName);
    mResponded = true;
  }

protected:
  nsRefPtr<PromiseType>* mPromiseMember;
  const char* mMethodName; // For logging
  bool mResponded;
};

/*
 * RAII class for APIs that return a promise to consumers which may be resolved
 * at some later point in time. This class binds to a member variable which
 * holds onto the promise for the period of time in which it is pending. If the
 * promise is resolved or rejected while this class is on the stack, the member
 * variable is cleared.
 *
 * AutoReturnMediaPromise promise(&mMyPromiseMember);
 * if (foo) {
 *   promise->Resolve(x);
 * } else if (bar) {
 *   promise->Reject(y);
 * } else {
 *   // The promise will be resolved async.
 *   ...
 * }
 *
 * return promise;
 */
template<typename PromiseType>
class MOZ_STACK_CLASS AutoReturnMediaPromise
  : public AutoManagePromiseMember<PromiseType>
{
public:
  typedef AutoManagePromiseMember<PromiseType> Base;
  explicit AutoReturnMediaPromise(nsRefPtr<PromiseType>* aPromiseMember,
                                  const char* aMethodName)
    : Base(aPromiseMember, aMethodName)
  {
    if (!*Base::mPromiseMember) {
      *Base::mPromiseMember = new PromiseType(Base::mMethodName);
    }
  }

  operator nsRefPtr<PromiseType>()
  {
    return nsRefPtr<PromiseType>(*Base::mPromiseMember);
  }
};

/*
 * RAII class for methods which do not return a promise to consumers, but may
 * (or may not) resolve or reject an existing outstanding promise.
 *
 * Asserts that the outstanding promise exists.
 */
template<typename PromiseType>
class MOZ_STACK_CLASS AutoServiceMediaPromise
  : public AutoManagePromiseMember<PromiseType>
{
public:
  typedef AutoManagePromiseMember<PromiseType> Base;
  explicit AutoServiceMediaPromise(nsRefPtr<PromiseType>* aPromiseMember,
                                   const char* aMethodName)
    : Base(aPromiseMember, aMethodName)
  {
    MOZ_ASSERT(*Base::mPromiseMember);
  }
};

#undef PROMISE_LOG

} // namespace mozilla

#endif
