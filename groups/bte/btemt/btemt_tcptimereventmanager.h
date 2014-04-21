// btemt_tcptimereventmanager.h                                       -*-C++-*-
#ifndef INCLUDED_BTEMT_TCPTIMEREVENTMANAGER
#define INCLUDED_BTEMT_TCPTIMEREVENTMANAGER

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a thread-enabled multiplexor of socket events and timers.
//
//@CLASSES:
//  btemt_TcpTimerEventManager: thread-enabled socket event/timer multiplexor
//
//@SEE_ALSO: btemt_channelpool bcep_threadpool bcemt_threadutil
//
//@AUTHOR: Herve Bronnimann (hbronnimann), Cheenu Srinivasan (csriniva)
//
//@DESCRIPTION: This component provides a concrete implementation of
// a thread-enabled multiplexer for socket events and timers.  An interested
// party can register a callback to be invoked whenever a particular event
// occurs on a particular socket, or when a timer expires.  Socket events
// have a permanent semantics (i.e., once registered, a callback is invoked
// until it is explicitly deregistered).  Timers have "one-time" semantics
// (i.e., a timer callback is invoked at most once).  Registering the same
// socket handle and the same event with different instances of this event
// manager may result in undefined behavior.  The timers are unstable with
// respect to registration; that is, if two timer callbacks are
// registered with the same time, they maybe invoked in an order different
// from the order of registration.  Socket events take priority over timers;
// if a socket event and a timer occur at the same time, a socket event
// callback is invoked first.
//
// All registered callbacks are invoked from an internal thread managed by
// 'this' event manager if its underlying thread is enabled (see below).  If
// the thread is disabled, the queries and registrations are processed (in a
// thread-safe fashion) but no callbacks are invoked.  On UNIX platforms all
// signals are disabled for this thread.
//
// An event manager can optimize its performance based on a hint provided
// at construction.  Generally speaking, a particular usage falls into two
// categories: socket events are registered: 1)frequently, and 2)infrequently.
// On some platforms, a significant performance improvement can be achieved
// if the registrations are infrequent.  For this situation, the appropriate
// hint should be provided to this event manager at construction for optimal
// performance.
//
///Thread safety
///-------------
// This event manager is *thread* *safe*, i.e., operations can be invoked
// simultaneously on a single instance of this event manager from multiple
// threads without undesirable side-effects.
//
// To the extent that this component uses local synchronization primitives
// internally to protect access to different pieces of private data, and to
// maximize the ability of different threads to operate concurrently on a
// single instance of this component, it is *thread* *enabled*.
//
// This component is not *async-safe*, i.e., one or more methods cannot be
// invoked safely from a signal handler.
//
///Performance
///-----------
// This component minimizes the number of system calls for the set of
// callbacks.  A significant performance improvement can be achieved if an
// appropriate registration frequency "hint" is provided.  Supplying a false
// hint (e.g., indicating infrequent registration when the reverse is true)
// may result in significant performance degradation.
//
// Given that T is the number of timers registered and S is the number of
// socket events registered, the following complexity is guaranteed by this
// component:
//..
//  NOTE: The following table is a place holder and needs to be rethought.
//  +========================================================================+
//  |        FUNCTION          | EXPECTED COMPLEXITY | WORST CASE COMPLEXITY |
//  +------------------------------------------------------------------------+
//  | registerTimer            |      O(log[T])      |      O(log[T])        |
//  +------------------------------------------------------------------------+
//  | deregisterTimer          |        O(T)         |        O(T)           |
//  +------------------------------------------------------------------------+
//  | registerSocketEvent      |        O(1)         |        O(S)           |
//  +------------------------------------------------------------------------+
//  | deregisterSocketEvent    |        O(1)         |        O(S)           |
//  +------------------------------------------------------------------------+
//  | isRegistered             |        O(1)         |        O(S)           |
//  +------------------------------------------------------------------------+
//  | deregisterSocket         |        O(1)         |        O(S)           |
//  +------------------------------------------------------------------------+
//  | deregisterAllSocketEvents|        O(S)         |        O(S)           |
//  +------------------------------------------------------------------------+
//  | deregisterAllTimers      |        O(T)         |        O(T)           |
//  +------------------------------------------------------------------------+
//  | deregisterAll            |       O(S+T)        |       O(S+T)          |
//  +------------------------------------------------------------------------+
//  | numEvents                |        O(1)         |        O(1)           |
//  +------------------------------------------------------------------------+
//  | numTimers                |        O(1)         |        O(1)           |
//  +------------------------------------------------------------------------+
//  | numSocketEvents          |        O(1)         |        O(S)           |
//  +========================================================================+
//..
///Usage
///-----
// In the following usage example, we demonstrate how to use timer
// functionality provided by 'btemt_TcpTimerEventManager'.  For simplicity,
// we simulate enqueueing a job to a queue periodically.  Let's assume that
// a job is represented by an integer.  First, let's implement a function that
// enqueues a job to a given queue.  This function will also re-register the
// "next" timer with the event manager since timers have "one-time" semantics.
//..
//  static void producer(bcec_Queue<int>            *workQueue,
//                       btemt_TcpTimerEventManager *manager,
//                       bdet_TimeInterval           nextTime)
//      // Enqueue a work item onto the specified 'workQueue' and register
//      // this function with the specified 'manager' to be invoked after
//      // the specified 'nextTime' absolute time interval.
//  {
//      assert(workQueue);
//      assert(manager);
//
//      enum { TIME_OFFSET = 5 };   // invoke timer every 5 seconds
//
//      int item = bdetu_SystemTime::now().nanoseconds() / 1000;
//      workQueue->pushBack(item);
//
//      bdet_TimeInterval nextNextTime(nextTime);
//      nextNextTime.addSeconds(TIME_OFFSET);
//      bdef_Function<void (*)()> callback(
//           bdef_BindUtil::bind(&producer, workQueue, manager, nextNextTime));
//
//      void *timerId = manager->registerTimer(nextTime, callback);
//      assert(timerId);
//  }
//..
// Second, implement the main function.  In main, create a work queue and
// a timer event manager, register with the event manager an initial timer
// with its callback and begin monitoring the work queue:
//..
//  int main() {
//      enum {
//         TIME_OFFSET         = 5 , // seconds
//         INITIAL_TIME_OFFSET = 1   // seconds
//      };
//      bcec_Queue<int> workQueue;
//      btemt_TcpTimerEventManager manager;
//      bdet_TimeInterval now = bdetu_SystemTime::now();
//      now.addSeconds(INITIAL_TIME_OFFSET);
//      bdet_TimeInterval nextTime(now);
//      nextTime.addSeconds(TIME_OFFSET);
//
//      bdef_Function<void (*)()> callback(
//         bdef_BindUtil::bind(&producer, &workQueue, &manager, nextNextTime));
//
//      void *timerId = manager.registerTimer(now, callback);
//      assert(timerId);
//      int rc = manager.enable();
//      assert(0 == rc);
//      while(1) {
//          // Monitor 'workQueue' here
//          // ...
//      }
//      return 0;
//  }
//..

