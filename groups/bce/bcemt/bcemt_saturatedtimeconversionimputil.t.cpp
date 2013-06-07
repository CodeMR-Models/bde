// bcemt_saturatedtimeconversionimputil.t.cpp                         -*-C++-*-

#include <bcemt_saturatedtimeconversionimputil.h>

#include <bsls_bsltestutil.h>

#include <bsl_cstdlib.h>
#include <bsl_iostream.h>
#include <bsl_limits.h>
#include <bsl_string.h>

#ifdef BCES_PLATFORM_WIN32_THREADS
#include <windows.h>
#endif

using namespace BloombergLP;
using bsl::cout;
using bsl::cerr;
using bsl::endl;
using bsl::flush;

//=============================================================================
//                                  TEST PLAN
//-----------------------------------------------------------------------------
//                                  Overview
//                                  --------
// The component consists of a number of independent class methods which may
// be tested independently -- none depend on each other.
//-----------------------------------------------------------------------------
// CLASS METHODS
// [ 6] USAGE
// [ 5] toMillisec(unsigned int  *, const bdet_TimeInterval&);
// [ 5] toMillisec(unsigned long *, const bdet_TimeInterval&);
// [ 4] toTimeT(bsl::time_t *, const bsls::Types::Int64);
// [ 3] toTimeSpec(TIMESPEC *, const bdet_TimeInterval&); -- tv_sec unsigned.
// [ 2] toTimeSpec(TIMESPEC *, const bdet_TimeInterval&); -- tv_sec signed.
// [ 1] Breathing test
//-----------------------------------------------------------------------------

//=============================================================================
//                    STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}
# define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

}  // close unnamed namespace

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------

#define LOOP_ASSERT(I,X) { \
    if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__);}}

#define LOOP2_ASSERT(I,J,X) { \
    if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\t" << #L << ": " << L <<  "\n";     \
              aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\t" << #L << ": " << L << "\t" <<    \
              #M << ": " << M <<  "\n";                                   \
              aSsErT(1, #X, __LINE__); } }

#define LOOP0_ASSERT    ASSERT
#define LOOP1_ASSERT    LOOP_ASSERT

//=============================================================================
//           BSLS_BSLTESTUTIL -- LIKE MACROS
// The 'BSLS_BSLTESTUTIL_EXPAND' macro is required to workaround a
// pre-processor issue on windows that prevents __VA_ARGS__ to be expanded in
// the definition of 'BSLS_BSLTESTUTIL_NUM_ARGS'
//-----------------------------------------------------------------------------

#define EXPAND(X)                                            \
    X

#define NUM_ARGS_IMPL(X5, X4, X3, X2, X1, X0, N, ...)        \
    N

#define NUM_ARGS(...)                                        \
    EXPAND(NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0, ""))

