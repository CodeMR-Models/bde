// baesu_stacktracetestallocator.cpp                                  -*-C++-*-
#include <baesu_stacktracetestallocator.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#include <baesu_stackaddressutil.h>
#include <baesu_stacktraceutil.h>

#include <bcemt_lockguard.h>

#include <bslma_mallocfreeallocator.h>
#include <bsls_alignmentutil.h>
#include <bsls_assert.h>
#include <bsls_types.h>

#include <bsl_algorithm.h>
#include <bsl_cstdlib.h>
#include <bsl_map.h>
#include <bsl_vector.h>
#include <bsl_utility.h>

namespace {

typedef BloombergLP::baesu_StackAddressUtil AddressUtil;

enum {
    IGNORE_FRAMES = AddressUtil::BAESU_IGNORE_FRAMES,
        // On some platforms, gathering the stack pointers wastes one frame
        // gathering the address of 'AddressUtil::getStackAddresses', which is
        // reflected in whether 'AddressUtil::BAESU_IGNORE_FRAMES' is 0 or 1.
        // This constant allows us to adjust for this and not display that
        // frame as it is of no interest to the user.

    MAX_ALIGNMENT = BloombergLP::bsls::AlignmentUtil::BSLS_MAX_ALIGNMENT
};

typedef BloombergLP::bsls::Types::UintPtr UintPtr;
typedef BloombergLP::bslma::Allocator::size_type size_type;

template <int A, int B>
struct Max {
    // Calculate, at compile time, the maximum of two ints

    enum { VALUE = A > B ? A : B };
};

}  // close unnamed namespace

// Magic number.  I wanted to make this into an enum, but I also want it to be
// the size of a pointer, and AIX does not support 64 bit enums.

#ifdef BSLS_PLATFORM_CPU_64_BIT
static
const UintPtr HIGH_ONES = (UintPtr) 1111111111 * 10 * 1000 * 1000 * 1000;
#else
static
const UintPtr HIGH_ONES = 0;
#endif

static const UintPtr UNFREED_BLOCK_MAGIC = 1222222221 + HIGH_ONES;
static const UintPtr FREED_BLOCK_MAGIC   = 1999999991 + HIGH_ONES;

                            // ----------------
                            // static functions
                            // ----------------

static
int getTraceBufferLength(int  specifiedMaxRecordedFrames)
    // Specify 'specifiedMaxRecordedFrames', the value passed to the
    // 'baesu_StackTraceTestAllocator' constructor, and a pointer to
    // 'd_traceBufferLength'.  Assign a value to 'maxRecordedFrames', the
    //  trace buffer, which may have to
    // be padded for alignment purposes.  Return the necessary number of frames
    // to saves, which may be greater than 'specifiedMaxRecordedFrames' due to
    // ignored frames.
{
    enum { PTRS_PER_MAX = MAX_ALIGNMENT / sizeof(void *) };

    BSLMF_ASSERT(0 == MAX_ALIGNMENT % sizeof(void *));

    int ret = ((specifiedMaxRecordedFrames + IGNORE_FRAMES + PTRS_PER_MAX - 1)
                                                / PTRS_PER_MAX) * PTRS_PER_MAX;

    BSLS_ASSERT(ret >= specifiedMaxRecordedFrames);
    BSLS_ASSERT(0 == ret % PTRS_PER_MAX);

    return ret;
}

namespace BloombergLP {

                 // ==========================================
                 // baesu_StackTraceTestAllocator::BlockHeader
                 // ==========================================

struct baesu_StackTraceTestAllocator::BlockHeader {
    // A record of this type is stored in each block, after the stack pointers
    // and immediate before the user area of memory in the block.  These
    // 'BlockHeader' objects form a doubly linked list consisting of all blocks
    // which are unfreed.
    //
    // Note that the 'd_magic' and 'd_allocator_p' fields are at the end of the
    // 'BlockHeader', putting them adjacent to the client's area of memory,
    // making them the most likely fields to be corrupted.  A corrupted
    // 'd_allocator_p' or especially a corrupted 'd_magic' are much more likely
    // to be properly diagnosed by the allocator with a meaningful error
    // message and no segfault than a corrupted 'd_next_p' or 'd_prevNext_p'.

