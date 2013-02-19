// btes_leakybucket.h                                                 -*-C++-*-
#ifndef INCLUDED_BTES_LEAKYBUCKET
#define INCLUDED_BTES_LEAKYBUCKET

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a mechanism to monitor the consumption rate of a resource.
//
//@CLASSES:
//   btes_LeakyBucket: a leaky bucket rate monitor
//
//@AUTHOR: Anvar Murtazin (amurtazin), Mikhail Kunitskiy (mkunitskiy1)
//
//@SEE_ALSO: btes_ratelimiter
//
//@DESCRIPTION This component provides a mechanism, 'btes_LeakyBucket', that
// implements a leaky bucket algorithm that allows clients to monitor whether a
// resource is being consumed at a particular rate.
//
// The name of this mechanism, leaky bucket, derives from an analogy of pouring
// water into a bucket with a hole at the bottom.  The maximum rate at which
// water will drain out the bucket depends on the size of the hole, and not on
// the rate at which water is poured into the bucket.  If more water is being
// poured into the bucket than being drained, the bucket will eventually
// overflow.  If the person pouring water into a leaky bucket ensures the
// bucket doesn't overflow, then the average rate they pour water will, over
// time, be limited by the rate at which water flows out of the bucket.  By
// analogy, a leaky bucket provides a means to limit the rate of consumption of
// some resource (water poured into the bucket) to a configured rate (the size
// of the hole in the bucket).
//
// The behavior of a leaky bucket is determined by two properties: the capacity
// and the drain rate.  The drain rate, measured in 'units/s', is the rate at
// which the resource is drained.  The capacity, measured in 'units', is the
// maximum amount of the resource that the leaky bucket can hold before it
// overflows.  'unit' is a generic unit of measurement (e.g., bytes, number of
// messages, packets, liters, clock cycles, etc.).  Note that the drain rate
// determines average rate of resource consumption, while the capacity
// restricts the time period over which the average actual rate of resource
// consumption approaches the drain rate.
//
///Adding Units
///------------
// Units can be added to a leaky bucket by either submitting them or reserving
// them.  Submitted units are removed from a leaky bucket at the drain rate,
// while reserved units remain unchanged until they are later either cancelled
// (removed from the leaky bucket) or submitted.
//
///Submitting Units
/// - - - - - - - -
// Units can be submitted to a leaky bucket by invoking the 'submit' method,
// and should be added only after the resource had been consumed.
//
// Figure 1 illustrates a typical workflow for submitting units to a leaky
// bucket.
//..
// Fig. 1:  Capacity = 5 units, Rate = 1 unit / second
//
//    Submit 5                     Submit 2
//
//    7|     |      7|     |       7|     |      7|     |
//    6|     |      6|     |       6|     |      6|     |
// c--5|~~~~~|   c--5|-----|    c--5|-----|   c--5|-----|
//    4|~~~~~|      4|     |       4|     |      4|     |
//    3|~~~~~|      3|     |       3|~~~~~|      3|     |
//    2|~~~~~|      2|     |       2|~~~~~|      2|     |
//    1|~~~~~|      1|~~~~~|       1|~~~~~|      1|     |
//     +-- --+       +-- --+        +-- --+       +-- --+
//
// Time: t0          t0 + 4s        t0 + 4s       t0 + 10s
//..
// Suppose that we have an empty leaky bucket with a capacity of 'c = 5 units'
// and a drain rate of 'd = 1 units/s'.  At 't0', we submit 5 units to the
// leaky bucket, bringing the total number of units held up to 5.  At 't0 +
// 4s', 4 units have been drained from the leaky bucket, bringing the number of
// units held down to 1.  Finally, at 't0 + 10s', all units have been drained
// from the leaky bucket, making it empty.
//
// Unlike a real-life water bucket, units submitted to a leaky bucket don't
// spillover after its capacity has been exceeded, instead the leaky bucket may
// hold a number of units beyond its capacity, as examined in Figure 2 below.,
// these units are still contained in the leaky bucket.
//
// Figure 2 illustrates what happens if a leaky bucket exceeeds its capacity.
// This scenario is the same as that in Figure 1, but at time 't0 + 4s', we
// submit 6 units instead of 2.
//..
// Fig. 2: Capacity = 5 units, Rate = 1 unit / second
//
//    Submit 5                     Submit 6
//
//    7|     |      7|     |       7|~~~~~|      7|     |
//    6|     |      6|     |       6|~~~~~|      6|     |
// c--5|~~~~~|   c--5|-----|    c--5|~~~~~|   c--5|-----|
//    4|~~~~~|      4|     |       4|~~~~~|      4|     |
//    3|~~~~~|      3|     |       3|~~~~~|      3|     |
//    2|~~~~~|      2|     |       2|~~~~~|      2|     |
//    1|~~~~~|      1|~~~~~|       1|~~~~~|      1|~~~~~|
//     +-- --+       +-- --+        +-- --+       +-- --+
//
// Time: t0          t0 + 4s        t0 + 4s       t0 + 10s
//..
// At 't0 + 4s', when the number of units held by the leaky bucket is 1, we
// submit 6 more units.  This brings the number of units held to 7, which
// exceeds the capacity of the leaky bucket.  At 't0 + 10s', 6 units had been
// drained from the leaky bucket bring the number of units held down to 1.
//
///Reserving Units
///- - - - - - - -
// Units can be reserved for a leaky bucket using the 'reserve' method, and
// they may be later canceled using the 'cancelReserved' method or submitted
// using the 'submitReserved' method.  Unlike submitted units, reserved units
// do *not* drain from the leaky bucket; like submitted units, reserved units
// count toward the total number of units for the purposes of determining
// whether a leaky bucket has exceeded its capacity.  Reserving units
// effectively decreases the capacity of a leaky bucket.  Therefore, the time
// interval between reserving units and submitting or canceling the reservation
// should be kept as short as possible.  For a practical example of using
// reserved units, please see 'btes_reservationguard'.
//
// Figure 3 illustrate an example of how reserving units works in a leaky
// bucket.
//..
// Fig. 3: Capacity = 5 units, Rate = 1 unit / second
//
//    Reserve 4                 Submit 3                  Cancel 1
//                            from reserve              from reserve
//
//    7|     |     7|     |     7|     |     7|     |     7|     |
//    6|     |     6|     |     6|     |     6|     |     6|     |
// c--5|-----|  c--5|-----|  c--5|-----|  c--5|-----|  c--5|-----|
//    4|#####|     4|#####|     4|~~~~~|     4|     |     4|     |
//    3|#####|     3|#####|     3|~~~~~|     3|     |     3|     |
//    2|#####|     2|#####|     2|~~~~~|     2|     |     2|     |
//    1|#####|     1|#####|     1|#####|     1|#####|     1|     |
//     +-- --+      +-- --+      +-- --+      +-- --+      +-- --+
//
// Time: t0         t0 + 5s      t0 + 6s      t0 + 9s      t0 + 10s
//..
// Suppose that we have an empty leaky bucket with a capacity of 'c = 5 units'
// and a drain rate of 'd = 1 units/s'.  At 't0' we reserve 4 units.  At 't0 +
// 5s', we observe that none of the reserved units are drained from the leaky
// bucket.  At 't0 + 6s', we submit 3 of the previously reserved units, which
// brings the number of reserved units down to 1 and the number of units held
// up to 3.  At 't0 + 9s', we observe that all but the remaining reserved unit
// have been drained from the bucket.  Finally, at 't0 + 10s', we cancel the
// remaining reserved unit.
//
///Monitoring Resource Usage
///-------------------------
// The recommended usage of a leaky bucket is to first check whether 1 unit can
// be added without causing the leaky bucket to overflow, and if so, consume
// the desired amount of the resource.  Afterwards, submit the amount of
// consumed resource to the leaky bucket.
//
///Checking for Overflow
///- - - - - - - - - - -
// A leaky bucket can be queried whether submitting a 1 more unit would cause
// it to overflow via the 'wouldOverflow' method.  This method facilitates the
// recommended usage of a leak bucket: check whether 1 more unit can be added
// without causing the leaky bucket to overflow, and if so, consume the desired
// amount of the resource (which may be more than 1).  Compared to the
// alternative -- checking whether the desired amount can be submitted without
// overflow -- this recommended usage may allow a limited spike in the rate of
// actual consumption when the leaky bucket is empty (which is often
// acceptable) but is able to sustain a long term average that is actually
// closer to the drain rate.
//
///Modeling a Network Connection
///-----------------------------
// The primary use case of leaky bucket is limiting the rate at which data is
// written on a network.  In this use case, the drain rate of the bucket
// corresponds to the *ideal* maximum transmission rate that the client wishes
// to enforce on their outgoing connection.  Clients may choose to provide a
// value related to the physical limitations of their network or any other
// arbitrary limit.  The function of a leaky bucket's capacity is to limit the
// time period over which the average actual transimission rate may exceed the
// configured drain rate of the leaky bucket (see 'Approximations' section and
// 'Sliding Time-Window' section).
//
///Approximations
///--------------
// Leaky bucket is modeled on a water bucket with a hole, but as a leaky bucket
// does not manage any resources, there are several approximations to this
// model:
//
//: 1 Units are submitted instantaneously to the leaky bucket, whereas the
//:   consumption of a resource occurs over time at a rate that depends on the
//:   nature and speed of the resource.
//:
//: 2 Leaky bucket simulates the consumption of a resource with a specified
//:   fixed drain rate, but the resource is actually consumed at different
//:   rates over time.  This approximation still guarantees that the actual
//:   consumption rate does not exceed the specified drain rate when amortized
//:   over some configured period of time (determined by the capacity and the
//:   drain rate of the bucket), but does not prevent the consumption rate from
//:   spiking above the drain rate for shorter periods of time (see 'Sliding
//:   Time-Window' section).
//
///Sliding Time-Window
///-------------------
// One of the properties of the resource pattern created by using a lleaky
// bucket is an approximation of a sliding time window over which the average
// consumption rate is guaranteed to be less than the drain rate.  This time
// period can be calculated using the leaky bucket's capacity and drain rate,
// which can be conveniently performed using the 'calculateTimeWindow' class
// method.
//
///Time Synchronization
///--------------------
// Leaky bucket does not utilize an internal timer, so timing must be handled
// manually.  Clients can specify an initial time interval for the leaky bucket
// at construction or using the 'reset' method.  Whenever the number of units
// in a leaky bucket needs to be updated, clients must invoke the 'updateState'
// method specifying the current time interval.  Since leaky bucket cares only
// about the elapsed time (not absolute time), the specified time intervals may
// be relative to any arbitrary time origin, though all of them must refer to
// the same origin.  For the sake of consistency, clients are encouraged to use
// the unix epoch time (such as the values returned by
// 'bdetu_SystemTime::now').
//
///Usage
///-----
// This section illustrates the intended use of this component.
//
///Example 1: Controlling Network Traffic Generation
///-------------------------------------------------
// In some systems, data is processed faster than they are consumed by I/O
// interfaces.  This could lead to data loss due to the overflowing of the
// buffers where data is queued before being processed.  In other systems,
// generic resources are shared, and their consumption might need to be managed
// in order to guarantee quality-of-service (QOS).
//
// Suppose we have a network interface capable of transferring at a rate of
// 1024 byte/s and an application wants to transmit 5 KiB (5120 bytes) of data
// over that network in 20 different 256-bytes data chunks.  We want to send
// data over this interface and want to ensure the transmission uses on average
// less than 50% of the available bandwidth, or 512 byte/s.  In this way, other
// clients can still reasonably send and receive data using the same network
// interface.
//
// Further suppose that we have a function, 'sendData', that transmits a
// specified data buffer over that network interface:
//..
//  bool sendData(const char *buffer, size_t dataSize);
//      // Send the specified 'buffer' of the specified size 'dataSize' through
//      // the network interface.  Return 'true' if data was sent successfully,
//      // and 'false' otherwise.
//..
// First, we create a leaky bucket having a drain rate of 512 bytes/s, a
// capacity of 2560 bytes, and a time origin set to the current time (as an
// interval from unix epoch).  Note that 'unit', the unit of measurement for
// leaky bucket, corresponds to 'byte' in this example:
//..
//  bsls_Types::Uint64 rate     = 512;  // bytes/second
//  bsls_Types::Uint64 capacity = 2560; // bytes
//  bdet_TimeInterval  now      = bdetu_SystemTime::now();
//
//  btes_LeakyBucket   bucket(rate, capacity, now);
//..
// Then, we define a data buffer to be sent, the size of each data chunk, and
// the total size of the data to transmit:
//..
//  char               buffer[5120];
//  unsigned int       chunkSize  = 256;             // in bytes
//  bsls_Types::Uint64 totalSize  = 20 * chunkSize;  // in bytes
//  bsls_Types::Uint64 dataSent   = 0;               // in bytes
//
//  // Load 'buffer'.
//  // ...
//..
// Notice that, for the sake of brevity, we elide the loading of 'buffer' with
// the data to be sent.
//
// Now, we send the chunks of data using a loop.  For each iteration, we check
// whether submitting another byte would cause the leaky bucket to overflow.
// If not, we send an additional chunk of data and submit the number of bytes
// sent to the leaky bucket.  Note that 'submit' is invoked only after the data
// has been sent.
//..
//  char *data = buffer;
//  while (dataSent < totalSize) {
//      now = bdetu_SystemTime::now();
//      if (!bucket.wouldOverflow(now)) {
//          if (true == sendData(data, chunkSize)) {
//              data += chunkSize;
//              bucket.submit(chunkSize);
//              dataSent += chunkSize;
//          }
//      }
//..
// Finally, if submitting another byte will cause the leaky bucket to overflow,
// then we wait until the submission will be allowed by waiting for an amount
// time returned by the 'calculateTimeToSubmit' method:
//..
//      else {
//          bdet_TimeInterval timeToSubmit = bucket.calculateTimeToSubmit(now);
//
//          // Round up the number of microseconds.
//          bsls_Types::Uint64 uS = timeToSubmit.totalMicroseconds() +
//                                 (timeToSubmit.nanoseconds() % 1000) ? 1 : 0;
//          bcemt_ThreadUtil::microSleep(uS);
//      }
//  }
//..
// Notice that we wait by putting the thread into a sleep state instead of
// using busy-waiting to better optimize for multi-threaded applications.
//..