#define LOOPN_ASSERT_IMPL(N, ...)                            \
    EXPAND(LOOP ## N ## _ASSERT(__VA_ARGS__))

#define LOOPN_ASSERT(N, ...)                                 \
    LOOPN_ASSERT_IMPL(N, __VA_ARGS__)

#define ASSERTV(...)                                         \
    LOOPN_ASSERT(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------

#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number
#define T_ cout << "\t" << flush;             // Print a tab (w/o newline)

//=============================================================================
//                               GLOBAL TYPEDEF
//-----------------------------------------------------------------------------

typedef bcemt_SaturatedTimeConversionImpUtil Obj;
typedef bsls::Types::Int64                   Int64;
typedef bsls::Types::Uint64                  Uint64;

//=============================================================================
//                               GLOBAL CONSTANTS
//-----------------------------------------------------------------------------

const Int64 intMax      = bsl::numeric_limits<int>::max();
const Int64 intMin      = bsl::numeric_limits<int>::min();
const Int64 int64Max    = bsl::numeric_limits<Int64>::max();
const Int64 int64Min    = bsl::numeric_limits<Int64>::min();
const Int64 uintMax     = bsl::numeric_limits<unsigned>::max();
const Int64 i16         = 1 << 16;
const Int64 i32         = ((Int64) 1 << 32);
const Int64 i48         = ((Int64) 1 << 48);

//=============================================================================
//                               GLOBAL VARIABLES
//-----------------------------------------------------------------------------

int verbose;
int veryVerbose;
// int veryVeryVerbose;

//=============================================================================
//                            ASSERTS ABOUT TYPES
//-----------------------------------------------------------------------------

#ifdef BCEMT_SATURATEDTIMECONVERSION_LONG_IS_64_BIT
BSLMF_ASSERT(sizeof(long) == sizeof(bsls::Types::Int64));
#else
BSLMF_ASSERT(sizeof(long) == sizeof(int));
#endif

BSLMF_ASSERT(sizeof(long long) == sizeof(bsls::Types::Int64));
BSLMF_ASSERT(sizeof(int)       <  sizeof(bsls::Types::Int64));

#ifdef BCES_PLATFORM_WIN32_THREADS

                        // * asserts about 'DWORD'

BSLMF_ASSERT((bsl::is_same<DWORD, unsigned long>::value));
BSLMF_ASSERT(sizeof(DWORD) == sizeof(int));     // 'long' is 4 bytes on
                                                // windows, 32 or 64 bit
BSLMF_ASSERT((DWORD) -1 > 0);

#endif

//=============================================================================
//                              STATIC FUNCTIONS
//-----------------------------------------------------------------------------

static int sign(Int64 value)
{
    return value < 0 ? -1 : 1;
}

                              // --------------
                              // timespec tests
                              // --------------

// These tests are templated to allow easy repetition for testing of both
// 'Obj::TimeSpec' and 'mach_timespec_t'.

                             // 'tv_sec' is signed

template <typename TIMESPEC>
void testSignedTimespec(const char *timeSpecName)
    // Check if the specified 'TIMESPEC' type is signed, and if so, test it
    // accordingly.
{
    enum { MILLION = 1000 * 1000,
           BILLION = MILLION * 1000 };

    TIMESPEC tm;

    tm.tv_sec = -1;
    if (tm.tv_sec > 0) {
        if (verbose) {
            bsl::string outStr(bsl::string("'") + timeSpecName +
                               "::tv_sec' IS UNSIGNED -- NO SIGNED TESTING\n");
            outStr.append(outStr.length() - 1, '=');
            cout << outStr << endl;
        }
        return;                                                       // RETURN
    }
    else {
        if (verbose) {
            bsl::string outStr(bsl::string("'") + timeSpecName +
                                           "::tv_sec' IS SIGNED -- TESTING\n");
            outStr.append(outStr.length() - 1, '=');
            cout << outStr << endl;
        }
    }

    if (veryVerbose) P(sizeof(tm.tv_sec));

    int ns, ct;

    if (sizeof(tm.tv_sec) == 4) {
        if (veryVerbose) Q(Vary secondss across non-saturating range);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = intMin; i <= intMax; i += i16, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == ns * sign(i));

            ti.setInterval(i, 0);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == 0);
        }
        ASSERT(ct > 65000);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = intMax; i >= intMin; i -= i16, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERTV(tm.tv_sec, i,   tm.tv_sec  ==  i);
            ASSERTV(tm.tv_nsec, ns, tm.tv_nsec == ns * sign(i));

            ti.setInterval(i, 0);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == 0);
        }
        ASSERT(ct > 65000);

        if (veryVerbose) Q(Vary seconds across positive saturating range);
        ns = 0, ct = 0;
        for (Int64 i = intMax; i < int64Max - i48; i += i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == intMax);
            ASSERT(tm.tv_nsec == (i == intMax ? ns : BILLION - 1));

            ti.setInterval(i, 0);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == intMax);
            ASSERT(tm.tv_nsec == (i == intMax ? 0 : BILLION - 1));
        }
        ASSERT(ct > 32000);

        if (veryVerbose) Q(Vary seconds across negative saturating range);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = intMin; i > int64Min + i48; i -= i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == intMin);
            ASSERT(tm.tv_nsec == (i == intMin ? ns * sign(i)
                                              : -(BILLION - 1)));

            ti.setInterval(i, 0);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == intMin);
            ASSERT(tm.tv_nsec == (i == intMin ? 0
                                              : -(BILLION - 1)));
        }
        ASSERT(ct > 32000);

        if (veryVerbose) Q(Try min and max values for 'seconds');
        {
            bdet_TimeInterval ti;

            ns = 500 * MILLION;;
            ti.setInterval(int64Max, ns);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == intMax);
            ASSERT(tm.tv_nsec == BILLION - 1);

            ti.setInterval(int64Min, -ns);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == intMin);
            ASSERT(tm.tv_nsec == -(BILLION - 1));
        }
    }
    else {
        ASSERTV(sizeof(tm.tv_sec), sizeof(tm.tv_sec) == 8);

        if (veryVerbose) Q(Vary seconds across full range with no saturation);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = int64Min; i < int64Max - i48; i += i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == i);
            ASSERT(tm.tv_nsec == ns * sign(i));
        }
        ASSERT(ct > 65000);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = int64Max; i > int64Min + i48; i -= i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == ns * sign(i));
        }
        ASSERT(ct > 65000);

        if (veryVerbose) Q(Try min and max values for seconds);
        {
            bdet_TimeInterval ti(int64Max, ns);

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == int64Max);
            ASSERT(tm.tv_nsec == ns);
        }

        {
            bdet_TimeInterval ti(int64Min, -ns);

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == int64Min);
            ASSERT(tm.tv_nsec == -ns);
        }
    }
}

                             // 'tv_sec' is unsigned