#ifndef INCLUDED_BTESCM_VERSION
#include <btescm_version.h>
#endif

#ifndef INCLUDED_BTESO_SOCKETHANDLE
#include <bteso_sockethandle.h>
#endif

#ifndef INCLUDED_BTESO_EVENTTYPE
#include <bteso_eventtype.h>
#endif

#ifndef INCLUDED_BTESO_IPV4ADDRESS
#include <bteso_ipv4address.h>
#endif

#ifndef INCLUDED_BTESO_IOUTIL
#include <bteso_ioutil.h>
#endif

#ifndef INCLUDED_BTESO_SOCKETIMPUTIL
#include <bteso_socketimputil.h>
#endif

#ifndef INCLUDED_BTESO_SOCKETOPTUTIL
#include <bteso_socketoptutil.h>
#endif

#ifndef INCLUDED_BTESO_EVENTMANAGER
#include <bteso_eventmanager.h>
#endif

#ifndef INCLUDED_BTESO_TIMEREVENTMANAGER
#include <bteso_timereventmanager.h>
#endif

#ifndef INCLUDED_BTESO_TIMEMETRICS
#include <bteso_timemetrics.h>
#endif

#ifndef INCLUDED_BCEC_QUEUE
#include <bcec_queue.h>
#endif

#ifndef INCLUDED_BCEC_TIMEQUEUE
#include <bcec_timequeue.h>
#endif