#ifndef INCLUDED_BTESCM_VERSION
#include <btescm_version.h>
#endif

#ifndef INCLUDED_BDET_TIMEINTERVAL
#include <bdet_timeinterval.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

namespace BloombergLP {

                        //=======================
                        // class btes_LeakyBucket
                        //=======================

class btes_LeakyBucket {
    // This mechanism implements a leaky bucket that allows clients to monitor
    // whether a resource is being consumed at a particular rate.  The behavior
    // of a leak bucket is determined by two properites: the drain rate (in
    // units/s) and capacity (in units), both of which can be specified at
    // construction or using the 'setRateAndCapacity' method.
    //
    // Units can be added to a leaky bucket by either submitting them using the
    // 'submit' method or reserving them using the 'reserve' method.  Submitted
    // units are removed from a leaky bucket at the drain rate, while reserved
    // units stays unaffected in a leaky bucket until they are either cancelled
    // (removed from the leaky bucket) using the 'cancelReserved' method or
    // submitted using the 'submitReserved' method.
    //
    // Adding units to a leaky bucket will cause it to overflow if after the
    // units are added, the total number of units in the leaky bucket
    // (including both submitted and reserved units) exceeds its capacity.  A
    // leaky bucket can be queried whether adding a specified number of units
    // would cause it to overflow via the 'wouldOverflow' method.  If
    // submitting units to a leaky bucket will cause it to overflow, the
    // estimated amount of time to wait before 1 more units can be submitted
    // without causing the leaky bucket to overflow can be determined using the
    // 'calculateTimeToSubmit' method.
    //
    // The state of a leaky bucket must be updated manually using the
    // 'updateState' method supplying the current time interval.  The time
    // intervals supplied should all refer to the same time origin.
    //
    // A leaky bucket keeps some statistics, including the number of submitted
    // units, that can be accessed using the 'getStatistics' method and reset
    // using the 'resetStatistics' method.
    //
    // The class invariants are:
    //: o 'capacity() > 0'
    //: o 'drainRate() > 0'
    //
    // This class:
    //: o is *exception* *neutral* (agnostic)
    //: o is *const* *thread-safe*
    // For terminology see 'bsldoc_glossary'.

