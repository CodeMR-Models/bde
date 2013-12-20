// bcec_atomicringbufferindexmanager.cpp                            -*-C++-*-
#include <bcec_atomicringbufferindexmanager.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bcec_atomicringbufferindexmanager_cpp,"$Id$ $CSID$")

#include <bcemt_threadutil.h>

#include <bslalg_arraydestructionprimitives.h>
#include <bslalg_arrayprimitives.h>
#include <bsls_assert.h>

#include <bsl_iomanip.h>
#include <bsl_ostream.h>

#include <bsl_iostream.h>

namespace BloombergLP {

///Implementation Note
///===================
//
// Each 'bcec_AtomicRingBufferIndexManager' object maintains a circular buffer
// of atomic integers, 'd_states', that encode the states of the corresponding
// elements in an external circular buffer (e.g.,'bcec_AtomicRingBuffer').
// The bits in the atomic integers of the 'd_states' array, as well as both
// 'd_pushIndex' and 'd_popIndex' encode multiple pieces of information, as
// described below.
//
// Test case -2 in this component's test-driver renders various states of a
// short queue, which may be helpful in understanding its operation.
//
///Generation Count
///----------------
// In order to alleviate the possibility of the ABA problem a generation count
// is encoded into:
//
//: o Each element in the states array 'd_states'
//: o 'd_pushIndex'
//: o 'd_popIndex'
//
///Encoding of 'd_states' elements
///-------------------------------
// The elements of the 'd_states' array indicate the state of the
// corresponding element in the externally managed circular buffer.  Each
// 'd_states' element encodes:
//
//: o the 'ElementState' (e_EMPTY, e_WRITING, e_FULL, or e_READING)
//: o the generation count
//
// The bit layout of the atomic integers in the  'd_states' array is below:
//..
//
// |31 30 . . . . . . . . . . . . . . . . . 4 3 | 1 0 |
// .--------------------------------------------------.
// |    Generation Count                        |     |
// `--------------------------------------------------'
//                                                 ^--ElementState
//..
//
///Terminology: Combined Index
///---------------------------
// We define the term *Combined Index* to be the combination of an index
// position (into the circular buffer 'd_states') and the generation count
// using the following formula:
//..
//  Combined Index   = (Generation Count * Queue Capacity) + Element Index
//..
// A combined index has the nice property that incrementing a combined index
// whose element index is at the end of an array, sets the element index back
// to 0 and increments the generation count.  Both 'd_pushIndex' and
// 'd_popIndex' maintain a combined index.
//
// A couple derived formulas that are used in this component:
//..
//  Generation Count = Combined Index / Queue Capacity
//  Element Index    = Combined Index % Queue Capacity
//..
//
///Description of 'd_pushIndex' and 'd_PopIndex'
///---------------------------------------------
// 'd_PushIndex' and 'd_popIndex' both can be used to determine the index of
// the element at which the next push or pop should occur (respectively) as
// well as the generation count.  Additionally, 'd_pushIndex' encodes the
// disabled status for the queue.
//
// The 'd_pushIndex' data member is encoded as follows:
//..
// |31 30 . . . . . . . . . . . . . . . . . . . . . 0 |
// .--------------------------------------------------.
// |  |  (generation count * d_capacity) + push index |
// `--------------------------------------------------'
//  ^--Disabled bit
//..
// 'd_popIndex' is the same, but does not maintain a disabled bit.
//
///Maximum Capacity
///----------------
// As noted earlier, the index manager uses a generation count to avoid ABA
// problems.  In order for generation count to be effective we must ensure that
// 'd_pushIndex', 'd_popIndex', and 'd_states' elements each can represent at
// least two generations.  'd_states' elements each have 'sizeof(int) - 2'
// bits available to represent the generation count (which we assume is plenty
// for 2 generations), and 'd_popIndex' has one more bit available than
// 'd_pushIndex'.
//
// For 'd_pushIndex' to represent at least 2 generations, we must reserve at
// least 1 bit for the generation count in addition to the 1 bit reserved for
// the disabled flag.  This leads to a maximum supportable capacity of:
// '1 << ((sizeof(int) * 8) - 2)
//
///Maximum Generation Count and Maximum Combined Index
///---------------------------------------------------
// The maximum generation count and maximum combined index are per-object
// constants, derived from a circular buffer's capacity, that are stored
// within the 'd_maxGeneration' and 'd_maxCombinedIndex' data members
// respectively.
//
//: 'd_maxGeneration': The maximum *complete* generation that can be
//:                    represented within a combined index.
//:
//: 'd_maxCombinedIndex': The maximum combined index (which is equal to
//:                       'd_maxGeneration * capacity').
//
// A *complete* generation is a generation value that can be represented by
// every element in the 'd_states' array.
//
// Note there's a possibility that the final generation could be incomplete.
// If 'd_capacity' is not a power of 2, the last representable combined index
// in that generation would fall in the middle of the buffer.
//
///Defining of Full and Empty Queues
///---------------------------------
// 'reservePushBack' and 'reservePopIndex' both return an error status
// indicating the queue is either full or empty respectively.  Although the
// meaning of full or empty is intuitive, in a lock-free context where
// multiple readers and writers are simultanouesly updating a queue it is
// worth defining precisely:
//
// In the context of this implementation, a full or empty queue means the
// failure status for the 'testAndSwap' where we attempt to mark the cell
// either 'e_READING' or 'e_WRITING' is such that we can guarantee the queue
// was either full or empty at the point of that 'testAndSwap'.
//
///Use of BSLS_ASSERT
///------------------
// 'BSLS_ASSERT' is used frequently in this component to test expected
// internal invariants.  Generally that is the purview of the test-driver, but
// it is difficult to externally test the expected invariants in a
// multi-threaded context.

namespace {

enum ElementState {
    ///State Values
    /// - - - - - -
    // The following constants define the possible states for elements in the
    // externally managed queue.  Note that these states are maintained by the
    // index manager in the 'd_states' member variable.