#ifndef INCLUDED_BCEF_VFUNC0
#include <bcef_vfunc0.h>
#endif

#ifndef INCLUDED_BCEMA_POOL
#include <bcema_pool.h>
#endif

#ifndef INCLUDED_BCEMT_THREAD
#include <bcemt_thread.h>
#endif

#ifndef INCLUDED_BCEMT_RWMUTEX
#include <bcemt_rwmutex.h>
#endif

#ifndef INCLUDED_BCEMT_LOCKGUARD
#include <bcemt_lockguard.h>
#endif

#ifndef INCLUDED_BCEMT_READLOCKGUARD
#include <bcemt_readlockguard.h>
#endif

#ifndef INCLUDED_BCES_ATOMICTYPES
#include <bces_atomictypes.h>
#endif

#ifndef INCLUDED_BDET_TIMEINTERVAL
#include <bdet_timeinterval.h>
#endif

#ifndef INCLUDED_BDEF_FUNCTION
#include <bdef_function.h>
#endif

#ifndef INCLUDED_BDEMA_MANAGEDPTR
#include <bdema_managedptr.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSLS_ATOMIC
#include <bsls_atomic.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

namespace BloombergLP {

class btemt_TcpTimerEventManager_Request;

              // ===============================================
              // class btemt_TcpTimerEventManager_ControlChannel
              // ===============================================

class btemt_TcpTimerEventManager_ControlChannel {
    // This class is an implementation detail of 'btemt_TcpTimerEventManager'.
    // Do not use.

    // DATA
    bsls::AtomicInt d_fds[2];              // connected socket pair
    const char      d_byte;                // signal byte

    int             d_numServerReads;      // read operations
    int             d_numServerBytesRead;  // total number of bytes read
    bces_AtomicInt  d_numPendingRequests;  // number of pending requests

    bsls::AtomicInt d_numReinitsAttempted; // number of reinitializations
                                           // attempted

    // NOT IMPLEMENTED
    btemt_TcpTimerEventManager_ControlChannel(
                             const btemt_TcpTimerEventManager_ControlChannel&);
    btemt_TcpTimerEventManager_ControlChannel& operator=(
                             const btemt_TcpTimerEventManager_ControlChannel&);

    int initialize();
        // Initialize this control channel.  Return 0 on success and a
        // non-zero value otherwise.

  public:
    // CREATORS
    btemt_TcpTimerEventManager_ControlChannel();
        // Create an instance of this component by instantiating a connected
        // pair of sockets.

    ~btemt_TcpTimerEventManager_ControlChannel();
        // Close the internal sockets and destroy this object.

    // MANIPULATORS
    int clientWrite(bool forceWrite = false);
        // Write the control byte into the client handle.  If the specified
        // 'forceWrite' is 'true' write to the client handle even if a previous
        // write is outstanding.  By default, 'forceWrite' is 'false' and the
        // control byte is not written to the client handle if a previous write
        // is outstanding.  Return the number of bytes written on success, and
        // a negative value otherwise.

    int serverRead();
        // Read as many bytes as possible from the server buffer without
        // blocking.  Return the number of bytes read on success, and a
        // negative value otherwise.

    int recreateSocketPair();
        // Close the current internal socket pair and create a new pair of
        // connected sockets in its place.  Return 0 on success and a non-zero
        // value otherwise.

    // ACCESSORS
    bteso_SocketHandle::Handle clientFd();
        // Return a handle to the client socket.

    bteso_SocketHandle::Handle serverFd();
        // Return a handle to the server socket.
};

                       // ================================
                       // class btemt_TcpTimerEventManager
                       // ================================

class btemt_TcpTimerEventManager : public bteso_TimerEventManager {
    // This class provides a thread-enabled implementation of an event
    // manager for socket events and timers.  The supported event types for
    // sockets are ACCEPT, CONNECT, READ, and WRITE; only READ and WRITE
    // can be registered simultaneously for a single socket.  Timers are stable
    // (i.e., the relative order of registrations is maintained).  Callbacks
    // are invoked from dedicated threads, created internally for this purpose
    // by this component.

  public:
    // TYPES
    enum Hint {
        // This enum lists the usage pattern hints that can be provided to this
        // event manager.
        //
        // DEPRECATED: The enumerated hints are not supported and will be
        // ignored by this component.