    // DATA
    bsls_Types::Uint64 d_drainRate;            // drain rate in units per
                                               // second

    bsls_Types::Uint64 d_capacity;             // the bucket capacity in
                                               // units

    bsls_Types::Uint64 d_unitsReserved;        // reserved units

    bsls_Types::Uint64 d_unitsInBucket;        // number of units currently in
                                               // the bucket

    bsls_Types::Uint64 d_fractionalUnitDrainedInNanoUnits;
                                               // fractional number of units
                                               // that is carried from the
                                               // last drain operation

    bdet_TimeInterval  d_lastUpdateTime;       // time of last drain, updated
                                               // via the 'updateState' method

    bdet_TimeInterval  d_maxUpdateInterval;    // time to drain maximum number
                                               // of units

    bsls_Types::Uint64 d_statSubmittedUnits;   // submitted unit counter,
                                               // number of submitted units
                                               // since last reset

    bsls_Types::Uint64 d_statSubmittedUnitsAtLastUpdate;
                                               // submitted unit counter saved
                                               // during last update

    bdet_TimeInterval  d_statisticsCollectionStartTime;
                                               // start time for the submitted
                                               // unit counter

  private:
    // NOT IMPLEMENTED
    btes_LeakyBucket& operator=(const btes_LeakyBucket&);
    btes_LeakyBucket(const btes_LeakyBucket&);