    e_EMPTY    = 0,   // element is empty and available for writing
    e_WRITING  = 1,   // element is reserved for writing
    e_FULL     = 2,   // element has a value in it
    e_READING  = 3    // element is reserved for reading
};

const char *toString(ElementState state)
    // Return a 0 terminated description of the specified 'state'.
{
    switch (state) {
      case e_EMPTY:   return "EMPTY";
      case e_WRITING: return "WRITING";
      case e_FULL:    return "FULL";
      case e_READING: return "READING";
    };
    BSLS_ASSERT(false);
    return 0;
}

///State Constants
///- - - - - - - -
// The following constants are used to manipulate the bits of elements
// in the 'd_states' array.

static const unsigned int e_ELEMENT_STATE_MASK     = 0x3;
                                  // bitmask used to determine the
                                  // 'ElementState' value from a 'd_states'
                                  // element

static const unsigned int e_GENERATION_COUNT_SHIFT = 0x2;
                                  // number of bits to left-shift the
                                  // generation count in a 'd_states' element
                                  // to make room for 'ElementState' value --
                                  // must be base2Log(e_ELEMENT_STATE_MASK)

static const unsigned int e_NUM_REPRESENTABLE_ELEMENT_STATE_GENERATIONS =
                                                      1 << (sizeof(int)*8 - 2);
                                  // The maximum generation count that can be
                                  // represented in a 'd_states' element.

///Push-Index Constants
/// - - - - - - - - - -
// The following constants are used to manipulate and modify the bits of
// 'd_pushIndex'.  The bits of 'd_pushIndex' encode the index of the
// of the next element to be pushed, as well as the current generation
// count and a flag indicating whether the queue is disabled.

static const unsigned int e_DISABLED_STATE_MASK = 1 << ((sizeof(int) * 8) - 1);
                                      // bitmask for the disabled state bit in
                                      // 'd_pushIndex'

static const unsigned int e_NUM_REPRESENTABLE_COMBINED_INDICES =
                                                         e_DISABLED_STATE_MASK;
                                      // maximum representable number of
                                      // combinations of index and generation
                                      // count value for 'd_pushIndex' and
                                      // 'd_popIndex' (this is used to
                                      // determine 'd_maxGeneration')


///State Element Encoding
///- - - - - - - - - - -
// The following operations are used to encode and decode the 'ElementState'
// and generation count from 'd_states' elements.

inline
static unsigned int encodeElementState(unsigned int generation,
                                       ElementState indexState)
    // Return an encoded state value comprising the specified 'generation' and
    // the specified 'indexState'.  Note that the resulting encoded value is
    // appropriate for storage in the 'd_states' array.
{
    return (generation << e_GENERATION_COUNT_SHIFT) | indexState;
}

inline
static unsigned int decodeGenerationFromElementState(unsigned int encodedState)
    // Return the generation count of the specified 'encodedState'.  The
    // behavior is undefined unless 'value' was encoded by
    // 'encodeElementState'.  Note that 'encodedState' is typically obtained
    // from the 'd_states' array.
{
    return encodedState >> e_GENERATION_COUNT_SHIFT;
}

inline
static ElementState decodeStateFromElementState(unsigned int encodedState)
    // Return the element state of the specified 'encodedState'.  The
    // behavior is undefined unless 'encodedState' was encoded by
    // 'encodeElementState'.  Note that 'encodedState' is typically obtained
    // from the 'd_states' array.
{
    return ElementState(encodedState & e_ELEMENT_STATE_MASK);
}


///Index Operations
/// - - - - - - - -
// The following operations are used to manipulate 'd_index' and 'd_popIndex'
// members.

inline
static bool isDisabledFlagSet(unsigned int encodedPushIndex)
     // Return 'true' if the specified 'encodedPushIndex' has the disabled flag
     // set, and 'false otherwise.
{
    return (encodedPushIndex & e_DISABLED_STATE_MASK);
}

inline
static unsigned int discardDisabledFlag(unsigned int encodedPushIndex)
    // Return the push-index of the specified 'encodedPushIndex', discarding
    // the disabled flag.
{
    return (encodedPushIndex & ~e_DISABLED_STATE_MASK);
}

}  // close unnamed namespace