        BTEMT_NO_HINT,                 // The (de)registrations will likely be
                                       // frequent.

        BTEMT_INFREQUENT_REGISTRATION  // The (de)registrations will likely be
                                       // infrequent.
#ifndef BDE_OMIT_INTERNAL_DEPRECATED
      , NO_HINT                 = BTEMT_NO_HINT
      , INFREQUENT_REGISTRATION = BTEMT_INFREQUENT_REGISTRATION
#endif // BDE_OMIT_INTERNAL_DEPRECATED
    };

#ifndef BDE_OMIT_INTERNAL_DEPRECATED
    typedef bcef_Vfunc0                   Callback;
        // DEPRECATED: Use 'bteso_TimerEventManager::Callback' instead.
#endif // BDE_OMIT_INTERNAL_DEPRECATED

  private:
    // PRIVATE TYPES
    enum State {
        BTEMT_ENABLED  = 0,  // dispatching thread is running
        BTEMT_DISABLED = 1   // dispatching thread is not running
    };

    typedef btemt_TcpTimerEventManager_ControlChannel ControlChannel;
        // Channel for sending messages from external threads operating on this
        // component and its internal dispatch thread.

    // DATA
    mutable bcema_Pool         d_requestPool;     // memory pool for operations

    mutable bcec_Queue<btemt_TcpTimerEventManager_Request*>
                               d_requestQueue;    // queue of requests to
                                                  // dispatcher thread

    mutable bcemt_ThreadUtil::Handle
                               d_dispatcher;      // dispatcher thread handle

    volatile State             d_state;           // the state of the
                                                  // dispatcher thread

    bces_AtomicInt             d_terminateThread; // signals end of dispatcher

    mutable bcemt_RWMutex      d_stateLock;       // protects access to the
                                                  // state changes via
                                                  // 'enable' and 'disable'

    bdef_Function<void (*)()>  d_dispatchThreadEntryPoint;
                                                  // functor containing the
                                                  // dispatch thread's entry
                                                  // point

    bteso_EventManager        *d_manager_p;       // socket event manager

    int                        d_isManagedFlag;   // whether or not event
                                                  // manager is internal or
                                                  // external

    bsl::vector<bdef_Function<void (*)()> >
                              *d_executeQueue_p;  // queue of executed timers
                                                  // (pointer, to be swappable
                                                  // in dispatcher thread loop)

    mutable bcemt_Mutex        d_executeQueueLock;
                                                  // protects access to the
                                                  // execute queue

    bcec_TimeQueue<bdef_Function<void (*)()> >
                               d_timerQueue;      // queue of registered timers

    mutable bdema_ManagedPtr<ControlChannel>
                               d_controlChannel_p;
                                                  // channel for sending
                                                  // control bytes from
                                                  // external threads operating
                                                  // on this manager to unlock
                                                  // its internal 'dispatch'
                                                  // thread after they post a
                                                  // message for the 'dispatch'
                                                  // thread to process

    mutable bteso_TimeMetrics  d_metrics;         // cached metrics

    const bool                 d_collectMetrics;  // whether to update
                                                  // 'd_metrics'

    bces_AtomicInt             d_numTotalSocketEvents;
                                                  // the total number of all
                                                  // socket events registered
                                                  // (excluding registered
                                                  // events of
                                                  // 'd_controlChannel_p')

    bslma::Allocator          *d_allocator_p;     // memory allocator (held,
                                                  // not owned)

    // NOT IMPLEMENTED
    btemt_TcpTimerEventManager(const btemt_TcpTimerEventManager&);
    btemt_TcpTimerEventManager& operator=(const btemt_TcpTimerEventManager&);

    // PRIVATE MANIPULATORS
    void initialize();
        // Initialize this event manager.

    void dispatchThreadEntryPoint();
        // Entry point for the dispatch thread.

    void controlCb();
        // Internal callback method to process control information received
        // on 'd_controlChannel_p.serverFd()'.

    int initiateRead(bool executeInSameThread);
        // Initiate a read request to the internal control channel's 'serverFd'
        // so that it is operational and can wait for subsequent writes.  Use
        // the specified 'executeInSameThread' to specify if the write should
        // happen in the same or a separate thread.  Return 0 on success and a
        // non-zero value otherwise.