  public:
    // CLASS METHODS
    static bdet_TimeInterval calculateDrainTime(bsls_Types::Uint64 numUnits,
                                                bsls_Types::Uint64 drainRate,
                                                bool               ceilFlag);
        // Return the time interval required to drain the specified 'numUnits'
        // at the specified 'drainRate', round up the number of nanoseconds in
        // the time interval if the specified 'ceilFlag' is set to 'true',
        // otherwise, round down the number of nanoseconds.  The behavior is
        // undefined unless the number of seconds in the calculated interval
        // may be represented by a 64-bit signed integral type.

    static bdet_TimeInterval calculateTimeWindow(bsls_Types::Uint64 drainRate,
                                                 bsls_Types::Uint64 capacity);
        // Return the time interval over which a leaky bucket *approximates* a
        // moving-total of submitted units, as the rounded-down ratio between
        // the specified 'capacity' and the specified 'drainRate'.  If the
        // rounded ratio is 0, return a time interval of 1 nanosecond.  The
        // behavior is undefined unless 'capacity / drainRate' can be
        // represented with 64-bit signed integral type.

    static bsls_Types::Uint64 calculateCapacity(
                                          bsls_Types::Uint64       drainRate,
                                          const bdet_TimeInterval& timeWindow);
        // Return the capacity of a leaky bucket as the rounded-down product of
        // the specified 'drainRate' by the specified 'timeWindow'.  If the
        // result evaluates to 0, return 1.  The behavior is undefined unless
        // the product of 'drainRate' and 'timeWindow' can be represented by a
        // 64-bit unsigned integral type.

