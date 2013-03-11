// bcemt_timedsemaphoreimpl_win32.cpp                                 -*-C++-*-
#include <bcemt_timedsemaphoreimpl_win32.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bcemt_timedsemaphoreimpl_win32_cpp,"$Id$ $CSID$")

#ifdef BCES_PLATFORM_WIN32_THREADS

#include <bcemt_SaturatedTimeConversion.h>

#include <bcemt_lockguard.h>     // for testing only
#include <bcemt_mutex.h>         // for testing only
#include <bcemt_threadutil.h>    // for testing only

namespace BloombergLP {

           // ---------------------------------------------------
           // class bcemt_TimedSemaphoreImpl<Win32TimedSemaphore>
           // ---------------------------------------------------

// MANIPULATORS
int
bcemt_TimedSemaphoreImpl<bces_Platform::Win32TimedSemaphore>::timedWait(
                                              const bdet_TimeInterval& timeout)
{
    DWORD milliTimeout = 0;
    bdet_TimeInterval now = bdetu_SystemTime::now();
    if (timeout > now) {
        bdet_TimeInterval reltime = timeout - now;
        bcemt_SaturatedTimeConversion::toMillisec(&milliTimeout, reltime);
    }

    return WaitForSingleObject(d_handle, milliTimeout);
}

}  // close namespace BloombergLP

#endif  // BCES_PLATFORM_WIN32_THREADS

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2010
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