    int reinitializeControlChannel();
        // Reinitialize this event manager's internal control channel.  Return
        // 0 on success and a non-zero value otherwise.

  public:
    // CREATORS
    explicit
    btemt_TcpTimerEventManager(bslma::Allocator *basicAllocator = 0);
    explicit
    btemt_TcpTimerEventManager(bool              collectTimeMetrics,
                               bslma::Allocator *basicAllocator = 0);
    btemt_TcpTimerEventManager(bool              collectTimeMetrics,
                               bool              poolTimerMemory,
                               bslma::Allocator *basicAllocator = 0);
        // Create an event manager.  Optionally specify 'collectTimeMetrics'
        // indicating whether this event manager should collect timing metrics.
        // If 'collectTimeMetrics' is unspecified or 'true' then the event
        // manager will provide a categorization of the time it spends
        // processing data via 'timeMetrics()', and if 'collectTimeMetrics' is
        // 'false' the value of 'timeMetrics()' is unspecified.  Optionally
        // specify 'poolTimerMemory' indicating whether the memory used for
        // internal timers should be pooled.  If 'poolTimerMemory' is
        // unspecified then the memory used for allocating timers will not be
        // pooled.  Optionally specify a 'basicAllocator' used to supply
        // memory.  If 'basicAllocator' is 0, the currently installed default
        // allocator is used.  The behavior is undefined unless
        // 'basicAllocator' refers to a *thread* *safe* allocator.  Note that
        // the dispatcher thread is NOT started by this method (i.e., it must
        // be started explicitly).

    explicit
    btemt_TcpTimerEventManager(Hint              registrationHint,
                               bslma::Allocator *basicAllocator = 0);
    btemt_TcpTimerEventManager(Hint              registrationHint,
                               bool              collectTimeMetrics,
                               bslma::Allocator *basicAllocator = 0);
    btemt_TcpTimerEventManager(Hint              registrationHint,
                               bool              collectTimeMetrics,
                               bool              poolTimerMemory,
                               bslma::Allocator *basicAllocator = 0);
        // The 'registrationHint' parameter is ignored, the behavior is exactly
        // as it would be had the corresponding constructor without that
        // argument been called.  Create an event manager.  Optionally specify
        // 'collectTimeMetrics' indicating whether this event manager should
        // collect timing metrics.  If 'collectTimeMetrics' is unspecified or
        // 'true' then the event manager will provide a categorization of the
        // time it spends processing data via 'timeMetrics()', and if
        // 'collectTimeMetrics' is 'false' the value of 'timeMetrics()' is
        // unspecified.  Optionally specify 'poolTimerMemory' indicating
        // whether the memory used for internal timers should be pooled.  If
        // 'poolTimerMemory' is unspecified then the memory used for allocating
        // timers will not be pooled.  Optionally specify a 'basicAllocator'
        // used to supply memory.  If 'basicAllocator' is 0, the currently
        // installed default allocator is used.  The behavior is undefined
        // unless 'basicAllocator' refers to a *thread* *safe* allocator.  Note
        // that the dispatcher thread is NOT started by this method (i.e., it
        // must be started explicitly).
        //
        // DEPRECATED: Use the corresponding constructor without the
        // 'registrationHint' argument.

    btemt_TcpTimerEventManager(bteso_EventManager  *rawEventManager,
                               bslma::Allocator    *basicAllocator = 0);
        // Create an event manager with timer support that uses the
        // specified 'rawEventManager' to monitor for socket events.
        // Optionally specify a 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.  'basicAllocator' must refer to a *thread* *safe* allocator,
        // and the behavior is undefined, otherwise.  The value of the event
        // manager's 'timeMetrics()' object will be unspecified, clients
        // interested in those metrics must use the 'bteso_TimeMetrics'
        // object provided to 'rawEventManager' on its construction.   Note
        // that the dispatcher thread is NOT started by this method (and,
        // therefore, must be started explicitly).

    virtual ~btemt_TcpTimerEventManager();
        // Terminate the dispatcher thread, if it is running, and destroy this
        // event manager without invoking any registered callback.

    // MANIPULATORS
    virtual void deregisterAllSocketEvents();
        // Deregister all callbacks associated with any event on any socket
        // handle so that no callbacks are not invoked when any event occurs
        // on any handle.

