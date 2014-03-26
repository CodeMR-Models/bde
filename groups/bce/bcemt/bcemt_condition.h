// bcemt_condition.h                                                  -*-C++-*-
#ifndef INCLUDED_BCEMT_CONDITION
#define INCLUDED_BCEMT_CONDITION

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a portable, efficient condition variable.
//
//@CLASSES:
//  bcemt_Condition: portable intra-process signaling mechanism
//
//@SEE_ALSO: bcemt_mutex
//
//@DESCRIPTION: The 'bcemt_Condition' class provided by this component
// implements the concept of a *condition* *variable*, enabling multiple
// threads to communicate information about the state of shared data.  A
// condition variable is a signaling mechanism associated with a mutex, which
// in turn protects a data invariant.  A condition variable enables threads to
// wait for a predicate (i.e., logical expression) to become true, and to
// communicate to other threads that the predicate might be true.
//
// One or more threads can wait efficiently on a condition variable, either
// indefinitely or until some absolute time, by invoking one of the following
// methods of 'bcemt_Condition':
//..
//  int wait(bcemt_Mutex *mutex);
//  int timedWait(bcemt_Mutex *mutex, const bdet_TimeInterval& absoluteTime);
//..
// The caller must lock the mutex before invoking these functions.  The
// 'bcemt_Condition' atomically releases the lock and waits, thereby preventing
// other threads from changing the predicate after the lock is released, but
// before the thread begins to wait.  The 'bcemt' package guarantees that this
// lock will be reacquired before returning from a call to the 'wait' and
// 'timedWait' methods, unless an error occurs.
//
// When invoking the 'timedWait' method, clients must specify a timeout after
// which the call will return even if the condition is not signaled.  The
// timeout is expressed as a 'bdet_TimeInterval' object that holds the absolute
// time according to the clock type the 'bcemt_Condition' object is constructed
// with (the default clock is 'bdetu_SystemClockType::e_REALTIME').  Clients
// should use the 'bdetu_SystemTime::now(clockType)' utility method to obtain
// the current time.
//
// Other threads can indicate that the predicate is true by signaling or
// broadcasting the same 'bcemt_Condition' object.  A broadcast wakes up all
// waiting threads, whereas a signal wakes only one thread.  The client has no
// control over which thread will be signaled if multiple threads are waiting:
//..
//  void signal();
//  void broadcast();
//..
// A thread waiting on a condition variable may be signaled (i.e., the thread
// may wake up without an error), but find that the predicate is still false.
// This situation can arise for a few reasons: spurious wakeups produced by the
// operating system, intercepted wakeups, and loose predicates.  Therefore, a
// waiting thread should always check the predicate *after* (as well as before)
// the call to the 'wait' function.
//
///Supported Clock-Types
///-------------------------
// The component 'bdetu_SystemClockType' supplies the enumeration indicating
// the system clock on which timeouts supplied to other methods should be
// based.  If the clock type indicated at construction is
// 'bdetu_SystemClockType::e_REALTIME', the timeout should be expressed as an
// absolute offset since 00:00:00 UTC, January 1, 1970 (which matches the epoch
// used in 'bdetu_SystemTime::now(bdetu_SystemClockType::e_REALTIME)'.  If the
// clock type indicated at construction is
// 'bdetu_SystemClockType::e_MONOTONIC', the timeout should be expressed as an
// absolute offset since the epoch of this clock (which matches the epoch used
// in 'bdetu_SystemTime::now(bdetu_SystemClockType::e_MONOTONIC)'.
//
///Usage
///-----
// Suppose we have a 'bcemt_Condition' object, 'condition', and a boolean
// predicate associated with 'condition' (represented here as a free function
// that returns a 'bool' value):
//..
//  bcemt_Condition condition;
//
//  bool predicate()
//      // Return 'true' if the invariant holds for 'condition', and 'false'
//      // otherwise.
//  {
//      // ...
//  }
//..
// The following usage pattern should always be followed:
//..
//    // ...
//
//    mutex.lock();
//    while (false == predicate()) {
//        condition.wait(&mutex);
//    }
//
//    // Modify shared resources and adjust the predicate here.
//
//    mutex.unlock();
//
//    // ...
//..
// The usage pattern for a timed wait is similar, but has extra branches to
// handle a timeout:
//..
//    // ...
//
//    enum { TIMED_OUT = -1 };
//    bdet_TimeInterval timeout = bdetu_SystemTime::now();
//
//    // Advance 'timeout' to some delta into the future here.
//
//    mutex.lock();
//    while (false == predicate()) {
//        const int status = condition.timedWait(&mutex, timeout);
//        if (TIMED_OUT == status) {
//            break;
//        }
//    }
//
//    if (false == predicate()) {
//        // The wait timed out and 'predicate' returned 'false'.  Perform
//        // timeout logic here.
//
//        // ...
//    }
//    else {
//        // The condition variable was either signaled or timed out and
//        // 'predicate' returned 'true'.  Modify shared resources and adjust
//        // predicate here.
//
//        // ...
//    }
//    mutex.unlock();
//
//    // ...
//..