                     // ---------------------------------------
                     // class bcec_AtomicRingBufferIndexManager
                     // ---------------------------------------


// CLASS METHODS
unsigned int bcec_AtomicRingBufferIndexManager::numRepresentableGenerations(
                                                         unsigned int capacity)
{
    return bsl::min(e_NUM_REPRESENTABLE_COMBINED_INDICES / capacity,
                    e_NUM_REPRESENTABLE_ELEMENT_STATE_GENERATIONS);
}

int bcec_AtomicRingBufferIndexManager::circularDifference(
                                                 unsigned int minuend,
                                                 unsigned int subtrahend,
                                                 unsigned int modulo)
{
    BSLS_ASSERT(modulo     <= static_cast<unsigned int>(INT_MAX) + 1);
    BSLS_ASSERT(minuend    <  modulo);
    BSLS_ASSERT(subtrahend <  modulo);

    int difference = minuend - subtrahend;
    if (difference > static_cast<int>(modulo / 2)) {
        return difference - modulo;                                   // RETURN
    }
    if (difference < -static_cast<int>(modulo / 2)) {
        return difference + modulo;                                   // RETURN
    }
    return difference;
}


// CREATORS
bcec_AtomicRingBufferIndexManager::bcec_AtomicRingBufferIndexManager(
                                              unsigned int      capacity,
                                              bslma::Allocator *basicAllocator)
: d_pushIndex(0)
, d_pushIndexPad()
, d_popIndex(0)
, d_popIndexPad()
, d_capacity(capacity)
, d_maxGeneration(numRepresentableGenerations(capacity) - 1)
, d_maxCombinedIndex(numRepresentableGenerations(capacity) * capacity - 1)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
    BSLS_ASSERT_OPT(0        <  capacity);
    BSLS_ASSERT_OPT(capacity <= e_MAX_CAPACITY);