    // CREATORS
    btes_LeakyBucket(bsls_Types::Uint64       drainRate,
                     bsls_Types::Uint64       capacity,
                     const bdet_TimeInterval& currentTime);
        // Create an empty leaky bucket having the specified 'drainRate', the
        // specified 'capacity', and the specified 'currentTime' as the initial
        // 'lastUpdateTime'.  The behavior is undefined unless '0 < newRate'
        // and '0 < newCapacity'.

#if defined(BSLS_ASSERT_SAFE_IS_ACTIVE)
    // The following destructor is generated by the compiler, except in "SAFE"
    // build modes (e.g., to enable the checking of class invariants).

    ~btes_LeakyBucket();
        // Destroy this object.
#endif

    // MANIPULATORS
    bdet_TimeInterval calculateTimeToSubmit(
                                         const bdet_TimeInterval& currentTime);
        // If 1 more unit can be submitted to this leaky bucket without causing
        // it to overflow, then return a time interval of 0 if .  Otherwise,
        // first update the state of this this leaky bucket to the specified
        // 'currentTime'.  Then, Return the estimated time interval that should
        // pass from 'currentTime' until 1 more unit can be submitted to this
        // leaky bucket without causing it to overflow.  The number of
        // nanoseconds in the returned time interval is rounded up.  Note that
        // a time interval of 0 can still be return after the state of this
        // leaky bucket has been updated to 'currentTime'.  Also note that
        // after waiting for the returned time interval, clients should
        // typically check again using this method, because additional units
        // may have been submitted in the interim.