    // DATA
    BlockHeader                   *d_next_p;      // next object in the
                                                  // doubly-linked list

    BlockHeader                  **d_prevNext_p;  // pointer to the 'd_next_p'
                                                  // field of the previous
                                                  // object, or the head ptr of
                                                  // the linked list if there
                                                  // is no previous object.

    baesu_StackTraceTestAllocator *d_allocator_p; // creator of block

    UintPtr                        d_magic;       // Magic number -- has
                                                  // different values for
                                                  // an unfreed block, a freed
                                                  // block, or the head node.

    // CREATOR
    BlockHeader(BlockHeader                   *next,
                BlockHeader                  **prevNext,
                baesu_StackTraceTestAllocator *stackTraceTestAllocator,
                UintPtr                        magic);
        // Create a block hear, populating the fields with the specified
        // 'next', 'prevNext', 'stackTraceTestAllocator', and 'magic'
        // arguments.
};


// CREATORS
inline
baesu_StackTraceTestAllocator::BlockHeader::BlockHeader(
                        BlockHeader                   *next,
                        BlockHeader                  **prevNext,
                        baesu_StackTraceTestAllocator *stackTraceTestAllocator,
                        UintPtr                        magic)
: d_next_p(next)
, d_prevNext_p(prevNext)
, d_allocator_p(stackTraceTestAllocator)
, d_magic(magic)
{
    BSLMF_ASSERT(sizeof(BlockHeader) == 4 * sizeof(void *));
}