    d_states = static_cast<bsls::AtomicInt*>(
                d_allocator_p->allocate(sizeof(bsls::AtomicInt) * capacity));

    // We use default construction since the initial state, 'e_EMPTY', is the
    // default constructed state.

    bslalg::ArrayPrimitives::defaultConstruct(d_states,
                                              capacity,
                                              d_allocator_p);
}

bcec_AtomicRingBufferIndexManager::~bcec_AtomicRingBufferIndexManager()
{
    bslalg::ArrayDestructionPrimitives::destroy(d_states,
                                                d_states + d_capacity);
    d_allocator_p->deallocate(d_states);
}

// MANIPULATORS
int bcec_AtomicRingBufferIndexManager::reservePushIndex(
                                                     unsigned int *generation,
                                                     unsigned int *index)
{
    BSLS_ASSERT(0 != generation);
    BSLS_ASSERT(0 != index);

    enum Status { e_SUCCESS = 0, e_QUEUE_FULL = 1, e_DISABLED_QUEUE = -1 };

    unsigned int loadedPushIndex = d_pushIndex.loadRelaxed();
    unsigned int savedPushIndex  = -1;
    unsigned int combinedIndex, currIndex, currGeneration;


    // We use 'savedPushIndex' to ensure we attempt to acquire an index at
    // least twice before returning 'e_QUEUE_FULL'.  This prevents
    // pathological contention between reading and writing threads for a queue
    // of length 1.

    for (;;) {
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         isDisabledFlagSet(loadedPushIndex))) {
            return e_DISABLED_QUEUE;                                  // RETURN
        }

        // Attempt to swap the 'd_states' element referred to by the
        // push-index to 'e_WRITING'.

        combinedIndex  = discardDisabledFlag(loadedPushIndex);

        currGeneration = combinedIndex / d_capacity;
        currIndex      = combinedIndex % d_capacity;


        const int compare = encodeElementState(currGeneration, e_EMPTY);
        const int swap    = encodeElementState(currGeneration, e_WRITING);
        const int was     = d_states[currIndex].testAndSwap(compare, swap);

        if (compare == was) {
            // We've successfully changed the state and thus acquired
            // the index.  Exit the loop.

            *generation = currGeneration;
            *index      = currIndex;
            break;
        }

        // We've failed to reserve the index.  We can use the result of the
        // 'testAndSwap' to determine wehther the queue was full at the
        // point of the 'testAndSwap'.  Either:
        //
        //: 1 The cell is from the previous generation, meaning that the queue
        //:   is full (though if the cell is currently being read, we will
        //:   wait)
        //:
        //: 2 Another cell has reserved the cell for writing, but has not yet
        //:   incremented the pop index
        //:
        //: 3 The push index has been incremented between the value being
        //:   loaded, and the attempt to test and swap.


        unsigned int elementGeneration = decodeGenerationFromElementState(was);

        int difference = static_cast<int>(currGeneration -
                                          elementGeneration);



        if (difference == 1 || BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
            difference == -static_cast<int>(d_maxGeneration))) {
            // If the 'currentGeneration' is *one* past the 'elementGeneration'
            // then the queue is full.  Note that this condition was an
            // optimized expression of:
            // '0 < circularDifference(currGeneration, elementGeneration)'.

            BSLS_ASSERT(1 == circularDifference(currGeneration,
                                                elementGeneration,
                                                d_maxGeneration+1));

            ElementState state = decodeStateFromElementState(was);
            if (e_READING == state) {
                // Another thread is currently reading this cell, yield the
                // processor, reload the pushIndex, and return to the top of
                // the loop.

                bcemt_ThreadUtil::yield();
                loadedPushIndex = d_pushIndex.loadRelaxed();
                continue;
            }

            // It should now be possible for an empty cell to be in the
            // previous generation (as emptying the cell, increments its
            // generation).

            BSLS_ASSERT(e_EMPTY != state);

            if (savedPushIndex != loadedPushIndex) {
                // Make two attempts before returning that the queue is full.

                bcemt_ThreadUtil::yield();
                savedPushIndex = loadedPushIndex;
                loadedPushIndex = d_pushIndex.loadRelaxed();
                continue;
            }
            return e_QUEUE_FULL;                                      // RETURN
        }
        BSLS_ASSERT(0 >= circularDifference(currGeneration,
                                            elementGeneration,
                                            d_maxGeneration+1));

        // Another thread has already acquired this cell.  Attempt to
        // increment the push index.

        unsigned int next = nextCombinedIndex(combinedIndex);
        loadedPushIndex   = d_pushIndex.testAndSwap(combinedIndex, next);
    }

    // We've acquired the cell, attempt to increment the push index.

    unsigned int next = nextCombinedIndex(combinedIndex);
    d_pushIndex.testAndSwap(combinedIndex, next);

    return e_SUCCESS;
}