template <typename TIMESPEC>
void testUnsignedTimespec(const char *timeSpecName)
    // Check if the specified 'TIMESPEC' type is unsigned, and if so, test it
    // accordingly.
{
    enum { MILLION = 1000 * 1000,
           BILLION = 1000 * MILLION };

    TIMESPEC tm;

    tm.tv_sec = -1;
    if (tm.tv_sec > 0) {
        if (verbose) {
            bsl::string outStr(bsl::string("'") + timeSpecName +
                                         "::tv_sec' IS UNSIGNED -- TESTING\n");
            outStr.append(outStr.length() - 1, '=');
            cout << outStr << endl;
        }
    }
    else {
        if (verbose) {
            bsl::string outStr(bsl::string("'") + timeSpecName +
                               "::tv_sec' IS SIGNED -- NO UNSIGNED TESTING\n");
            outStr.append(outStr.length() - 1, '=');
            cout << outStr << endl;
        }
        return;                                                       // RETURN
    }

    int ns, ct;

    if (veryVerbose) P(sizeof(tm.tv_sec));

    if (sizeof(tm.tv_sec) == 4) {
        if (veryVerbose) Q(Vary across full non-saturating range);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = 0; i <= uintMax; i += i16, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == ns * sign(i));
        }
        ASSERT(ct > 65000);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = uintMax; i >= 0; i -= i16, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == i);
            ASSERT(tm.tv_nsec == ns * sign(i));
        }
        ASSERT(ct > 65000);

        if (veryVerbose) Q(Vary across positive saturating range);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = (Int64) uintMax + 1; i < int64Max - i48;
                                                        i += i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == uintMax);
            ASSERT(tm.tv_nsec == BILLION - 1);
        }
        ASSERT(ct > 32000);

        if (veryVerbose) Q(Vary across negative saturating range);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = -1; i > int64Min + i48; i -= i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec == 0);
            ASSERT(tm.tv_nsec == 0);
        }
        ASSERT(ct > 32000);

        if (veryVerbose) Q(Test exact min and max values of seconds);
        {
            bdet_TimeInterval ti;

            ti.setInterval(int64Max, 310);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == uintMax);
            ASSERT(tm.tv_nsec == BILLION - 1);

            ti.setInterval(int64Min, -237);
            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == 0);
            ASSERT(tm.tv_nsec == 0);
        }
    }
    else {
        ASSERT(sizeof(tm.tv_sec) == 8);

        if (veryVerbose) {
            Q(Test across full range:);
            Q(... saturates on negative and no saturatiion on positive);
        }
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = int64Min; i < int64Max - i48; i += i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            if (i >= 0) {
                ASSERT(tm.tv_sec  == i);
                ASSERT(tm.tv_nsec == ns);
            }
            else {
                ASSERT(tm.tv_sec  == 0);
                ASSERT(tm.tv_nsec == 0);
            }
        }
        ASSERT(ct > 65000);
        ns = 500 * MILLION, ct = 0;
        for (Int64 i = int64Max; i > int64Min + i48; i -= i48, ++ns, ++ct) {
            bdet_TimeInterval ti(i, ns * sign(i));

            Obj::toTimeSpec(&tm, ti);

            if (i >= 0) {
                ASSERT(tm.tv_sec  == i);
                ASSERT(tm.tv_nsec == ns);
            }
            else {
                ASSERT(tm.tv_sec  == 0);
                ASSERT(tm.tv_nsec == 0);
            }
        }
        ASSERT(ct > 65000);

        if (veryVerbose) Q(Try exact max and min second values);
        {
            bdet_TimeInterval ti(int64Max, ns);

            Obj::toTimeSpec(&tm, ti);

            ASSERT((Uint64) tm.tv_sec == (Uint64) int64Max);

            ASSERT(tm.tv_nsec == ns);

            ti.setInterval(int64Min, -ns);

            Obj::toTimeSpec(&tm, ti);

            ASSERT(tm.tv_sec  == 0);
            ASSERT(tm.tv_nsec == 0);
        }
    }
}

//=============================================================================
//                                MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? bsl::atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;
    // veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 7: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //
        // Concerns:
        //: 1 Demonstrate the principle of 'saturation'.
        //
        // Plan:
        //: 1 Assign values from an 'Int64' to an 'unsigned int' using
        //:   'toMillisec', demonstrating that values within range are
        //:   undistorted while values above and below range are saturated.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        // Suppose we need to assign a value held in a 'bdet_TimeInterval' to
        // an 'unsigned int', where the 'unsigned int' is to contain an
        // equilavent time interval expressed in milliseconds.  A
        // 'bdet_TimeInterval' is able to represent intervals that are outside
        // the range of intervals that can be represented by an 'unsigned int'
        // number of milliseconds (e.g., any negative time interval).
        // 'bcemt_SaturatedTimeConversionImpUtil' handles values outside the
        // representable range of the destination type by "saturating", that is
        // values outside the representable range of the destination type will
        // be assigned the maximum or minimum representable value of the
        // destination type (whichever is closer to the source value).

        // First, we define variables of our source ('bdet_TimeInterval') and
        // destination ('unsigned int') types:

        unsigned int destinationInterval;
        bdet_TimeInterval sourceInterval;

        // Then, we try a value that does not require saturation and observe
        // that 'toMillisec' converts it without modification (beyond loss of
        // precision:

        sourceInterval.setInterval(4, 321000000);
        bcemt_SaturatedTimeConversionImpUtil::toMillisec(
                                         &destinationInterval, sourceInterval);
        ASSERT(4321 == destinationInterval);

        // Next, we calculate the maximum value that can be represented in an
        // 'usngined int' number of milliseconds, and verify that converting an
        // equivalent 'bdet_TimeInterval' does not modify the value:

        const unsigned int maxDestinationInterval =
                                      bsl::numeric_limits<unsigned int>::max();
        bdet_TimeInterval maxiumumTimeInterval(
                                maxDestinationInterval / 1000,
                                (maxDestinationInterval % 1000) * 1000 * 1000);
        bcemt_SaturatedTimeConversionImpUtil::toMillisec(
                                   &destinationInterval, maxiumumTimeInterval);
        ASSERT(maxDestinationInterval == destinationInterval);

        // Now, we attempt to convert a value higher than the maximum
        // representable in an 'unsigned int' milliseconds and verify that the
        // resulting value is the maximum representable 'unsigned int' value:

        bdet_TimeInterval aboveMaxInterval = maxiumumTimeInterval +
                                             bdet_TimeInterval(0, 1000 * 1000);
        bcemt_SaturatedTimeConversionImpUtil::toMillisec(
                                       &destinationInterval, aboveMaxInterval);
        ASSERT(maxDestinationInterval == destinationInterval);

        // Finally, we try a value less than 0 and observe the result of the
        // saturated conversion is 0 (the minimum representable value):

        bdet_TimeInterval belowMinimumInterval(-1, 0);
        bcemt_SaturatedTimeConversionImpUtil::toMillisec(
                                   &destinationInterval, belowMinimumInterval);
        ASSERT(0 == destinationInterval);
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // ASSERTS ABOUT 'TimeSpec' and 'mach_timespec_t'
        //
        // Concerns:
        //    Our implementations of 'toTimeSpec' make assumptions about
        //    whether the destination values are signed or unsigned.  Verify
        //    thse assumptions here.
        //
        // Plan:
        //    Assign '-t' to both fields of 'TimeSpec' and verify that the
        //    values are negative, also verify that 'TimeSpec::tv_nsec' can
        //    can handle values to +- one billion, and that
        //    'mach_timespec_t::tv_nsec' can handle up to one billion.
        // --------------------------------------------------------------------

        if (verbose) cout <<
                            "ASSERTS ABOUT 'TimeSpec' and 'mach_timespec_t'\n"
                            "==============================================\n";

        enum { BILLION = 1000 * 1000 * 1000 };

        Obj::TimeSpec ts;

        ts.tv_sec  = -1;
        ASSERT(ts.tv_sec  < 0);
        ts.tv_nsec = -1;
        ASSERT(ts.tv_nsec < 0);

        ts.tv_nsec = -BILLION;
        ASSERT(-BILLION == (Int64) ts.tv_nsec)
        ts.tv_nsec =  BILLION;
        ASSERT( BILLION == (Int64) ts.tv_nsec)