    void reserve(bsls_Types::Uint64 numUnits);
        // Reserve the specified 'numUnits' for future use by this leaky
        // bucket.  The behavior is undefined unless 'unitsReserved() +
        // unitsInBucket() + numOfUnits' can be represented by a 64-bit
        // unsigned integral type.  Note that after this operation, this bucket
        // may overflow.  Also note that the time interval between the
        // invocations of 'reserve' and 'submitReserved' or 'cancelReserved'
        // should be kept as short as possible; otherwise, the precision of the
        // time interval calculated by 'calculateTimeToSubmit' may be
        // negatively affected.

    void cancelReserved(bsls_Types::Uint64 numUnits);
        // Cancel the specified 'numUnits' that were previously reserved.  This
        // method reduces the number of reserved units by 'numUnits'.  The
        // behavior is undefined unless 'numUnits <= unitsReserved()'.

    void submitReserved(bsls_Types::Uint64 numUnits);
        // Submit the specified 'numUnits' that were previously reserved.  This
        // method reduces the number of reserved units by 'numUnits' and
        // submits 'numUnits' to this leaky bucket.  The behavior is undefined
        // unless 'numUnits <= unitsReserved()'.

    void reset(const bdet_TimeInterval& currentTime);
        // Reset the the following statistic counters for this leaky bucket to
        // 0: 'unitsInBucket', 'unitsReserved', 'submittedUnits', and
        // 'unusedUnits'.  Set the 'lastUpdateTime' and the
        // 'statisticCollectionStartTime' to the 'currentTime' of this leaky
        // bucket.

    void resetStatistics();
        // Reset the statics collected for this leaky bucket by setting the
        // number of units used and the number of units submitted to 0, and set
        // the 'statisticsCollectionStartTime' to the 'lastUpdateTime' of this
        // leaky bucket.

    void setRateAndCapacity(bsls_Types::Uint64 newRate,
                            bsls_Types::Uint64 newCapacity);
        // Set the drain rate of this leaky bucket to the specified 'newRate'
        // and the capacity of this leaky bucket to the specified
        // 'newCapacity'.  The behavior is undefined unless '0 < newRate' and
        // '0 < newCapacity'.

    void submit(bsls_Types::Uint64 numUnits);
        // Submit the specified 'numUnits' to this leaky bucket.  The behavior
        // is undefined unless 'unitsReserved() + unitsInBucket() + numUnits'
        // can be represented by a 64-bit unsigned integral type.  Note that
        // after this operation, this leaky bucket may overflow.

    void updateState(const bdet_TimeInterval& currentTime);
        // Set the 'lastUpdateTime' of this leaky bucket to the specified
        // 'currentTime'.  If 'currentTime' is after 'lastUpdateTime', then
        // update the 'unitsInBucket' of this leaky bucket by subtracting from
        // it the number of units drained from 'lastUpdateTime' to
        // 'currentTime'.  If 'currentTime' is before the
        // 'statisticsCollectionStartTime' of this leaky bucket, set
        // 'statisticsCollectionStartTime' to 'currentTime'.

    bool wouldOverflow(const bdet_TimeInterval& currentTime);
        // Update the state of this this leaky bucket to the specified
        // 'currentTime', and return 'true' if adding 1 more unit to this leaky
        // bucket would cause the total number of units held by this leaky
        // bucket to exceed its capacity, and 'false' otherwise.  Note that
        // this method counts both submitted units and reserved units toward
        // the total number of units held by this leaky bucket .

    // ACCESSORS
    bsls_Types::Uint64 capacity() const;
        // Return the capacity of this leaky bucket.

    bsls_Types::Uint64 drainRate() const;
        // Return the drain rate of this leaky bucket.