void bcec_AtomicRingBufferIndexManager::commitPushIndex(
                                                       unsigned int generation,
                                                       unsigned int index)
{
    BSLS_ASSERT(generation <= d_maxGeneration);
    BSLS_ASSERT(index      <  d_capacity);
    BSLS_ASSERT(e_WRITING  == decodeStateFromElementState(d_states[index]));
    BSLS_ASSERT(generation ==
                decodeGenerationFromElementState(d_states[index]));


    // We cannot guarantee the full pre-conditions of this function, the
    // preceding assertions are as close as we can get.

    // Mark the pushed cell with the 'FULL' state.

    d_states[index] = encodeElementState(generation, e_FULL);
}


int bcec_AtomicRingBufferIndexManager::reservePopIndex(
                                                      unsigned int *generation,
                                                      unsigned int *index)
{
    BSLS_ASSERT(0 != generation);
    BSLS_ASSERT(0 != index);

    enum Status { e_SUCCESS = 0, e_QUEUE_EMPTY = 1 };

    unsigned int loadedPopIndex = d_popIndex.load();
    unsigned int savedPopIndex  = -1;

    // We use 'savedPopIndex' to ensure we attempt to acquire an index at
    // least twice before returning 'e_QUEUE_EMPTY'.  This is purely a
    // performance adjustment.

    unsigned int currIndex, currGeneration;

    for (;;) {
        currGeneration = loadedPopIndex / d_capacity;
        currIndex      = loadedPopIndex % d_capacity;

        // Attempt to swap this cell's state from e_FULL to 'e_READING'

        const int compare = encodeElementState(currGeneration, e_FULL);
        const int swap    = encodeElementState(currGeneration, e_READING);
        const int was     = d_states[currIndex].testAndSwap(compare, swap);

        if (compare == was) {
            // We've successfully changed the state and thus acquired
            // the index.  Exit the loop.

            *generation = currGeneration;
            *index      = currIndex;

            break;
        }

        // We've failed to reserve the index.  We can use the result of the
        // 'testAndSwap' to determine wehther the queue was empty at the
        // point of the 'testAndSwap'.  Either:
        //
        //: 1 The cell is from the previous generation, meaning that we are
        //:   waiting for poppers from the previous generation (queue is
        //:   empty)
        //:
        //: 2 The cell is from the current generation and empty (queue is
        //:   empty)
        //:
        //: 3 The cell is currently being written to (sleep)
        //:
        //: 4 The pop index has been incremented between the value being
        //:   loaded, and the attempt to test and swap.


        unsigned int elementGeneration = decodeGenerationFromElementState(was);
        ElementState state             = decodeStateFromElementState(was);

        int difference = currGeneration - elementGeneration;

        if (difference == 1 || BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
            difference == -static_cast<int>(d_maxGeneration))) {
            // The cell is from the previous generation, meaning that we have
            // wrapped around and are attempting to reserve a cell that is
            // still being popped from the previous generation.

            BSLS_ASSERT(state == e_READING);
            BSLS_ASSERT(1 == circularDifference(currGeneration,
                                                elementGeneration,
                                                d_maxGeneration + 1));

            return e_QUEUE_EMPTY;                                     // RETURN
        }
        BSLS_ASSERT(0 >= circularDifference(currGeneration,
                                            elementGeneration,
                                            d_maxGeneration+1));

        if (0 == difference && e_EMPTY == state) {
            // The cell is empty in the current generation, meaning the queue
            // is empty.

            if (savedPopIndex != loadedPopIndex) {
                // Make two attempts before returning that the queue is empty.

                bcemt_ThreadUtil::yield();
                savedPopIndex = loadedPopIndex;
                loadedPopIndex = d_popIndex.loadRelaxed();
                continue;
            }
            return e_QUEUE_EMPTY;                                     // RETURN
        }

        if (0 != difference || e_WRITING == state) {
            // The cell is currently being written, or our pop index is very
            // out of date. Delay and try and reserve it again.

            bcemt_ThreadUtil::yield();
            loadedPopIndex = d_popIndex.loadRelaxed();
            continue;
        }

        unsigned int next = nextCombinedIndex(loadedPopIndex);
        loadedPopIndex   = d_popIndex.testAndSwap(loadedPopIndex, next);
    }

    // Attempt to increment the pop index.

    d_popIndex.testAndSwap(loadedPopIndex,
                           nextCombinedIndex(loadedPopIndex));

    return 0;
}