#ifdef BSLS_PLATFORM_OS_DARWIN
        mach_timespec_t mts;

        mts.tv_sec = -1;
        ASSERT(mts.tv_sec > 0);

        mts.tv_nsec = BILLION;
        ASSERT(BILLION == (Int64) mts.tv_nsec)
#endif
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING 'toMillisec'
        //
        // Concerns:
        //: 1 That 'toMilliSec' copies values that are in range without
        //:   distortion.
        //: 2 That for values above the range that can be copied properly,
        //:   '*dst' is set to its max value;
        //: 3 That for values below the range that can be copied properly,
        //:   '*dst' is set to 0 (its min value);
        //
        // Plan:
        //: o Calculate 'maxSec' and 'maxNSec', the 'seconds' and 'nanoSeconds'
        //:   fields that a 'bdet_TimeInterval' will have and map to the exact
        //:   highest value of DWORD (== uintMax) when properly converted
        //:   by 'toMilliSec'.
        //: o Do trials with 'seconds() == maxSex', and 'nanoseconds()' varying
        //:   between the max possible value and 0 by increments of a million,
        //:   observing that for nanonseconds greater than 'maxNSec', the
        //:   result is saturate, and below that it is not.
        //: o Vary input with 'seconds' from 'maxSec - 1000' to 'maxSec + 1000'
        //:   with 'nanoseconds == 0', verifying that for values below
        //:   'maxSec' no saturation occurs, and above 'maxSec' saturation
        //:   does occur.
        //: o Vary input with 'seconds' from 'maxSec - 1000' to 'maxSec + 1000'
        //:   with 'nanoseconds == maxNSec + 1000000', verifying that for
        //:   values below 'maxSec' no saturation occurs, and '>= maxSec'
        //:   saturation does occur.
        //: o Vary the seconds from 0 to 'maxSec + 2000000', incrementing
        //:   nanoseconds gradually, observing that saturation occurs at
        //:   exactly the right times.
        //: o Set nanoseconds to 'maxNSec' and vary seconds from 'maxSec' to
        //:   lower values down to 0, observing that no saturation occurs.
        //: o Set nanoseconds to a negative value and vary seconds from 0 down
        //:   to 'int64Min' by increments of '(1 << 48)', observing that
        //:   saturation always occurs.
        //: o Set nanoseconds to 'maxNSec' and slowly increment it while
        //:   setting seconds to 'maxSec' and incrementing it by '(1 << 48)'
        //:   and observe that saturation always occurs.
        //: o Set seconds to 0 and vary nanoseconds over the range
        //:   '( -BILLION, BILLION )' by increments of a million, observing
        //:   that negative values are saturated and positive values are not.
        //: o Set seconds to the max and min possible values, varying
        //:   nanoseconds over the full possible range by increments of a
        //:   million, and observe that saturation always properly occurs.
        //
        // Testing:
        //   'toMillisec'
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING 'toMilliseec'\n"
                             "=====================\n";

#ifdef BCES_PLATFORM_POSIX_THREADS
        typedef unsigned int DWORD;
#else
        BSLMF_ASSERT((bsl::is_same<DWORD, unsigned long>::value));