#ifndef INCLUDED_BCESCM_VERSION
#include <bcescm_version.h>
#endif

#ifndef INCLUDED_BCEMT_CONDITIONIMPL_PTHREAD
#include <bcemt_conditionimpl_pthread.h>
#endif

#ifndef INCLUDED_BCEMT_CONDITIONIMPL_WIN32
#include <bcemt_conditionimpl_win32.h>
#endif

#ifndef INCLUDED_BCES_PLATFORM
#include <bces_platform.h>
#endif

#ifndef INCLUDED_BDET_TIMEINTERVAL
#include <bdet_timeinterval.h>
#endif

#ifndef INCLUDED_BDETU_SYSTEMCLOCKTYPE
#include <bdetu_systemclocktype.h>
#endif

namespace BloombergLP {

template <typename THREAD_POLICY>
class bcemt_ConditionImpl;

class bcemt_Mutex;

                           // =====================
                           // class btemt_Condition
                           // =====================

class bcemt_Condition {
    // This 'class' implements a portable inter-thread signaling primitive.

    // DATA
    bcemt_ConditionImpl<bces_Platform::ThreadPolicy>
                                    d_imp;  // platform-specific implementation

    // NOT IMPLEMENTED
    bcemt_Condition(const bcemt_Condition&);
    bcemt_Condition& operator=(const bcemt_Condition&);

  public:
    // CREATORS
    explicit
    bcemt_Condition(bdetu_SystemClockType::Enum clockType
                                          = bdetu_SystemClockType::e_REALTIME);
        // Create a condition variable object.  Optionally specify a
        // 'clockType' indicating the type of the system clock against which
        // the 'bdet_TimeInterval' timeouts passed to the 'timedWait' method
        // are to be interpreted.  If 'clockType' is not specified then the
        // realtime system clock is used.

    ~bcemt_Condition();
        // Destroy this condition variable object.

    // MANIPULATORS
    void broadcast();
        // Signal this condition variable object by waking up *all* threads
        // that are currently waiting on this condition.  If there are no
        // threads waiting on this condition, this method has no effect.

    void signal();
        // Signal this condition variable object by waking up a single thread
        // that is currently waiting on this condition.  If there are no
        // threads waiting on this condition, this method has no effect.

    int timedWait(bcemt_Mutex *mutex, const bdet_TimeInterval& absoluteTime);
        // Atomically unlock the specified 'mutex' and suspend execution of the
        // current thread until this condition object is "signaled" (i.e., one
        // of the 'signal' or 'broadcast' methods is invoked on this object) or
        // until the specified 'timeout', then re-acquire a lock on the
        // 'mutex'.  The 'timeout' is an absolute time represented as an
        // interval from some epoch, which is detemined by the clock indicated
        // at construction (see {Supported Clock-Types} in the component
        // documentation).  Return 0 on success, -1 on timeout, and a non-zero
        // value different from -1 if an error occurs.  The behavior is
        // undefined unless 'mutex' is locked by the calling thread prior to
        // calling this method.  Note that 'mutex' remains locked by the
        // calling thread upon returning from this function with success or
        // timeout, but is *not* guaranteed to remain locked otherwise.  Also
        // note that spurious wakeups are rare but possible, i.e., this method
        // may succeed (return 0) and return control to the thread without the
        // condition object being signaled.

    int wait(bcemt_Mutex *mutex);
        // Atomically unlock the specified 'mutex' and suspend execution of the
        // current thread until this condition object is "signaled" (i.e.,
        // either 'signal' or 'broadcast' is invoked on this object in another
        // thread), then re-acquire a lock on the 'mutex'.  Return 0 on
        // success, and a non-zero value otherwise.  Spurious wakeups are rare
        // but possible; i.e., this method may succeed (return 0), and return
        // control to the thread without the condition object being signaled.
        // The behavior is undefined unless 'mutex' is locked by the calling
        // thread prior to calling this method.  Note that 'mutex' remains
        // locked by the calling thread upon successfully returning from this
        // function, but is *not* guaranteed to remain locked if an error
        // occurs.
};

// ============================================================================
//                        INLINE FUNCTION DEFINITIONS
// ============================================================================

                           // ---------------------
                           // class bcemt_Condition
                           // ---------------------

// CREATORS
inline
bcemt_Condition::bcemt_Condition(bdetu_SystemClockType::Enum clockType)
: d_imp(clockType)
{
}

inline
bcemt_Condition::~bcemt_Condition()
{
}

// MANIPULATORS
inline
void bcemt_Condition::broadcast()
{
    d_imp.broadcast();
}

inline
void bcemt_Condition::signal()
{
    d_imp.signal();
}

inline
int bcemt_Condition::timedWait(bcemt_Mutex              *mutex,
                               const bdet_TimeInterval&  absoluteTime)
{
    return d_imp.timedWait(mutex, absoluteTime);
}

inline
int bcemt_Condition::wait(bcemt_Mutex *mutex)
{
    return d_imp.wait(mutex);
}

}  // close namespace BloombergLP

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2014
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