void bcec_AtomicRingBufferIndexManager::commitPopIndex(
                                                    unsigned int generation,
                                                    unsigned int index)
{
    BSLS_ASSERT(generation <= d_maxGeneration);
    BSLS_ASSERT(index      <  d_capacity);
    BSLS_ASSERT(e_READING  == decodeStateFromElementState(d_states[index]));
    BSLS_ASSERT(generation ==
                decodeGenerationFromElementState(d_states[index]));

    // We cannot guarantee the full preconditions of this function, the
    // preceding assertions are as close as we can get.

    // Mark the popped cell with the subsequent generation and the EMPTY
    // state.

    d_states[index] = encodeElementState(nextGeneration(generation),
                                         e_EMPTY);
}


void bcec_AtomicRingBufferIndexManager::disable()
{

    // Loop until we detect the disabled bit in the push index has been set.

    for (;;) {
        const unsigned int pushIndex = d_pushIndex;

        if (isDisabledFlagSet(pushIndex)) {
            // The queue is already disabled.

            return;                                                   // RETURN
        }

        if (pushIndex == static_cast<unsigned int>(
                d_pushIndex.testAndSwap(pushIndex,
                                        pushIndex | e_DISABLED_STATE_MASK))) {
            // The queue has been successfully disabled.

            break;
        }
    }
}

void bcec_AtomicRingBufferIndexManager::enable()
{

    // Loop until we detect the disabled bit in the push index has been unset.

    for (;;) {
        const unsigned int pushIndex = d_pushIndex;

        if (!isDisabledFlagSet(pushIndex)) {
            // The queue is already enabled.

            return;                                                   // RETURN
        }

        if (pushIndex == static_cast<unsigned int>(
                d_pushIndex.testAndSwap(pushIndex,
                                        pushIndex & ~e_DISABLED_STATE_MASK))) {
            // The queue has been successfully enabled.

            break;
        }
    }
}