#endif

        BSLMF_ASSERT(sizeof(DWORD) == sizeof(unsigned int));

        enum { MILLION = 1000 * 1000,
               BILLION = 1000 * MILLION };

        DWORD dst;

        const Int64 maxSec  = uintMax / 1000;
        const int   maxNSec = (uintMax % 1000) * MILLION;

        ASSERT(uintMax == maxSec * 1000 + maxNSec / MILLION);

        if (veryVerbose) {
            Q(Vary nsec values of input around the top saturating range);
        }
        for (int ns = BILLION - 1; ns >= 0; ns -= MILLION) {
            bdet_TimeInterval ti(maxSec, ns);
            Obj::toMillisec(&dst, ti);

            Int64 expected = ns > maxNSec ? uintMax
                                          : 1000 * maxSec + ns / MILLION;
            ASSERTV(uintMax, maxNSec, ns, dst, expected,
                                                      (Int64) dst == expected);
        }

        if (veryVerbose) {
            Q(Vary sec values of input around the top saturating range);
        }
        for (Int64 i = maxSec - 1000; i < maxSec + 1000; ++i) {
            bdet_TimeInterval ti(i, 0);
            Obj::toMillisec(&dst, ti);

            ASSERT((Int64) dst == (i > maxSec ? uintMax : i * 1000));
        }

        if (veryVerbose) {
            Q(Vary sec values of input around the top saturating range);
            Q(... with nanoseconds above exactly saturating level);
        }
        Int64 nsDiv = maxNSec / MILLION + 1;
        ASSERT(nsDiv < 1000);
        for (Int64 i = maxSec - 1000; i < maxSec + 1000; ++i) {
            bdet_TimeInterval ti(i, maxNSec + MILLION);
            Obj::toMillisec(&dst, ti);

            ASSERT((Int64) dst == (i >= maxSec ? uintMax : i * 1000 + nsDiv));
        }

        if (veryVerbose) {
            Q(Vary sec values from 0 to above saturating by 1000s);
        }
        Int64 stopAt = maxSec + 2 * MILLION;
        int ns = 0;
        for (Int64 i = 0; i < stopAt; i += 1000, ++ns) {
            bdet_TimeInterval ti(i, ns);
            Obj::toMillisec(&dst, ti);

            Int64 expected = i * 1000 + ns / MILLION;
            if (i > maxSec || (i == maxSec && ns > maxNSec)) {
                expected = uintMax;
            }
            ASSERT(expected >= 0 && expected <= uintMax);

            ASSERT((Int64) dst == expected);
        }

        if (veryVerbose) {
            Q(Vary sec values from 0 to above saturating by 1000s);
            Q(... with nsecs at exactly saturating level);
        }
        ns = maxNSec;
        nsDiv = ns / MILLION;
        for (Int64 i = maxSec; i >= 0; i -= 1000) {
            bdet_TimeInterval ti(i, ns);
            Obj::toMillisec(&dst, ti);

            ASSERT(dst == i * 1000 + nsDiv);
        }

        if (veryVerbose) {
            Q(Vary sec values from 0 down to large negative values);
        }
        ns = -MILLION;
        for (Int64 i = 0; i > -i48; i -= i32, --ns) {
            bdet_TimeInterval ti(i, ns);
            Obj::toMillisec(&dst, ti);

            ASSERT(dst == 0);
        }

        if (veryVerbose) {
            Q(Vary sec values from maxSec up to large positive values);
        }
        ns = maxNSec;
        for (Int64 i = maxSec; i < i48; i += i32, ++ns) {
            bdet_TimeInterval ti(i, ns);
            Obj::toMillisec(&dst, ti);

            ASSERT(dst == uintMax);
        }

        if (veryVerbose) {
            Q(Hold sec at Zero and vary nsec across full range);
        }
        for (ns = -BILLION + MILLION; ns < BILLION; ns += MILLION / 4) {
            bdet_TimeInterval ti(0, ns);
            ASSERT(ti.nanoseconds() == ns);

            Obj::toMillisec(&dst, ti);

            if (ns <= 0) {
                ASSERT(0 == dst);
            }
            else {
                ASSERT(ns / MILLION >= 0);
                ASSERT((DWORD) (ns / MILLION) == dst);
            }
        }

        if (veryVerbose) {
            Q(Hold sec at max and vary nsec over full range);
        }
        ns = 0;
        for (ns = 0; ns < BILLION; ns += MILLION) {
            bdet_TimeInterval ti(int64Max, ns);
            Obj::toMillisec(&dst, ti);

            ASSERT(dst == uintMax);
        }

        if (veryVerbose) {
            Q(Hold sec at min and vary nsec over full range);
        }
        ns = 0;
        for (ns = 0; ns > -BILLION; ns -= MILLION) {
            bdet_TimeInterval ti(int64Min, ns);
            Obj::toMillisec(&dst, ti);

            ASSERT(dst == 0);
        }

        if (veryVerbose) {
            cout << "\tTest 64bit toMilleconds" << endl;
        }
        {
            typedef bsls::Types::Int64 Int64;
            typedef bsls::Types::Uint64 Uint64;

            const Int64  MAX_INT64  = bsl::numeric_limits<Int64>::max();
            const Int64  MIN_INT64  = bsl::numeric_limits<Int64>::min();
            const Uint64 MAX_UINT64 = bsl::numeric_limits<Uint64>::max();


            enum {
                MILLISECS_PER_SEC     = 1000,        // one thousand
                MICROSECS_PER_SEC     = 1000000,     // one million
                NANOSECS_PER_MICROSEC = 1000,        // one thousand
                NANOSECS_PER_MILLISEC = 1000000,     // one million
                NANOSECS_PER_SEC      = 1000000000   // one billion
            };

            // Compute the threshold for saturation.

            const Int64    SEC_LIMIT = MAX_UINT64 / MILLISECS_PER_SEC;
            const int NANO_SEC_LIMIT = (MAX_UINT64 - (SEC_LIMIT * 1000))
                                                    * NANOSECS_PER_MILLISEC;

            P(MAX_UINT64);
            P(SEC_LIMIT);
            struct {
                int    d_line;
                Int64  d_seconds;
                Int64  d_nanoseconds;
                Uint64 d_expectedResult;
            } VALUES[] = {
                { L_,             0,         0,                 0 },
                { L_,     MAX_INT64, 999999999,        MAX_UINT64 },
                { L_,     MIN_INT64,         0,                 0 },

                // Test values approach and just past the limit
       { L_, SEC_LIMIT - 1,         0, ((Uint64)SEC_LIMIT -1) * 1000 },
       { L_,     SEC_LIMIT,         0,      (Uint64)SEC_LIMIT * 1000 },

       { L_,     SEC_LIMIT, NANO_SEC_LIMIT - 1000000, MAX_UINT64 - 1 },
       { L_,     SEC_LIMIT,           NANO_SEC_LIMIT,     MAX_UINT64 },
       { L_,     SEC_LIMIT,       NANO_SEC_LIMIT + 1,     MAX_UINT64 },
       { L_,     SEC_LIMIT, NANO_SEC_LIMIT + 1000000,     MAX_UINT64 },
       { L_, SEC_LIMIT + 1,                        0,    MAX_UINT64 },
       { L_, SEC_LIMIT + 2,                        0,    MAX_UINT64 },


            };
            const int NUM_VALUES = sizeof(VALUES) / sizeof(*VALUES);


            for (int i = 0; i < NUM_VALUES; ++i) {
                const int    LINE     = VALUES[i].d_line;
                const Uint64 EXPECTED = VALUES[i].d_expectedResult;
                bdet_TimeInterval input(VALUES[i].d_seconds,
                                        VALUES[i].d_nanoseconds);

                Uint64 result;

                Obj::toMillisec(&result, input);

                ASSERTV(LINE, EXPECTED, result, input, EXPECTED == result);
            }
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING 'toTimeT'
        //
        // Concerns:
        //   Note that the exact type of 'time_t' is not clearly specified
        //   and may vary with the platform.  We must test for possibilities
        //   of 'time_t' being signed or unsigned, 4 or 8 bit.
        //: 1 That 'toTimeT' assigns 'Int64's to 'time_t's properly, exactly
        //:   copying values whenever possible.
        //: 2 That 'toTimeT' correctly saturating when it is not possible to
        //:   copy a value exactly.
        //
        // Plan:
        //: 1 32-bit signed
        //:   o Iterate over the range of values that can be exactly assigned,
        //:     incrementing in 64K increments, being sure to include the min
        //:     and max possible values.
        //:   o iterate from the max exactly assignable value to the max of
        //:     of the input range, in iterations of (1 << 48), observing
        //:     saturation.
        //:   o iterate from the min exactly assignable value to the min of
        //:     of the input range, in iterations of (1 << 48), observing
        //:     saturation.
        //:   o test the exact min and max values of the input range.
        //: 2 64-bit signed
        //:   o iterate from the min of the input range to the max of the
        //:     input range in increments of (1 << 48), observing that values
        //:     are assigned without modification.
        //:   o iterate from the max of the input range to the min of the
        //:     input range in increments of (1 << 48), observing that values
        //:     are assigned without modification.
        //: 3 32-bit unsigned
        //:   o Iterate over the range of values that can be exactly assigned,
        //:     incrementing in 64K increments, being sure to include the min
        //:     and max possible values.
        //:   o iterate from the max value that can be accurately copied to
        //:     the max of the input range in (1 << 48) increments, observing
        //:     saturation.
        //:   o iterate from 0 to the min of the input range in (1 << 48)
        //:     increments, observing saturation.
        //:   o test the exact min and max of the input range, observing
        //:     saturation.
        //: 4 64-bit unsigned
        //:   o traverse the input range in (1 << 48) increments, taking care
        //:     to include both the absolute min and max values, observing
        //:     that non-negative values are copied without modification,
        //:     and negative values are saturated as 0.
        //
        // TESTING
        //   'toTimeT'
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING TIME_T\n"
                             "==============\n";

        bsl::time_t tt;
        int ct;

        tt = -1;

        if (verbose) {
            P_((tt < 0));    P(sizeof(bsl::time_t));
        }

        if (tt < 0 && sizeof(tt) == 4) {
            if (verbose) Q(Signed and 4 bytes);

            if (veryVerbose) Q(Vary across full non-saturating range);
            ct = 0;
            for (Int64 i = intMin; i <= intMax; i += i16, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == i);
            }
            ASSERT(ct > 65000);
            ct = 0;
            for (Int64 i = intMax; i >= intMin; i -= i16, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == i);
            }
            ASSERT(ct > 65000);

            if (veryVerbose) Q(Vary across full positve-saturating range);
            ct = 0;
            for (Int64 i = intMax; i < int64Max - i48; i += i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == intMax);
            }
            ASSERT(ct > 32000);

            if (veryVerbose) Q(Vary across full negatve-saturating range);
            ct = 0;
            for (Int64 i = intMin; i > int64Min + i48; i -= i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == intMin);
            }
            ASSERT(ct > 32000);

            if (veryVerbose) Q(Try max & min inputs);
            Obj::toTimeT(&tt, int64Max);
            ASSERT((Int64) tt == intMax);

            Obj::toTimeT(&tt, int64Min);
            ASSERT((Int64) tt == intMin);
        }
        else if (tt < 0 && sizeof(tt) == 8) {
            if (verbose) Q(Signed and 8 bytes);

            if (veryVerbose) Q(Vary across full range with no saturation);
            ct = 0;
            for (Int64 i = int64Min; i < int64Max - i48; i += i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == i);
            }
            ASSERT(ct > 65000);
            ct = 0;
            for (Int64 i = int64Max; i > int64Min + i48; i -= i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Int64) tt == i);
            }
            ASSERT(ct > 65000);
        }
        else if (tt > 0 && sizeof(tt) == 4) {
            if (verbose) Q(Unsigned and 4 bytes);

            if (veryVerbose) Q(Vary across full non-saturating range);
            ct = 0;
            for (Int64 i = 0; i <= uintMax; i += i16, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT(i >= 0 && (Uint64) tt == (Uint64) i);
            }
            ASSERT(ct > 65000);
            ct = 0;
            for (Int64 i = uintMax; i >= 0; i -= i16, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT(i >= 0 && (Uint64) tt == (Uint64) i);
            }
            ASSERT(ct > 65000);

            if (veryVerbose) Q(Vary across positive saturating range);
            ct = 0;
            for (Int64 i = uintMax; i < int64Max - i48; i += i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT((Uint64) tt == (Uint64) uintMax);
            }
            ASSERT(ct > 32000);

            if (veryVerbose) Q(Vary across negative saturating range);
            ct = 0;
            for (Int64 i = 0; i > int64Min + i48; i -= i48, ++ct) {
                Obj::toTimeT(&tt, i);

                ASSERT(tt == 0);
            }
            ASSERT(ct > 32000);

            if (veryVerbose) Q(Try exact max and min inputs);

            Obj::toTimeT(&tt, int64Max);
            ASSERT((Uint64) tt == (Uint64) uintMax);

            Obj::toTimeT(&tt, int64Min);
            ASSERT(tt == 0);
        }
        else if (tt > 0 && sizeof(tt) == 8) {
            if (verbose) Q(Unsigned and 8 bytes);

            if (veryVerbose) Q(Vary across full range with no saturation);
            ct = 0;
            for (Int64 i = int64Min; i < int64Max - i48; i += i48, ++ct) {
                Obj::toTimeT(&tt, i);

                if (i < 0) {
                    ASSERT(0 == tt);
                }
                else {
                    ASSERT((Uint64) tt == (Uint64) i);
                }
            }
            ASSERT(ct > 65000);
            ct = 0;
            for (Int64 i = int64Max; i > int64Min + i48; i -= i48, ++ct) {
                Obj::toTimeT(&tt, i);

                if (i < 0) {
                    ASSERT(0 == tt);
                }
                else {
                    ASSERT((Uint64) tt == (Uint64) i);
                }
            }
            ASSERT(ct > 65000);
        }
        else {
            ASSERT(0);
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING TIMESPEC -- UNSIGNED TV_SEC
        //
        // Concerns:
        //   This test case concerns the case where the 'tv_sec' field of
        //   'timespec' is unsigned.
        //: 1 That 'toTimeSpec' will properly assign a value from a
        //:   'bdet_TimeInterval' to a 'timespec' if the 'bdet_TimeInterval's
        //:   value can be exactly represented by the 'timespec'.
        //: 2 That if a value is too high or too low to be represented by the
        //:   'timespec', the 'timespec' is assigned the highest or lowest
        //:   value it can represent, whichever is closer to the intended
        //:   value.
        //
        // Plan:
        //: 1 sizeof(tv_sec) == 4
        //:   o Iterate testing for values of 'TimeInterval.seconds()' from 0
        //:     to 'uintMax' at intervals of 64K, making sure to include the
        //:     exact values of 0 and 'uintMax', varying the nanosecond field
        //:     slightly but keeping it in normal range.
        //:   o Test with values above and below the range which can be
        //:     represented with a 'timespec', and observe that both fields of
        //:     the 'timespec' are at the appropriate extreme.
        //:   o Test with 'timeInterval.second()' at its exact minimum and
        //:     maximum values, and verify the results are as they should be.
        //: 2 sizeof(tv_sec) == 8
        //:   o Iterate from the minimum to the maximum values of
        //:     'timeInterval.seconds()', verifying that 'tv_sec' is always
        //:     exactly equal to the '.seconds()' value.
        //
        // TESTING
        //   toTimeSpec
        // --------------------------------------------------------------------

        testUnsignedTimespec<Obj::TimeSpec>("TimeSpec");

#ifdef BSLS_PLATFORM_OS_DARWIN
        testUnsignedTimespec<mach_timespec_t>("mach_timespec_t");
#endif
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING TIMESPEC -- SIGNED TV_SEC
        //
        // Concerns:
        //   This test case concerns the case where the 'tv_sec' field of
        //   'timespec' is signed.
        //: 1 That 'toTimeSpec' will properly assign a value from a
        //:   'bdet_TimeInterval' to a 'timespec' if the 'bdet_TimeInterval's
        //:   value can be exactly represented by the 'timespec'.
        //: 2 That if a value is too high or too low to be represented by the
        //:   'timespec', the 'timespec' is assigned the highest or lowest
        //:   value it can represent, whichever is closer to the intended
        //:   value.
        //
        // Plan:
        //: 1 sizeof(tv_sec) == 4
        //:   o Iterate testing for values of 'TimeInterval.seconds()' from
        //:     'intMin' to 'intMax' at intervals of 64K, making sure to
        //:     include the exact values of 'intMin' and 'intMax', varying the
        //:     nanosecond field slightly but keeping it in normal range.
        //:   o Test with values above and below the range which can be
        //:     represented with a 'timespec', and observe that both fields of
        //:     the 'timespec' are at the appropriate extreme.
        //:   o Test with 'timeInterval.second()' at its exact minimum and
        //:     maximum values, and verify the results are as they should be.
        //: 1 sizeof(tv_sec) == 8
        //:   o Iterate from the minimum to the maximum values of
        //:     'timeInterval.seconds()', verifying that 'tv_sec' is always
        //:     exactly equal to the '.seconds()' value.
        //
        // TESTING
        //   toTimeSpec
        // --------------------------------------------------------------------

        testSignedTimespec<Obj::TimeSpec>("TimeSpec");

#ifdef BSLS_PLATFORM_OS_DARWIN
        testSignedTimespec<mach_timespec_t>("mach_timespec_t");
#endif
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //
        // Concerns:
        //   Demonstrate basic functionality, which will vary depending upon
        //   the type of 'TimeSpec'.
        //
        // Plan:
        //: 1 Demonstrate assignment on all platforms with no saturation..
        //: 2 Demonstrate saturation if 'tv_sec' is 4 byte unsigned.
        //: 3 Demonstrate saturation if 'tv_sec' is 4 byte signed.
        //: 4 Demonstrate assignment & saturation if 'tv_sec' is 8 byte
        //:   unsigned.
        //: 5 Demonstrate more assignment (no saturation possible) if 'tv_sec'
        //:   is 8 byte signed.
        // --------------------------------------------------------------------

        if (verbose) cout << "BREATHING TEST\n"
                             "==============\n";

        typedef bsls::Types::Int64  Int64;
        typedef bsls::Types::Uint64 Uint64;

        enum { MAX_NANOSECONDS = 1000 * 1000 * 1000 - 1 };

        Obj::TimeSpec ts;

        if (veryVerbose) Q(Demonstrate assignment with no saturation);

        Obj::toTimeSpec(&ts, bdet_TimeInterval(123456, 987654321));
        ASSERT(123456    == ts.tv_sec);
        ASSERT(987654321 == ts.tv_nsec);

        Obj::toTimeSpec(&ts, bdet_TimeInterval(0, 0));
        ASSERT(0 == ts.tv_sec);
        ASSERT(0 == ts.tv_nsec);

        ts.tv_sec = -1;

        // We sometimes have to introduce a 'matcher' variable to avoid
        // compiler warnings when code intended for one type of 'timespec'
        // gets compiled on a platform where 'timespec' has a different type.

        if (4 == sizeof(ts.tv_sec)) {
            if (ts.tv_sec > 0) {
                if (veryVerbose) Q(Demonstrate saturation on 4 byte unsigned);

                unsigned int matcher = 0;
                matcher += bsl::numeric_limits<unsigned int>::max();

                Obj::toTimeSpec(&ts, bdet_TimeInterval((Int64) 1 << 48,
                                                       987654321));
                ASSERT(matcher         == (unsigned int) ts.tv_sec);
                ASSERT(MAX_NANOSECONDS == ts.tv_nsec);

                Obj::toTimeSpec(&ts, bdet_TimeInterval((Int64) -1 << 48,
                                                       -987654321));
                ASSERT(0 == ts.tv_sec);
                ASSERT(0 == ts.tv_nsec);
            }
            else {
                if (veryVerbose) Q(Demonstrate saturation with 4 byte signed);

                Obj::toTimeSpec(&ts, bdet_TimeInterval((Int64) 1 << 48,
                                                       987654321));
                ASSERT(bsl::numeric_limits<int>::max() == ts.tv_sec);
                ASSERT(MAX_NANOSECONDS                 == ts.tv_nsec);

                int matcher = bsl::numeric_limits<int>::max();
                matcher = -matcher - 1;    // numeric_limites::min

                Obj::toTimeSpec(&ts, bdet_TimeInterval((Int64) -1 << 48,
                                                       -987654321));
                ASSERT(matcher          == (int) ts.tv_sec);
                ASSERT(-MAX_NANOSECONDS == ts.tv_nsec);
            }
        }
        else {
            ASSERT(8 == sizeof(ts.tv_sec));

            if (ts.tv_sec > 0) {
                if (veryVerbose) Q(Demonstrate assignment on 8 byte unsigned);

                Uint64 matcher = 1;
                matcher <<= 48;

                Obj::toTimeSpec(&ts, bdet_TimeInterval(matcher, 987654321));
                ASSERT(matcher   == (Uint64) ts.tv_sec);
                ASSERT(987654321 == ts.tv_nsec);

                if (veryVerbose) Q(Demonstrate saturation on 8 byte unsigned);

                Obj::toTimeSpec(&ts, bdet_TimeInterval((Int64) -1 << 48,
                                                        -987654321));
                ASSERT(0 == ts.tv_sec);
                ASSERT(0 == ts.tv_nsec);
            }
            else {
                if (veryVerbose) Q(Demonstrate assignment with 8 byte signed);

                Int64 matcher = 1;
                matcher <<= 48;

                Obj::toTimeSpec(&ts, bdet_TimeInterval(matcher, 987654321));
                ASSERT(matcher   == (Int64) ts.tv_sec);
                ASSERT(987654321 == ts.tv_nsec);

                matcher = -1;
                matcher <<= 48;

                Obj::toTimeSpec(&ts, bdet_TimeInterval(matcher, -987654321));
                ASSERT(matcher    == (Int64) ts.tv_sec);
                ASSERT(-987654321 == ts.tv_nsec);
            }
        }
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }
    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2013
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