                        // -----------------------------
                        // baesu_StackTraceTestAllocator
                        // -----------------------------

// PRIVATE ACCESSORS
int baesu_StackTraceTestAllocator::preDeallocateCheckBlockHeader(
                                         const BlockHeader *blockHdr) const
{
    int rc = 0;

    if (UNFREED_BLOCK_MAGIC != blockHdr->d_magic) {
        if (FREED_BLOCK_MAGIC == blockHdr->d_magic) {
            // Note: this write may allocate the freed block, trashing
            // '*blockHdr'.

            *d_ostream << "Error: block at " << &blockHdr[1]
                       << " freed second time by allocator '" << d_name
                       << "'\n";
        }
        else {
            *d_ostream << "Error: corrupted block at "
                       << &blockHdr[1] << " attempted to be freed by"
                       << " allocator '" << d_name << "'\n";
        }

        return -1;                                                    // RETURN
    }

    if (this != blockHdr->d_allocator_p) {
        baesu_StackTraceTestAllocator *otherAlloc = blockHdr->d_allocator_p;
        bool notOurs = STACK_TRACE_TEST_ALLOCATOR_MAGIC != otherAlloc->d_magic;
        *d_ostream << "Error: attempt to free block by wrong"
                   << " allocator.\n    Block belongs to allocator '"
                   << (notOurs ? "<<Not a baesu_StackTraceTestAllocator>>"
                               : otherAlloc->d_name)
                   << "'\n    Attempted to free by allocator '"
                   << d_name << "'\n";

        if (notOurs) {
            return -1;                                                // RETURN
        }

        rc = -1;
    }

    if (0 == d_blocks || 0 == *blockHdr->d_prevNext_p) {
        *d_ostream << "Error: block at " << &blockHdr[1]
                   << " corrupted: null list ptr(s)\n";

        return -1;                                                    // RETURN
    }

    // check out next node

    const BlockHeader *next = blockHdr->d_next_p;
    if (next) {
        if (UNFREED_BLOCK_MAGIC != next->d_magic) {
            if (FREED_BLOCK_MAGIC == next->d_magic) {
                *d_ostream << "Error: freed object on unfreed block list"
                           << " of allocator '" << d_name << "' at "
                           << next << bsl::endl;
            }
            else {
                *d_ostream << "Error: block list of allocator '"
                           << d_name << "' corrupted, bad magic number: "
                           << next->d_magic << bsl::endl;
            }

            rc = -1;
        }
    }

    return rc;
}

// CLASS METHODS
void baesu_StackTraceTestAllocator::failureHandlerAbort()
{
    bsl::abort();
}

void baesu_StackTraceTestAllocator::failureHandlerNoop()
{
    ;  // do nothing
}

// CREATORS
baesu_StackTraceTestAllocator::baesu_StackTraceTestAllocator(
                                     bsl::ostream     *ostream,
                                     int               numRecordedFrames,
                                     bool              demanglingPreferredFlag,
                                     bslma::Allocator *basicAllocator)
: d_magic(STACK_TRACE_TEST_ALLOCATOR_MAGIC)
, d_numBlocksInUse(0)
, d_blocks(0)
, d_mutex()
, d_name("<unnamed>")
, d_failureHandler(&failureHandlerAbort)
, d_maxRecordedFrames(numRecordedFrames + IGNORE_FRAMES)
, d_traceBufferLength(getTraceBufferLength(numRecordedFrames))
, d_ostream(ostream)
, d_demangleFlag(demanglingPreferredFlag)
, d_allocator_p(basicAllocator ? basicAllocator
                               : &bslma::MallocFreeAllocator::singleton())
{
    BSLMF_ASSERT(0 == sizeof(BlockHeader) % MAX_ALIGNMENT);

    BSLS_ASSERT(numRecordedFrames >= 2);

    BSLS_ASSERT(d_maxRecordedFrames >= numRecordedFrames);
    BSLS_ASSERT(d_traceBufferLength >= d_maxRecordedFrames);
}

baesu_StackTraceTestAllocator::baesu_StackTraceTestAllocator(
                                     const char       *name,
                                     bsl::ostream     *ostream,
                                     int               numRecordedFrames,
                                     bool              demanglingPreferredFlag,
                                     bslma::Allocator *basicAllocator)
: d_magic(STACK_TRACE_TEST_ALLOCATOR_MAGIC)
, d_numBlocksInUse(0)
, d_blocks(0)
, d_mutex()
, d_name(name ? name : "<unnamed>")
, d_failureHandler(&failureHandlerAbort)
, d_maxRecordedFrames(numRecordedFrames + IGNORE_FRAMES)
, d_traceBufferLength(getTraceBufferLength(numRecordedFrames))
, d_ostream(ostream)
, d_demangleFlag(demanglingPreferredFlag)
, d_allocator_p(basicAllocator ? basicAllocator
                               : &bslma::MallocFreeAllocator::singleton())
{
    BSLMF_ASSERT(0 == sizeof(BlockHeader) % MAX_ALIGNMENT);

    BSLS_ASSERT(numRecordedFrames >= 2);

    BSLS_ASSERT(d_maxRecordedFrames >= numRecordedFrames);
    BSLS_ASSERT(d_traceBufferLength >= d_maxRecordedFrames);
}

baesu_StackTraceTestAllocator::~baesu_StackTraceTestAllocator()
{
    if (numBlocksInUse() > 0) {
        *d_ostream << "======================================================="
                   << "========================\nError: memory leaked:\n";

        reportBlocksInUse();

        (*d_failureHandler)();

        release();
    }
}

void *baesu_StackTraceTestAllocator::allocate(size_type size)
{
    if (0 == size) {
        return 0;                                                     // RETURN
    }

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    // The underlying allocator might align the block differently depending on
    // the size passed.  The alignment must be large enough to accomodate the
    // stack addresses (type 'void *') in the buffer, it must be large enough
    // to accomodate the 'BlockHeader's alignment requirements, and it must be
    // large enough to accomodate whatever the alignment requirements of
    // whatever the client intends to store in their portion of the block.  We
    // can infer the requirements of our pointers and block header at compile
    // time in 'FIXED_ALIGN', then we infer the alignment requirement of the
    // client's section from the size passed, and take the maximum of the two
    // to get the alignment required.  We then round the size we will pass to
    // the underlying allocator up to a multiple of 'align', so that the
    // underlying allocator cannot infer a lower value of the alignment
    // requirement.

    enum { FIXED_ALIGN =
                   Max<bsls::AlignmentFromType<void *>::VALUE,
                       bsls::AlignmentFromType<BlockHeader>::VALUE>::VALUE };
    const int align = bsl::max<int>(
                        FIXED_ALIGN,
                        bsls::AlignmentUtil::calculateAlignmentFromSize(size));
    const int lowBits = align - 1;
    BSLS_ASSERT(0 == (align & lowBits));    // verify 'align' is power of 2
    size = (size + lowBits) & ~lowBits;     // round 'size' up to multiple of
                                            // 'align'

    void **framesBegin = (void **) d_allocator_p->allocate(
          d_traceBufferLength * sizeof(void *) + sizeof(BlockHeader) + size);

    BlockHeader *blockHdr = (BlockHeader*) (framesBegin + d_traceBufferLength);

    new (blockHdr) BlockHeader(d_blocks,
                               &d_blocks,
                               this,
                               UNFREED_BLOCK_MAGIC);
    if (d_blocks) {
        d_blocks->d_prevNext_p = &blockHdr->d_next_p;
    }
    d_blocks = blockHdr;

    bsl::fill(framesBegin, framesBegin + d_maxRecordedFrames, (void *) 0);
    AddressUtil::getStackAddresses(framesBegin,
                                   d_maxRecordedFrames);

    void *ret = blockHdr + 1;

    BSLS_ASSERT(0 == ((UintPtr) ret & ((sizeof(void *) - 1) | lowBits)));

    ++d_numBlocksInUse;

    return ret;
}

void baesu_StackTraceTestAllocator::deallocate(void *address)
{
    if (!address) {
        return;                                                       // RETURN
    }

    if (0 != ((UintPtr) address & (sizeof(void *) - 1))) {
        // Badly aligned, can't be a block we allocated.

        *d_ostream << "Badly aligned block passed to allocator '"
                   << d_name << "' must have been allocated by another type"
                   << " of allocator\n";
        (*d_failureHandler)();

        return;                                                       // RETURN
    }

    BlockHeader *blockHdr = (BlockHeader *) address - 1;

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    if (preDeallocateCheckBlockHeader(blockHdr)) {
        guard.release()->unlock();
        (*d_failureHandler)();

        return;                                                       // RETURN
    }

    if (blockHdr->d_next_p) {
        blockHdr->d_next_p->d_prevNext_p = blockHdr->d_prevNext_p;
    }
    *blockHdr->d_prevNext_p = blockHdr->d_next_p;
    blockHdr->d_magic = FREED_BLOCK_MAGIC;

    d_allocator_p->deallocate((void **) blockHdr - d_traceBufferLength);

    --d_numBlocksInUse;
    BSLS_ASSERT(d_numBlocksInUse >= 0);
}

void baesu_StackTraceTestAllocator::release()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    for (BlockHeader *blockHdr = d_blocks;
                                     blockHdr; blockHdr = blockHdr->d_next_p) {
        if (preDeallocateCheckBlockHeader(blockHdr)) {
            guard.release()->unlock();
            (*d_failureHandler)();

            return;                                                   // RETURN
        }
    }