int bcec_AtomicRingBufferIndexManager::reservePopIndexForClear (
                                              unsigned int *disposedGeneration,
                                              unsigned int *disposedIndex,
                                              unsigned int  endGeneration,
                                              unsigned int  endIndex)
{
    BSLS_ASSERT(disposedGeneration);
    BSLS_ASSERT(disposedIndex);
    BSLS_ASSERT(endGeneration <= d_maxGeneration);
    BSLS_ASSERT(endIndex      <  d_capacity);

    enum { e_SUCCESS = 0, e_FAILURE = -1 };

    unsigned int loadedCombinedIndex = d_popIndex.loadRelaxed();
    for (;;) {
        unsigned int endCombinedIndex = endGeneration * d_capacity + endIndex;

        if (0 == circularDifference(endCombinedIndex,
                                    loadedCombinedIndex,
                                    d_maxCombinedIndex + 1)) {
            return e_FAILURE;                                         // RETURN
        }
        // We should not be able to get beyond the 'endGeneration' and
        // 'endIndex' if the preconditions of this function (that it holds a
        // write reservation) are met.

        BSLS_ASSERT(0 < circularDifference(endCombinedIndex,
                                           loadedCombinedIndex,
                                           d_maxCombinedIndex + 1));

        unsigned int currentIndex      = loadedCombinedIndex % d_capacity;
        unsigned int currentGeneration = loadedCombinedIndex / d_capacity;


        // Attempt to swap this cell's state from e_FULL to 'e_READING', note
        // that we set this to 'e_EMPTY' only after we attempt to increment
        // the popIndex, so that another popping thread will not accidentally
        // see this cell as empty and return that the queue is empty.

        const int compare = encodeElementState(currentGeneration, e_FULL);
        const int swap    = encodeElementState(currentGeneration, e_READING);
        const int was     = d_states[currentIndex].testAndSwap(compare, swap);

        if (compare == was) {
            // We've successfully disposed of this index.

            *disposedGeneration = currentGeneration;
            *disposedIndex      = currentIndex;
            break;
        }

        int state = decodeStateFromElementState(was);

        if (e_WRITING == state || e_FULL == state) {
            // Another thread is currently writing to this cell, or this
            // thread has been asleep for an entire generation.  Re-load
            // the pop index and return to the top of the loop.

            bcemt_ThreadUtil::yield();
            loadedCombinedIndex = d_popIndex.loadRelaxed();
            continue;
        }
        unsigned int next   = nextCombinedIndex(loadedCombinedIndex);
        loadedCombinedIndex = d_popIndex.testAndSwap(loadedCombinedIndex,
                                                     next);
    }

    // Attempt to increment the operation index.

    d_popIndex.testAndSwap(loadedCombinedIndex,
                           nextCombinedIndex(loadedCombinedIndex));

    return e_SUCCESS;
}

void bcec_AtomicRingBufferIndexManager::abortPushIndexReservation(
                                                       unsigned int generation,
                                                       unsigned int index)
{
    BSLS_ASSERT(generation <= d_maxGeneration);
    BSLS_ASSERT(index      <  d_capacity);
    BSLS_ASSERT(static_cast<unsigned int>(d_popIndex.loadRelaxed()) ==
                generation * d_capacity + index);
    BSLS_ASSERT(e_WRITING  == decodeStateFromElementState(d_states[index]));
    BSLS_ASSERT(generation ==
                decodeGenerationFromElementState(d_states[index]));


    unsigned int loadedPopIndex = d_popIndex.loadRelaxed();

    // Note that the preconditions for this function -- that (1) the current
    // thread hold a push-index reservation on 'generation' and 'index', and
    // (2) have called 'clearPopIndex' on all the preceding generation and
    // index values -- require that 'd_popIndex' refer to 'generation' and
    // 'index.


    BSLS_ASSERT(generation == loadedPopIndex / d_capacity);
    BSLS_ASSERT(index      == loadedPopIndex % d_capacity);


    // Marking the cell first for reading in the current generation, and then
    // after incrementing the pop index, marking it empty for the subsequent
    // generation matches the states when popping the cell.  Although the
    // intermediate 'e_READING' state is not strictly necessary, it reduces
    // the set of states that 'reservePopIndex' may encounter.

    d_states[index] = encodeElementState(generation, e_READING);


    unsigned int nextIndex = nextCombinedIndex(loadedPopIndex);
    d_popIndex.testAndSwap(loadedPopIndex, nextIndex);

    d_states[index] = encodeElementState(nextGeneration(generation), e_EMPTY);


}