    void getStatistics(bsls_Types::Uint64* submittedUnits,
                       bsls_Types::Uint64* unusedUnits) const;
        // Load, into the specified 'submittedUnits' and the specified
        // 'unusedUnits' respectively, the numbers of submitted units and the
        // number of unused units for this leaky bucket from the
        // 'statisticsCollectionStartTime' to the 'lastUpdateTime'.  The number
        // of unused units is the difference between the number of units that
        // could have been consumed and the number of units actually submitted
        // for the time period.

    bdet_TimeInterval lastUpdateTime() const;
        // Return the time interval when this leaky bucket was last updated.

    bdet_TimeInterval statisticsCollectionStartTime() const;
        // Return the time interval when the collection of the statistics (as
        // returned by 'getStatistics') started.

    bsls_Types::Uint64 unitsInBucket() const;
        // Return the number of submitted units in this leaky bucket.

    bsls_Types::Uint64 unitsReserved() const;
        // Return the number of reserved units in this leaky bucket.
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

                        //-----------------------
                        // class btes_LeakyBucket
                        //-----------------------

// CREATORS
#if defined(BSLS_ASSERT_SAFE_IS_ACTIVE)
inline
btes_LeakyBucket::~btes_LeakyBucket()
{
    BSLS_ASSERT_SAFE(0 < d_drainRate);
    BSLS_ASSERT_SAFE(0 < d_capacity);
}
#endif

// MANIPULATORS
inline
void btes_LeakyBucket::reserve(bsls_Types::Uint64 numUnits)
{
    // Check whether adding 'numUnits' causes an unsigned 64-bit integral type
    // to overflow.

    BSLS_ASSERT_SAFE(numUnits <= ULLONG_MAX - d_unitsReserved);

    BSLS_ASSERT_SAFE(
                 d_unitsInBucket <= ULLONG_MAX - d_unitsReserved - numUnits);

    d_unitsReserved += numUnits;
}

inline
void btes_LeakyBucket::cancelReserved(bsls_Types::Uint64 numUnits)
{
    BSLS_ASSERT_SAFE(numUnits <= d_unitsReserved);

    if (numUnits > d_unitsReserved) {
        d_unitsReserved = 0;
    }
    else {
        d_unitsReserved -= numUnits;
    }
}

inline
void btes_LeakyBucket::submitReserved(bsls_Types::Uint64 numUnits)
{
    BSLS_ASSERT_SAFE(numUnits <= d_unitsReserved);

    d_unitsReserved -= numUnits;

    submit(numUnits);
}


inline
void btes_LeakyBucket::reset(const bdet_TimeInterval& currentTime)
{
    d_lastUpdateTime = currentTime;
    d_unitsInBucket  = 0;
    d_unitsReserved  = 0;
    resetStatistics();
}

inline
void btes_LeakyBucket::resetStatistics()
{
    d_statisticsCollectionStartTime  = d_lastUpdateTime;
    d_statSubmittedUnits             = 0;
    d_statSubmittedUnitsAtLastUpdate = 0;
}

inline
void btes_LeakyBucket::submit(bsls_Types::Uint64 numUnits)
{
    // Check whether adding 'numUnits' causes an unsigned 64-bit integer type
    // to overflow.

    BSLS_ASSERT_SAFE(numUnits <= ULLONG_MAX - d_unitsInBucket);

    BSLS_ASSERT_SAFE(
                   d_unitsReserved <= ULLONG_MAX - d_unitsInBucket - numUnits);

    d_unitsInBucket  += numUnits;
    d_statSubmittedUnits += numUnits;
}

// ACCESSORS
inline
bsls_Types::Uint64 btes_LeakyBucket::capacity() const
{
    return d_capacity;
}

inline
bsls_Types::Uint64 btes_LeakyBucket::drainRate() const
{
    return d_drainRate;
}

inline
bdet_TimeInterval btes_LeakyBucket::lastUpdateTime() const
{
    return d_lastUpdateTime;
}

inline
bdet_TimeInterval btes_LeakyBucket::statisticsCollectionStartTime() const
{
    return d_statisticsCollectionStartTime;
}

inline
bsls_Types::Uint64 btes_LeakyBucket::unitsInBucket() const
{
    return d_unitsInBucket;
}

inline
bsls_Types::Uint64 btes_LeakyBucket::unitsReserved() const
{
    return d_unitsReserved;
}

}  // close enterprise namespace

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2007-2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