    int numBlocks = 0;
    for (BlockHeader *blockHdr = d_blocks; blockHdr; ) {
        ++numBlocks;

        BlockHeader *condemned = blockHdr;
        blockHdr = blockHdr->d_next_p;

        d_allocator_p->deallocate((void **) condemned - d_traceBufferLength);
    }

    d_blocks = 0;

    BSLS_ASSERT(numBlocks == d_numBlocksInUse);
    d_numBlocksInUse = 0;
}

void baesu_StackTraceTestAllocator::setFailureHandler(FailureHandler func)
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    BSLS_ASSERT(0 != func);

    d_failureHandler = func;
}

// ACCESSORS
baesu_StackTraceTestAllocator::FailureHandler
baesu_StackTraceTestAllocator::failureHandler() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    return d_failureHandler;
}

bsl::size_t baesu_StackTraceTestAllocator::numBlocksInUse() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    return d_numBlocksInUse;
}

void baesu_StackTraceTestAllocator::reportBlocksInUse(
                                                   bsl::ostream *ostream) const
{
    typedef bsl::vector<const void *>    StackTraceVec;
    typedef bsl::map<StackTraceVec, int> StackTraceVecMap;

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    if (0 == ostream) {
        ostream = d_ostream;
    }

    if (0 == d_numBlocksInUse) {
        return;                                                       // RETURN
    }

    *ostream << d_numBlocksInUse << " block(s) in allocator '"
             << d_name << "' in use.\n";

    StackTraceVecMap stackTraceVecMap(d_allocator_p);
    StackTraceVec traceVec(d_allocator_p);

    int numBlocksInUse = 0;
    for (BlockHeader *blockHdr = d_blocks; blockHdr;
                                               blockHdr = blockHdr->d_next_p) {
        if (!blockHdr || UNFREED_BLOCK_MAGIC != blockHdr->d_magic) {
            if (blockHdr && FREED_BLOCK_MAGIC != blockHdr->d_magic) {
                *ostream << "baesu_StackTraceTestAllocator: freed block at "
                         << (blockHdr + 1) << " in alloced list\n";
            }
            else {
                *ostream << "baesu_StackTraceTestAllocator: memory corruption"
                         << " detected, possible buffer underrun\n";

                // We don't want to go on traversing
                // the list because we don't even know that 'blockHdr' points
                // at a 'blockHdr', so we can't trust 'blockHdr->d_next_p'.

                break;
            }
        }

        void **startTrace = (void **) blockHdr - d_traceBufferLength;
        void **endTrace   = startTrace + d_maxRecordedFrames;
        startTrace += IGNORE_FRAMES;

        // trim off the zeroes at the end of the trace

        while (endTrace > startTrace && 0 == endTrace[-1]) {
            --endTrace;
        }

        traceVec.clear();
        traceVec.insert(traceVec.end(), startTrace, endTrace);
#if 0
        // This would be a better way to do it, but 'bsl::map' uses the
        // default allocator to implement 'operator[]'.  We want to avoid using
        // the default allocator in a test allocator since the client may be
        // debugging memory issues, and may want to closely monitor traffic on
        // the default allocator.  What's more, *THIS* *ALLOCATOR* may be the
        // default allocator, which would cause a mutex deadlock.

        ++stackTraceVecMap[traceVec];
#else
        StackTraceVecMap::iterator stmit = stackTraceVecMap.find(traceVec);
        if (stackTraceVecMap.end() == stmit) {
            StackTraceVecMap::value_type pr(traceVec, 1, d_allocator_p);
            stackTraceVecMap.insert(pr);
        }
        else {
            ++stmit->second;
        }
#endif

        ++numBlocksInUse;
    }
    BSLS_ASSERT(d_numBlocksInUse == numBlocksInUse);

    *ostream << "Block(s) allocated from "
             << stackTraceVecMap.size() << " trace(s).\n";

    int place = 0;
    baesu_StackTrace st(d_allocator_p);
    const StackTraceVecMap::iterator mapend = stackTraceVecMap.end();
    for (StackTraceVecMap::iterator it = stackTraceVecMap.begin();
                                                      mapend != it; ++it) {
        *ostream << "------------------------------------------"
                 << "-------------------------------------\n"
                 << "Allocation trace " << ++place << ", "
                 << it->second << " block(s) in use.\n"
                 << "Stack trace at allocation time:\n";

        int rc = baesu_StackTraceUtil::loadStackTraceFromAddressArray(
                                                       &st,
                                                       it->first.begin(),
                                                       (int) it->first.size(),
                                                       d_demangleFlag);
        if (rc || 0 == st.length()) {
            *ostream << "... stack trace failed ...\n";
        }
        else {
            baesu_StackTraceUtil::printFormatted(*ostream, st);
        }
        st.removeAll();
    }
}

}  // close enterprise namespace

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