// ACCESSORS
unsigned int bcec_AtomicRingBufferIndexManager::length() const
{
    // Note that 'bcec_AtomicRingBuffer::pushBack' and
    // 'bcec_AtomicRingBuffer::popFront' rely on the fact that the
    // following atomic loads are sequentially consistent.  If this were to
    // change, 'bcec_AtomicRingBuffer::tryPushBack' and
    // 'bcec_AtomicRingBuffer::tryPopFront' would need to be modified.

    unsigned int combinedPushIndex = discardDisabledFlag(d_pushIndex);
    unsigned int combinedPopIndex  = d_popIndex;

    // Note that the following is logically equivalent to:
    //..
    // int difference = circularDifference(combinedPushIndex,
    //                                     combinedPopIndex,
    //                                     d_maxCombinedIndex + 1);
    //
    // if      (difference <  0)          { return 0; }
    // else if (difference >= d_capacity) { return d_capacity; }
    // return difference;
    //..
    // However we can perform some minor optimization knowing that
    // 'combinedPushIndex' was loaded (with sequential consistency) *before*
    // 'combinedPopIndex' so it is not possible for the 'difference' to be
    // greater than 'd_capacity' unless 'combinedPopIndex' has wrapped around
    // 'd_maxCombinedIndex' and the length is 0.

    int difference = combinedPushIndex - combinedPopIndex;
    if (difference >= 0) {
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                difference > static_cast<int>(d_capacity))) {
            // Because the pop index is acquired after the push index, its
            // possible for the push index to be immediately before
            // 'd_maxCombinedIndex' and then 'combinedPopIndex' to be acquired
            // after it wraps around to 0, resulting in a very large positive
            // value.

            BSLS_ASSERT(0 > circularDifference(combinedPushIndex,
                                               combinedPopIndex,
                                               d_maxCombinedIndex + 1));

            return 0;                                                 // RETURN
        }
        return static_cast<unsigned int>(difference);                 // RETURN
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
            difference < -static_cast<int>(d_maxCombinedIndex / 2))) {
        BSLS_ASSERT(0 < circularDifference(combinedPushIndex,
                                           combinedPopIndex,
                                           d_maxCombinedIndex + 1));

        difference += d_maxCombinedIndex + 1;
        return bsl::min(static_cast<unsigned int>(difference), d_capacity);
                                                                      // RETURN
    }
    return 0;
}


bool bcec_AtomicRingBufferIndexManager::isEnabled() const
{
    return !isDisabledFlagSet(d_pushIndex.load());
}


bsl::ostream& bcec_AtomicRingBufferIndexManager::print(
                                                   bsl::ostream& stream) const
{
    const unsigned int pushIndex = discardDisabledFlag(d_pushIndex);
    const unsigned int popIndex  = discardDisabledFlag(d_popIndex);


    stream << bsl::endl
           << "        capacity: " << capacity()         << bsl::endl
           << "         enabled: " << isEnabled()        << bsl::endl
           << "   maxGeneration: " << d_maxGeneration    << bsl::endl
           << "maxCombinedIndex: " << d_maxCombinedIndex << bsl::endl
           << "  pushGeneration: " << pushIndex / capacity() << bsl::endl
           << "       pushIndex: " << pushIndex % capacity() << bsl::endl
           << "   popGeneration: " << popIndex / capacity()  << bsl::endl
           << "        popIndex: " << popIndex % capacity()  << bsl::endl;

    const unsigned int popIdx  = d_popIndex % d_capacity;
    const unsigned int pushIdx = discardDisabledFlag(d_pushIndex) % d_capacity;

    for (unsigned int i = 0; i < capacity(); ++i) {
        unsigned int state = d_states[i];

        unsigned int  generation = decodeGenerationFromElementState(state);
        const char  *stateName  = toString(decodeStateFromElementState(state));
        stream << bsl::right << bsl::setw(8) << i << ": { "
               << bsl::left  << bsl::setw(3) << generation << " | "
               << bsl::left  << bsl::setw(7) << stateName << " }";

        if (pushIdx == i && popIdx == i) {
            stream << " <-- push & pop";
        }
        else if (pushIdx == i) {
            stream << " <-- push";
        }
        else if (popIdx == i) {
            stream << " <-- pop";
        }
        stream << bsl::endl;
    }
    return stream;
}


}  // close namespace BloombergLP

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2013
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