    virtual void deregisterAllTimers();
        // Remove all timer callbacks currently registered.  The number of
        // timers (as reported by 'numTimers') method is 0 after this
        // method is completed.

    virtual void deregisterAll();
        // Deregister all callbacks currently registered.  Note that the
        // behavior is equivalent to the sequence:
        //..
        //  deregisterAllTimers();
        //  deregisterAllSocketEvents();
        //..

    virtual void deregisterSocketEvent(
                                      const bteso_SocketHandle::Handle& handle,
                                      bteso_EventType::Type             event);
        // Deregister the callback associated with the specified 'event' on the
        // specified 'handle' so that said callback will not be invoked if the
        // 'event' occurs.

    virtual void deregisterSocket(const bteso_SocketHandle::Handle& handle);
        // Deregister all callbacks associated with any event on the specified
        // 'handle' such that no callback will be invoked if an event
        // occurs on the 'handle'.

    virtual void deregisterTimer(const void *timerId);
        // Deregister the callback associated with the specified 'timerId'
        // (returned when the timer callback was registered) so that the
        // callback will not be invoked at the appointed time.

    int disable();
        // Destroy the internal thread responsible for monitoring sockets and
        // dispatching timer and socket callbacks.  Return 0 on success and
        // a non-zero value otherwise.  If this event manager is not already
        // enabled, as reported by 'isEnabled' method, this method returns
        // immediately with 0.  Otherwise, it will block until the internal
        // thread exits.  Note that 'disable' will always fail if invoked,
        // through a callback, in the internal thread.

    int enable();
    int enable(const bcemt_Attribute& attribute);
        // Create the internal thread responsible for monitoring sockets and
        // dispatching timer and socket callbacks, optionally setting the
        // 'attribute' of the new thread to the specified 'attribute'.  Return
        // 0 on success and a non-zero value otherwise.  If this event manager
        // is already enabled, as reported by 'isEnabled' method, this method
        // returns immediately with 0.  Otherwise, it will block until the
        // internal thread is started or failure occurs.

    void execute(const bdef_Function<void (*)()>& functor);
        // Execute the specified 'functor' in the internal thread, if it is
        // started.

    void clearExecuteQueue();
        // Clear the functors enqueued to the execute queue.

    virtual int registerSocketEvent(const bteso_SocketHandle::Handle&   handle,
                                    bteso_EventType::Type               event,
                                    const bteso_EventManager::Callback& cb);
        // Register the specified 'cb' functor to be invoked whenever the
        // specified 'event' occurs on the socket specified by 'handle'.
        // Return 0 on success and a negative number on error.  No two
        // different socket events can have callbacks registered with the
        // same socket handle other than read and write.  Any invocation of
        // this method that would cause this to occur will result in an error
        // and the callback will not be registered.  The callback is recurring
        // (i.e., it remains registered until it is explicitly deregistered).
        // Note that the callback will be invoked only from the internal
        // thread and that the callback may be invoked before this method
        // returns.

    virtual void *registerTimer(const bdet_TimeInterval&            timeout,
                                const bteso_EventManager::Callback& cb);
        // Register the specified 'cb' functor to be invoked when the
        // absolute time of the specified 'timeout' is reached or exceeded.
        // Return a 'void*' registration id which can be used to deregister
        // this timer before expiration.  Specifying a 'timeout'
        // previous to the current time will result in the associated 'cb'
        // being executed almost immediately.  Note also
        // that the callback is not recurring (i.e., after being invoked it is
        // deregistered automatically).  Note that the callback will be
        // invoked only from the internal thread and that the callback
        // may be invoked before this method returns.

    int rescheduleTimer(const void               *timerId,
                        const bdet_TimeInterval&  timeout);
        // Reschedule the timer indicated by the specified 'timerId' such that
        // the callback function supplied to 'registerTimer' will be invoked
        // after the specified 'expiryTime' is reached.  Return 0 on success,
        // and a negative value otherwise.  'expiryTime' is expressed as the
        // absolute time from 00:00:00 UTC, January 1, 1970 (the epoch time
        // defined by 'bdetu_epoch').  The behavior is undefined unless
        // 'timerId' is a timer id returned from 'registerTimer', and has not
        // subsequently been deregistered.  Note that if 'expiryTime' is
        // earlier than the current time ('bdetu_SystemTime::now()') the
        // associated callback will be invoked the first time that the
        // callbacks are dispatched.

    // ACCESSORS
    virtual bool hasLimitedSocketCapacity() const;
        // Return 'true' if this event manager has a limited socket capacity,
        // and 'false' otherwise.

    virtual int isRegistered(const bteso_SocketHandle::Handle& handle,
                             bteso_EventType::Type             event) const;
        // Return 1 if a callback is registered to be invoked when a socket
        // event of the specified 'event' occurs on the specified socket
        // 'handle', and 0 otherwise.

    virtual int numEvents() const;
        // Return the total number of callbacks registered with this event
        // manager for either timers or sockets.

    virtual int numTimers() const;
        // Return the number of timers that are currently registered.

    virtual int numSocketEvents(
                               const bteso_SocketHandle::Handle& handle) const;
        // Return the number of callbacks registered for the specified
        // socket 'handle'.

    int numTotalSocketEvents() const;
        // Return the number of socket callbacks registered with this event
        // manager.

    bteso_TimeMetrics *timeMetrics() const;
        // Return the address of the modifiable time metrics object that
        // provides a breakdown of the time this event manager has spent
        // processing data.  Note that the metrics managed by the returned
        // object have unspecified values unless 'hasTimeMetrics' returns
        // 'true'.

    bcemt_ThreadUtil::Handle dispatcherThreadHandle() const;
        // Return the thread handle of the dispatcher thread of this object.

    const ControlChannel *controlChannel() const;
        // Return a pointer providing non-modifiable access to this object's
        // internal control channel.

    int isEnabled() const;
        // Return 1 if the dispatch thread is created/running and 0
        // otherwise.

    bool hasTimeMetrics() const;
        // Return 'true' if the object returned by 'timeMetrics()' contains a
        // valid value, and 'false' otherwise.  This value will be 'false' if
        // either the (optional) 'collectTimeMetrics'  value supplied at
        // construction was 'false', or if a 'rawEventManager' was provided at
        // construction.  Note that if the value is 'false', the value of the
        // object returned by 'timeMetrics()' is not specified.
};

//-----------------------------------------------------------------------------
//                      INLINE FUNCTIONS DEFINITIONS
//-----------------------------------------------------------------------------

               // -----------------------------------------------
               // class btemt_TcpTimerEventManager_ControlChannel
               // -----------------------------------------------

// CREATORS
inline
btemt_TcpTimerEventManager_ControlChannel::
    ~btemt_TcpTimerEventManager_ControlChannel()
{
    bteso_SocketImpUtil::close(static_cast<bteso_SocketHandle::Handle>(
                                                                    d_fds[0]));
    bteso_SocketImpUtil::close(static_cast<bteso_SocketHandle::Handle>(
                                                                    d_fds[1]));
}

// MANIPULATORS
inline
bteso_SocketHandle::Handle
btemt_TcpTimerEventManager_ControlChannel::clientFd()
{
    return static_cast<bteso_SocketHandle::Handle>(d_fds[0].loadRelaxed());
}

inline
bteso_SocketHandle::Handle
btemt_TcpTimerEventManager_ControlChannel::serverFd()
{
    return static_cast<bteso_SocketHandle::Handle>(d_fds[1].loadRelaxed());
}

                        // --------------------------------
                        // class btemt_TcpTimerEventManager
                        // --------------------------------

// MANIPULATORS
inline
int btemt_TcpTimerEventManager::enable()
{
    return enable(bcemt_Attribute());
}

// ACCESSORS
inline
bool btemt_TcpTimerEventManager::hasLimitedSocketCapacity() const
{
    return d_manager_p->hasLimitedSocketCapacity();
}

inline
bteso_TimeMetrics *btemt_TcpTimerEventManager::timeMetrics() const
{
    return &d_metrics;
}

inline
bcemt_ThreadUtil::Handle
btemt_TcpTimerEventManager::dispatcherThreadHandle() const
{
    return d_dispatcher;
}

inline
const btemt_TcpTimerEventManager_ControlChannel *
btemt_TcpTimerEventManager::controlChannel() const
{
    return d_controlChannel_p.ptr();
}

inline
bool btemt_TcpTimerEventManager::hasTimeMetrics() const
{
    return d_collectMetrics;
}

}  // close namespace BloombergLP

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2005
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
