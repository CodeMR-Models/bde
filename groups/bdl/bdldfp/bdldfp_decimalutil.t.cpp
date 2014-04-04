// bdldfp_decimalutil.t.cpp                                           -*-C++-*-
#include <bdldfp_decimalutil.h>

#include <bslma_testallocator.h>
#include <bslma_defaultallocatorguard.h>

#include <bsls_assert.h>
#include <bsls_asserttest.h>

#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_cstdlib.h>
#include <bsl_climits.h>
#include <bsl_limits.h>
#include <bsl_cmath.h>

#include <typeinfo>

using namespace BloombergLP;
using bsl::cout;
using bsl::cerr;
using bsl::flush;
using bsl::endl;
using bsl::atoi;

//=============================================================================
//                                 TEST PLAN
//-----------------------------------------------------------------------------
//                                  Overview
//                                  --------
// TBD:
//-----------------------------------------------------------------------------
// CREATORS
//
// MANIPULATORS
//
// ACCESSORS
//
// FREE OPERATORS
//
// TRAITS
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [  ] USAGE EXAMPLE
// ----------------------------------------------------------------------------


//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP6_ASSERT(I,J,K,L,M,N,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\t" << #N << ": " << N << "\n"; \
       aSsErT(1, #X, __LINE__); } }

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number
#define T_ cout << "\t" << flush;             // Print tab w/o newline

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

namespace BDEC = BloombergLP::bdldfp;

#if defined(BSLS_PLATFORM_OS_WINDOWS) && !defined(FP_NAN)

// MS does not provide standard floating-point classification so we do

// First, make sure the environment is sane

#if defined(FP_NORMAL) || defined(FP_INFINITE) || defined(FP_ZERO) || \
    defined(FP_SUBNORMAL)
#error Standard FP_ macros are not defined properly.
#endif

// Make it look like stiff MS has in ymath.h

#define FP_SUBNORMAL (-2)
#define FP_NORMAL    (-1)
#define FP_ZERO        0
#define FP_INFINITE    1
#define FP_NAN         2

#endif


//=============================================================================
//                               USAGE EXAMPLE
//-----------------------------------------------------------------------------

namespace UsageExample {
  // TBD
}  // close namespace UsageExample

// TODO: Find out why the 17 digit variants are failing tests.
const long long mantissas[] = {
                          //-12345678901234567ll,
                            - 2345678901234567ll,
                            - 1234567890123456ll,
                            -  234567890123456ll,
                            -  123456789012345ll,
                            -   23456789012345ll,
                            -   12345678901234ll,
                            -    2345678901234ll,
                            -    1234567890123ll,
                            -     234567890123ll,
                            -     123456789012ll,
                            -      23456789012ll,
                            -      12345678901ll,
                            -       2345678901ll,
                            -       1234567890ll,
                            -        234567890ll,
                            -        123456789ll,
                            -         23456789ll,
                            -         12345678ll,
                            -          2345678ll,
                            -          1234567ll,
                            -           234567ll,
                            -           123456ll,
                            -            23456ll,
                            -            12345ll,
                            -             6721ll,
                            -             5723ll,
                            -              317ll,
                            -              100ll,
                            -               83ll,
                            -               27ll,
                            -                9ll,
                            -                5ll,
                                             0ll,
                                             5ll,
                                             9ll,
                                            27ll,
                                            83ll,
                                           100ll,
                                           317ll,
                                          5723ll,
                                          6721ll,
                                         12345ll,
                                         23456ll,
                                        123456ll,
                                        234567ll,
                                       1234567ll,
                                       2345678ll,
                                      12345678ll,
                                      23456789ll,
                                     123456789ll,
                                     234567890ll,
                                    1234567890ll,
                                    2345678901ll,
                                   12345678901ll,
                                   23456789012ll,
                                  123456789012ll,
                                  234567890123ll,
                                 1234567890123ll,
                                 2345678901234ll,
                                12345678901234ll,
                                23456789012345ll,
                               123456789012345ll,
                               234567890123456ll,
                              1234567890123456ll,
                              2345678901234567ll,
                           //12345678901234567ll,
                        };
const int numMantissas = sizeof(mantissas) / sizeof(*mantissas);

const int exps[] =            {
                                          -321,
                                          -129,
                                          - 23,
                                          - 10,
                                          -  7,
                                          -  2,
                                             0,
                                             2,
                                             7,
                                            10,
                                            23,
                                           129,
                                           321,
                        };
const int numExps      = sizeof(exps) / sizeof(*mantissas);

//=============================================================================
//              GLOBAL HELPER FUNCTIONS AND CLASSES FOR TESTING
//-----------------------------------------------------------------------------

                 // stringstream helpers - not thread safe!

void getStringFromStream(bsl::ostringstream &o, bsl::string  *out)
{
    bslma::TestAllocator osa("osstream");
    bslma::DefaultAllocatorGuard g(&osa);
    *out = o.str();
}

void getStringFromStream(bsl::wostringstream &o, bsl::wstring *out)
{
    bslma::TestAllocator osa("osstream");
    bslma::DefaultAllocatorGuard g(&osa);
    *out = o.str();
}

 // String compare for decimal floating point numbers needs 'e'/'E' conversion

bsl::string& decLower(bsl::string& s)
{
    for (size_t i = 0; i < s.length(); ++i) if ('E' == s[i]) s[i] = 'e';
    return s;
}

bsl::wstring& decLower(bsl::wstring& s)
{
    for (size_t i = 0; i < s.length(); ++i) if (L'E' == s[i]) s[i] = L'e';
    return s;
}

//-----------------------------------------------------------------------------


template <class Expect, class Received>
void checkType(const Received&)
{
    ASSERT(typeid(Expect) == typeid(Received));
}

template <class TYPE>
struct NumberMaker;

template <>
struct NumberMaker<BDEC::Decimal64>
{
    BDEC::Decimal64 operator()(long long mantissa, int exponent) const
    {
        return BDEC::DecimalImplUtil::makeDecimalRaw64(mantissa, exponent);
    }
};

template <>
struct NumberMaker<BDEC::Decimal128>
{
    BDEC::Decimal128 operator()(long long mantissa, int exponent) const
    {
        return BDEC::DecimalImplUtil::makeDecimalRaw128(mantissa, exponent);
    }
};


                          // Stream buffer helpers

template <int Size>
struct BufferBuf : bsl::streambuf {
    BufferBuf() { reset(); }
    const char *str() { *this->pptr() =0; return this->pbase(); }
    void reset() { this->setp(this->d_buf, this->d_buf + Size); }
    char d_buf[Size + 1];
};

struct PtrInputBuf : bsl::streambuf {
    PtrInputBuf(const char *s)
    {
        char *x = const_cast<char *>(s);
        this->setg(x, x, x + strlen(x));
    }
};

struct NulBuf : bsl::streambuf {
    char d_dummy[64];
    virtual int overflow(int c)
    {
        setp( d_dummy, d_dummy + sizeof(d_dummy));
        return traits_type::not_eof(c);
    }
};

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    int               test = argc > 1 ? atoi(argv[1]) : 0;
    int           verbose1 = argc > 2;
    int           verbose2 = argc > 3;
    int           verbose3 = argc > 4;
    int allocatorVerbosity = argc > 5;  // always the last

    using bsls::AssertFailureHandlerGuard;

    bslma::TestAllocator defaultAllocator("default", allocatorVerbosity);
    bslma::Default::setDefaultAllocator(&defaultAllocator);

    bslma::TestAllocator globalAllocator("global", allocatorVerbosity);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    cout << "TEST " << __FILE__ << " CASE " << test << endl;;

    bslma::TestAllocator  ta(allocatorVerbosity);
    bslma::TestAllocator *pa = &ta;

    typedef BDEC::DecimalUtil Util;

    cout.precision(35);


    switch (test) { case 0:
    case 9: {
        // --------------------------------------------------------------------
        // TESTING fabs
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: fabs
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "fabs Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for quantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases work correctly.
            // o sNaN
            ASSERT(Util::isNan(Util::fabs(sNaN)));

            // o qNaN
            ASSERT(Util::isNan(Util::fabs(qNaN)));

            // o +Inf
            ASSERT(Util::fabs(pInf) == pInf);

            // o -Inf
            ASSERT(Util::fabs(nInf) == pInf);


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test the value of what quantum returns:
                    ASSERT(Util::fabs(value) ==
                           makeNumber(
                                     bsl::max(mantissas[tiM], -mantissas[tiM]),
                                                                   exps[tiE]));
                }
            }


        }
        if (verbose1) bsl::cout << "fabs Decimal128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for quantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases work correctly.
            // o sNaN
            ASSERT(Util::isNan(Util::fabs(sNaN)));

            // o qNaN
            ASSERT(Util::isNan(Util::fabs(qNaN)));

            // o +Inf
            ASSERT(Util::fabs(pInf) == pInf);

            // o -Inf
            ASSERT(Util::fabs(nInf) == pInf);


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test the value of what quantum returns:
                    ASSERT(Util::fabs(value) ==
                          makeNumber(bsl::max(mantissas[tiM], -mantissas[tiM]),
                                                                   exps[tiE]));
                }
            }


        }
    } break;
    case 8: {
        // --------------------------------------------------------------------
        // TESTING quantize
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: quantize
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "quantize Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for quantize, which depends upon the strict, narrow
            // contract for makeDecimalRaw.


            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test all special cases with each other,
            // organized by LHS.

            // o sNaN
            // Concern: quantize with NaN in either parameter must return a
            // NaN.
            // Concern: quantize of NaN and Inf must set the invalid exception
            // bit in the flags.  The Invalid bit must not be set for NaN/NaN
            // cases.  This concern isn't presently tested.
            ASSERT(Util::isNan(Util::quantize(sNaN, sNaN)));
            ASSERT(Util::isNan(Util::quantize(sNaN, qNaN)));
            ASSERT(Util::isNan(Util::quantize(sNaN, pInf)));
            ASSERT(Util::isNan(Util::quantize(sNaN, nInf)));

            // o qNaN
            // Concern: quantize with NaN in either parameter must return a
            // NaN.
            // Concern: quantize of NaN and Inf must set the invalid exception
            // bit in the flags.  The Invalid bit must not be set for NaN/NaN
            // cases.  This concern isn't presently tested.
            ASSERT(Util::isNan(Util::quantize(qNaN, sNaN)));
            ASSERT(Util::isNan(Util::quantize(qNaN, qNaN)));
            ASSERT(Util::isNan(Util::quantize(qNaN, pInf)));
            ASSERT(Util::isNan(Util::quantize(qNaN, nInf)));

            // o +Inf
            // Concern: quantize with NaN in either parameter must return a
            // NaN.
            // Concern: quantize of NaN and Inf must set the invalid exception
            // bit in the flags.  The Invalid bit must not be set for NaN/NaN
            // cases.  This concern isn't presently tested.
            // Concern: Infinity by infinity must return an infinity with
            // the sign of the first argument
            ASSERT(Util::isNan(Util::quantize(pInf, sNaN)));
            ASSERT(Util::isNan(Util::quantize(pInf, qNaN)));
            ASSERT(pInf ==     Util::quantize(pInf, pInf));
            ASSERT(pInf ==     Util::quantize(pInf, nInf));

            // o -Inf
            // Concern: quantize with NaN in either parameter must return a
            // NaN.
            // Concern: quantize of NaN and Inf must set the invalid exception
            // bit in the flags.  The Invalid bit must not be set for NaN/NaN
            // cases.  This concern isn't presently tested.
            // Concern: Infinity by infinity must return an infinity with
            // the sign of the first argument
            ASSERT(Util::isNan(Util::quantize(nInf, sNaN)));
            ASSERT(Util::isNan(Util::quantize(nInf, qNaN)));
            ASSERT(nInf ==     Util::quantize(nInf, pInf));
            ASSERT(nInf ==     Util::quantize(nInf, nInf));


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test all special cases on both sides:
                    // o sNaN
                    // o qNaN
                    // o +Inf
                    // o -Inf
                    // Concern: quantize with NaN in either parameter must
                    // return a NaN.
                    // Concern: quantize of NaN and Inf must set the invalid
                    // exception bit in the flags.  The Invalid bit must not be
                    // set for NaN/NaN cases.  This concern isn't presently
                    // tested.
                    // Concern: Infinity by infinity must return an infinity
                    // with the sign of the first argument
                    ASSERT(Util::isNan(Util::quantize(value, sNaN)));
                    ASSERT(Util::isNan(Util::quantize(value, qNaN)));
                    ASSERT(Util::isNan(Util::quantize(value, pInf)));
                    ASSERT(Util::isNan(Util::quantize(value, nInf)));

                    ASSERT(Util::isNan(Util::quantize(value, sNaN)));
                    ASSERT(Util::isNan(Util::quantize(value, qNaN)));
                    ASSERT(Util::isNan(Util::quantize(value, pInf)));
                    ASSERT(Util::isNan(Util::quantize(value, nInf)));
                }
            }

            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            // These will be compared with other values created from
            // the same table, in the same way.  Quantize will
            // produce things which have the same quantum.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE lhs =
                               makeNumber(mantissas[tiM], exps[tiE]);

                    for (long long tjM = 0; tjM < numMantissas; ++tjM) {
                        for (  int tjE = 0; tjE < numExps;      ++tjE) {
                            const TYPE rhs =
                               makeNumber(mantissas[tjM], exps[tjE]);

                            (void) rhs;
                            (void) lhs;

                            //TODO: Find out why this test fails:
                            #if 0
                            LOOP4_ASSERT(mantissas[tiM], exps[tiE],
                                         mantissas[tjM], exps[tjE],
                                         Util::sameQuantum(
                                                      Util::quantize(lhs, rhs),
                                                      rhs));
                            #endif
                        }
                    }
                }
            }
        }

        // TODO: Make the Decimal128 variant.
    } break;
    case 7: {
        // --------------------------------------------------------------------
        // TESTING quantum
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: quantum
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "quantum Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for quantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                AssertFailureHandlerGuard g(bsls::AssertTest::failTestDriver);
                // o sNaN
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(sNaN));

                // o qNaN
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(qNaN));

                // o +Inf
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(pInf));

                // o -Inf
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test the value of what quantum returns:
                    ASSERT(Util::quantum(value) == exps[tiE]);
                }
            }


        }
        if (verbose1) bsl::cout << "quantum Decimal128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for quantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                AssertFailureHandlerGuard g(bsls::AssertTest::failTestDriver);
                // o sNaN
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(sNaN));

                // o qNaN
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(qNaN));

                // o +Inf
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(pInf));

                // o -Inf
                BSLS_ASSERTTEST_ASSERT_FAIL(Util::quantum(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test the value of what quantum returns:
                    ASSERT(Util::quantum(value) == exps[tiE]);
                }
            }
        }
    } break;
    case 6: {
        // --------------------------------------------------------------------
        // TESTING sameQuantum
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: sameQuantum
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "sameQuantum Decimal64 tests..."
                                << bsl::endl;

        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for sameQuantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.


            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test all special cases with each other,
            // organized by LHS.

            // o sNaN
            ASSERT( Util::sameQuantum(sNaN, sNaN));
            ASSERT( Util::sameQuantum(sNaN, qNaN));
            ASSERT(!Util::sameQuantum(sNaN, pInf));
            ASSERT(!Util::sameQuantum(sNaN, nInf));

            // o qNaN
            ASSERT( Util::sameQuantum(qNaN, sNaN));
            ASSERT( Util::sameQuantum(qNaN, qNaN));
            ASSERT(!Util::sameQuantum(qNaN, pInf));
            ASSERT(!Util::sameQuantum(qNaN, nInf));

            // Note that +Inf compares equal to all inf values
            // o +Inf
            ASSERT(!Util::sameQuantum(pInf, sNaN));
            ASSERT(!Util::sameQuantum(pInf, qNaN));
            ASSERT( Util::sameQuantum(pInf, pInf));
            ASSERT( Util::sameQuantum(pInf, nInf));

            // Note that -Inf compares equal to all inf values
            // o -Inf
            ASSERT(!Util::sameQuantum(nInf, sNaN));
            ASSERT(!Util::sameQuantum(nInf, qNaN));
            ASSERT( Util::sameQuantum(nInf, pInf));
            ASSERT( Util::sameQuantum(nInf, nInf));


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test all special cases on both sides:
                    // o sNaN
                    // o qNaN
                    // o +Inf
                    // o -Inf
                    ASSERT(!Util::sameQuantum(value, sNaN));
                    ASSERT(!Util::sameQuantum(value, qNaN));
                    ASSERT(!Util::sameQuantum(value, pInf));
                    ASSERT(!Util::sameQuantum(value, nInf));

                    ASSERT(!Util::sameQuantum(sNaN, value));
                    ASSERT(!Util::sameQuantum(qNaN, value));
                    ASSERT(!Util::sameQuantum(pInf, value));
                    ASSERT(!Util::sameQuantum(nInf, value));
                }
            }

            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            // These will be compared with other values created from
            // the same table, in the same way.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE lhs =
                               makeNumber(mantissas[tiM], exps[tiE]);

                    for (long long tjM = 0; tjM < numMantissas; ++tjM) {
                        for (  int tjE = 0; tjE < numExps;      ++tjE) {
                            const TYPE rhs =
                               makeNumber(mantissas[tjM], exps[tjE]);

                            LOOP4_ASSERT(mantissas[tiM], exps[tiE],
                                         mantissas[tjM], exps[tjE],
                                (tiE == tjE) == Util::sameQuantum(lhs, rhs));
                        }
                    }
                }
            }
        }

        if (verbose1) bsl::cout << "sameQuantum Decimal128 tests..."
                                << bsl::endl;

        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for sameQuantum, which depends upon the strict, narrow
            // contract for makeDecimalRaw.


            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test all special cases with each other,
            // organized by LHS.

            // o sNaN
            ASSERT( Util::sameQuantum(sNaN, sNaN));
            ASSERT( Util::sameQuantum(sNaN, qNaN));
            ASSERT(!Util::sameQuantum(sNaN, pInf));
            ASSERT(!Util::sameQuantum(sNaN, nInf));

            // o qNaN
            ASSERT( Util::sameQuantum(qNaN, sNaN));
            ASSERT( Util::sameQuantum(qNaN, qNaN));
            ASSERT(!Util::sameQuantum(qNaN, pInf));
            ASSERT(!Util::sameQuantum(qNaN, nInf));

            // Note that +Inf compares equal to all inf values
            // o +Inf
            ASSERT(!Util::sameQuantum(pInf, sNaN));
            ASSERT(!Util::sameQuantum(pInf, qNaN));
            ASSERT( Util::sameQuantum(pInf, pInf));
            ASSERT( Util::sameQuantum(pInf, nInf));

            // Note that -Inf compares equal to all inf values
            // o -Inf
            ASSERT(!Util::sameQuantum(nInf, sNaN));
            ASSERT(!Util::sameQuantum(nInf, qNaN));
            ASSERT( Util::sameQuantum(nInf, pInf));
            ASSERT( Util::sameQuantum(nInf, nInf));


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test all special cases on both sides:
                    // o sNaN
                    // o qNaN
                    // o +Inf
                    // o -Inf
                    ASSERT(!Util::sameQuantum(value, sNaN));
                    ASSERT(!Util::sameQuantum(value, qNaN));
                    ASSERT(!Util::sameQuantum(value, pInf));
                    ASSERT(!Util::sameQuantum(value, nInf));

                    ASSERT(!Util::sameQuantum(sNaN, value));
                    ASSERT(!Util::sameQuantum(qNaN, value));
                    ASSERT(!Util::sameQuantum(pInf, value));
                    ASSERT(!Util::sameQuantum(nInf, value));
                }
            }

            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            // These will be compared with other values created from
            // the same table, in the same way.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE lhs =
                               makeNumber(mantissas[tiM], exps[tiE]);

                    for (long long tjM = 0; tjM < numMantissas; ++tjM) {
                        for (  int tjE = 0; tjE < numExps;      ++tjE) {
                            const TYPE rhs =
                               makeNumber(mantissas[tjM], exps[tjE]);

                            LOOP4_ASSERT(mantissas[tiM], exps[tiE],
                                         mantissas[tjM], exps[tjE],
                                (tiE == tjE) == Util::sameQuantum(lhs, rhs));
                        }
                    }
                }
            }
        }
    } break;
    case 5: {
        // --------------------------------------------------------------------
        // TESTING isFinite
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: isFinite
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "isFinite Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isFinite, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT(!Util::isFinite(sNaN));

                // o qNaN
                ASSERT(!Util::isFinite(qNaN));

                // o +Inf
                ASSERT(!Util::isFinite(pInf));

                // o -Inf
                ASSERT(!Util::isFinite(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't Inf
                    ASSERT(Util::isFinite(value));
                }
            }
        }
        if (verbose1) bsl::cout << "isFinite Decimal128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT(!Util::isFinite(sNaN));

                // o qNaN
                ASSERT(!Util::isFinite(qNaN));

                // o +Inf
                ASSERT(!Util::isFinite(pInf));

                // o -Inf
                ASSERT(!Util::isFinite(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't NaN
                    ASSERT(Util::isFinite(value));
                }
            }
        }
    } break;
    case 4: {
        // --------------------------------------------------------------------
        // TESTING isInf
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: isInf
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "isInf Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isInf, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT(!Util::isInf(sNaN));

                // o qNaN
                ASSERT(!Util::isInf(qNaN));

                // o +Inf
                ASSERT( Util::isInf(pInf));

                // o -Inf
                ASSERT( Util::isInf(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't Inf
                    ASSERT(!Util::isInf(value));
                }
            }
        }
        if (verbose1) bsl::cout << "isInf Decimal128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT(!Util::isInf(sNaN));

                // o qNaN
                ASSERT(!Util::isInf(qNaN));

                // o +Inf
                ASSERT( Util::isInf(pInf));

                // o -Inf
                ASSERT( Util::isInf(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't NaN
                    ASSERT(!Util::isInf(value));
                }
            }
        }
    } break;
    case 3: {
        // --------------------------------------------------------------------
        // TESTING isUnordered
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: isUnordered
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "isUnordered Decimal64 tests..."
                                << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            {
                // Test all special cases with each other,
                // organized by LHS.

                // o sNaN
                ASSERT( Util::isUnordered(sNaN, sNaN));
                ASSERT( Util::isUnordered(sNaN, qNaN));
                ASSERT( Util::isUnordered(sNaN, pInf));
                ASSERT( Util::isUnordered(sNaN, nInf));

                // o qNaN
                ASSERT( Util::isUnordered(qNaN, sNaN));
                ASSERT( Util::isUnordered(qNaN, qNaN));
                ASSERT( Util::isUnordered(qNaN, pInf));
                ASSERT( Util::isUnordered(qNaN, nInf));

                // Note that +Inf compares equal to all inf values
                // o +Inf
                ASSERT( Util::isUnordered(pInf, sNaN));
                ASSERT( Util::isUnordered(pInf, qNaN));
                ASSERT(!Util::isUnordered(pInf, pInf));
                ASSERT(!Util::isUnordered(pInf, nInf));

                // Note that -Inf compares equal to all inf values
                // o -Inf
                ASSERT( Util::isUnordered(nInf, sNaN));
                ASSERT( Util::isUnordered(nInf, qNaN));
                ASSERT(!Util::isUnordered(nInf, pInf));
                ASSERT(!Util::isUnordered(nInf, nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test all special cases on both sides:
                    // o sNaN
                    // o qNaN
                    // o +Inf
                    // o -Inf
                    ASSERT( Util::isUnordered(value, sNaN));
                    ASSERT( Util::isUnordered(value, qNaN));
                    ASSERT(!Util::isUnordered(value, pInf));
                    ASSERT(!Util::isUnordered(value, nInf));

                    ASSERT( Util::isUnordered(sNaN, value));
                    ASSERT( Util::isUnordered(qNaN, value));
                    ASSERT(!Util::isUnordered(pInf, value));
                    ASSERT(!Util::isUnordered(nInf, value));
                }
            }
        }

        if (verbose1) bsl::cout << "isUnordered Decimal128 tests..."
                                << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            {
                // Test all special cases with each other,
                // organized by LHS.

                // o sNaN
                ASSERT( Util::isUnordered(sNaN, sNaN));
                ASSERT( Util::isUnordered(sNaN, qNaN));
                ASSERT( Util::isUnordered(sNaN, pInf));
                ASSERT( Util::isUnordered(sNaN, nInf));

                // o qNaN
                ASSERT( Util::isUnordered(qNaN, sNaN));
                ASSERT( Util::isUnordered(qNaN, qNaN));
                ASSERT( Util::isUnordered(qNaN, pInf));
                ASSERT( Util::isUnordered(qNaN, nInf));

                // Note that +Inf compares equal to all inf values
                // o +Inf
                ASSERT( Util::isUnordered(pInf, sNaN));
                ASSERT( Util::isUnordered(pInf, qNaN));
                ASSERT(!Util::isUnordered(pInf, pInf));
                ASSERT(!Util::isUnordered(pInf, nInf));

                // Note that -Inf compares equal to all inf values
                // o -Inf
                ASSERT( Util::isUnordered(nInf, sNaN));
                ASSERT( Util::isUnordered(nInf, qNaN));
                ASSERT(!Util::isUnordered(nInf, pInf));
                ASSERT(!Util::isUnordered(nInf, nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test all special cases on both sides:
                    // o sNaN
                    // o qNaN
                    // o +Inf
                    // o -Inf
                    ASSERT( Util::isUnordered(value, sNaN));
                    ASSERT( Util::isUnordered(value, qNaN));
                    ASSERT(!Util::isUnordered(value, pInf));
                    ASSERT(!Util::isUnordered(value, nInf));

                    ASSERT( Util::isUnordered(sNaN, value));
                    ASSERT( Util::isUnordered(qNaN, value));
                    ASSERT(!Util::isUnordered(pInf, value));
                    ASSERT(!Util::isUnordered(nInf, value));
                }
            }
        }
    } break;
    case 2: {
        // --------------------------------------------------------------------
        // TESTING isNan
        // Concerns: Forwarding to the right routines
        // Plan: Try with several variations and combinations of
        //       decimal floats (different mantissas and exponents, both
        //       positive and negative.)
        // Testing: isNan
        // --------------------------------------------------------------------

        if (verbose1) bsl::cout << "isNan Decimal64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT( Util::isNan(sNaN));

                // o qNaN
                ASSERT( Util::isNan(qNaN));

                // o +Inf
                ASSERT(!Util::isNan(pInf));

                // o -Inf
                ASSERT(!Util::isNan(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't NaN
                    ASSERT(!Util::isNan(value));
                }
            }
        }
        if (verbose1) bsl::cout << "isNan Decimal128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 TYPE;
            typedef bsl::numeric_limits<TYPE> NumLim;
            NumberMaker<TYPE> makeNumber;

            // Test for isNan, which depends upon the strict, narrow
            // contract for makeDecimalRaw.

            // All special case values:
            // o signaling NaN     (sNaN)
            const TYPE sNaN(NumLim::signaling_NaN());
            // o quiet NaN         (qNaN)
            const TYPE qNaN(NumLim::quiet_NaN());
            // o positive Infinity (+Inf)
            const TYPE pInf(NumLim::infinity());
            // o negative Infinity (-Inf)
            const TYPE nInf(-pInf);

            // Test that all special cases fail.
            {
                // o sNaN
                ASSERT( Util::isNan(sNaN));

                // o qNaN
                ASSERT( Util::isNan(qNaN));

                // o +Inf
                ASSERT(!Util::isNan(pInf));

                // o -Inf
                ASSERT(!Util::isNan(nInf));
            }


            // Iterate through all possible pairings of mantissa and
            // exponent, and build Decimal64 values for each of them.
            for (long long tiM = 0; tiM < numMantissas; ++tiM) {
                for (  int tiE = 0; tiE < numExps;      ++tiE) {
                    const TYPE value =
                            makeNumber(mantissas[tiM], exps[tiE]);

                    // Test that any real value isn't NaN
                    ASSERT(!Util::isNan(value));
                }
            }
        }
    } break;
    case 1: {
        // --------------------------------------------------------------------
        // TESTING Breathing test
        // Concerns: Forwarding to the right routines
        // Plan: Try all operations see if basics work
        // Testing: all functions
        // --------------------------------------------------------------------
        if (verbose1) bsl::cout << "\nBreathing Test"
                                << "\n==============" << bsl::endl;

        if (verbose2) bsl::cout << "makeDecimalNNRaw functions" << bsl::endl;

        if (verbose3) bsl::cout << "makeDecimalRaw32" << bsl::endl;

        ASSERT(Util::makeDecimalRaw32(314159, -5) ==
               BDLDFP_DECIMAL_DF(3.14159));

        if (verbose3) bsl::cout << "makeDecimalRaw64" << bsl::endl;

        ASSERT(Util::makeDecimalRaw64(314159, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));
        ASSERT(Util::makeDecimalRaw64(314159u, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));
        ASSERT(Util::makeDecimalRaw64(314159ll, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));
        ASSERT(Util::makeDecimalRaw64(314159ull, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));

        if (verbose3) bsl::cout << "makeDecimalRaw128" << bsl::endl;

        ASSERT(Util::makeDecimalRaw128(314159, -5) ==
               BDLDFP_DECIMAL_DL(3.14159));
        ASSERT(Util::makeDecimalRaw128(314159u, -5) ==
               BDLDFP_DECIMAL_DL(3.14159));
        ASSERT(Util::makeDecimalRaw128(314159ll, -5) ==
               BDLDFP_DECIMAL_DL(3.14159));
        ASSERT(Util::makeDecimalRaw128(314159ull, -5) ==
               BDLDFP_DECIMAL_DL(3.14159));

        if (verbose2) bsl::cout << "makeDecimalNN functions" << bsl::endl;

        if (verbose3) bsl::cout << "makeDecimal64" << bsl::endl;

#if defined(BSLS_PLATFORM_CMP_GNU) && BSLS_PLATFORM_CMP_VER_MAJOR > 40700
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
#endif

        // Test some zero-rounded values.

        ASSERT(Util::makeDecimal64(-1234567890123456ll, -382-16+1) ==
               BDLDFP_DECIMAL_DD(-1.234567890123456e-382));
        ASSERT(Util::makeDecimal64(1234567890123456ull, -382-16+1) ==
               BDLDFP_DECIMAL_DD(1.234567890123456e-382));

#if defined(BSLS_PLATFORM_CMP_GNU) && BSLS_PLATFORM_CMP_VER_MAJOR > 40700
#pragma GCC diagnostic pop
#endif

        // Test some simple values.

        ASSERT(Util::makeDecimal64(314159, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));
        ASSERT(Util::makeDecimal64(314159u, -5) ==
               BDLDFP_DECIMAL_DD(3.14159));

        if (verbose2) bsl::cout << "parseDecimalNN functions" << bsl::endl;

        {
            BDEC::Decimal32 result;
            ASSERT(Util::parseDecimal32(&result, "1234567") == 0);
            ASSERT(BDLDFP_DECIMAL_DF(1234567.0) == result);
        }
        {
            BDEC::Decimal64 result;
            ASSERT(Util::parseDecimal64(&result, "1234567890123456") == 0);
            ASSERT(BDLDFP_DECIMAL_DD(1234567890123456.0) == result);
        }
        {
            BDEC::Decimal128 result;
            ASSERT(Util::parseDecimal128(&result,
                                         "1234567890123456789012345678901234")
                   == 0);
            ASSERT(BDLDFP_DECIMAL_DL(1234567890123456789012345678901234.0)
                   == result);
        }

        {
            BDEC::Decimal32 result;
            const bsl::string str("1234567", pa);
            ASSERT(Util::parseDecimal32(&result, str) == 0);
            ASSERT(BDLDFP_DECIMAL_DF(1234567.0) == result);
        }
        {
            BDEC::Decimal64 result;
            const bsl::string str("1234567890123456", pa);
            ASSERT(Util::parseDecimal64(&result, str) == 0);
            ASSERT(BDLDFP_DECIMAL_DD(1234567890123456.0) == result);
        }
        {
            BDEC::Decimal128 result;
            const bsl::string str("1234567890123456789012345678901234", pa);
            ASSERT(Util::parseDecimal128(&result,str) == 0);
            ASSERT(BDLDFP_DECIMAL_DL(1234567890123456789012345678901234.0)
                   == result);
        }

        if (verbose2) bsl::cout << "fma functions" << bsl::endl;

        // TODO TBD - How to test if fma really does not round too early?

        ASSERT(Util::fma(BDLDFP_DECIMAL_DD(3.),
                         BDLDFP_DECIMAL_DD(4.),
                         BDLDFP_DECIMAL_DD(5.)) == BDLDFP_DECIMAL_DD(17.));

        ASSERT(Util::fma(BDLDFP_DECIMAL_DL(3.),
                         BDLDFP_DECIMAL_DL(4.),
                         BDLDFP_DECIMAL_DL(5.)) == BDLDFP_DECIMAL_DL(17.));

        if (verbose2) bsl::cout << "fabs functions" << bsl::endl;

        ASSERT(Util::fabs(BDLDFP_DECIMAL_DF(-1.234567e-94))
               == BDLDFP_DECIMAL_DF(1.234567e-94));

#if defined(BSLS_PLATFORM_CMP_GNU) && BSLS_PLATFORM_CMP_VER_MAJOR > 40700
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
#endif

        // Test using zero-rounded numbers

        ASSERT(Util::fabs(BDLDFP_DECIMAL_DD(-1.234567890123456e-382))
               == BDLDFP_DECIMAL_DD(1.234567890123456e-382));
        ASSERT(Util::fabs(BDLDFP_DECIMAL_DL(-1.234567890123456e-382))
               == BDLDFP_DECIMAL_DL(1.234567890123456e-382));

#if defined(BSLS_PLATFORM_CMP_GNU) && BSLS_PLATFORM_CMP_VER_MAJOR > 40700
#pragma GCC diagnostic pop
#endif

        if (verbose2) bsl::cout << "Classification functions" << bsl::endl;

        if (verbose3) bsl::cout << "Decimal32" << bsl::endl;
        {
            typedef BDEC::Decimal32 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DF(x)

            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            ASSERT(Util::classify(oNaNq) == FP_NAN);
            ASSERT(Util::classify(oNaNs) == FP_NAN);

            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);
            ASSERT(Util::classify(oInfP) == FP_INFINITE);
            ASSERT(Util::classify(oInfN) == FP_INFINITE);

            const Tested oNorm1(NumLim::max());
            const Tested oNorm2(NumLim::min());
            ASSERT(Util::classify(oNorm1) == FP_NORMAL);
            ASSERT(Util::classify(oNorm2) == FP_NORMAL);

            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            ASSERT(Util::classify(oZeroP) == FP_ZERO);
            ASSERT(Util::classify(oZeroN) == FP_ZERO);

            const Tested oDenmP(NumLim::denorm_min());
            const Tested oDenmN(-oDenmP);
            ASSERT(Util::classify(oDenmP) == FP_SUBNORMAL);
            ASSERT(Util::classify(oDenmN) == FP_SUBNORMAL);

            ASSERT(Util::isNan(oNaNq)  == true);
            ASSERT(Util::isNan(oNaNs)  == true);
            ASSERT(Util::isNan(oInfP)  == false);
            ASSERT(Util::isNan(oInfN)  == false);
            ASSERT(Util::isNan(oNorm1) == false);
            ASSERT(Util::isNan(oNorm2) == false);
            ASSERT(Util::isNan(oZeroP) == false);
            ASSERT(Util::isNan(oZeroN) == false);
            ASSERT(Util::isNan(oDenmP) == false);
            ASSERT(Util::isNan(oDenmN) == false);

            ASSERT(Util::isInf(oNaNq)  == false);
            ASSERT(Util::isInf(oNaNs)  == false);
            ASSERT(Util::isInf(oInfP)  == true);
            ASSERT(Util::isInf(oInfN)  == true);
            ASSERT(Util::isInf(oNorm1) == false);
            ASSERT(Util::isInf(oNorm2) == false);
            ASSERT(Util::isInf(oZeroP) == false);
            ASSERT(Util::isInf(oZeroN) == false);
            ASSERT(Util::isInf(oDenmP) == false);
            ASSERT(Util::isInf(oDenmN) == false);

            ASSERT(Util::isFinite(oNaNq)  == false);
            ASSERT(Util::isFinite(oNaNs)  == false);
            ASSERT(Util::isFinite(oInfP)  == false);
            ASSERT(Util::isFinite(oInfN)  == false);
            ASSERT(Util::isFinite(oNorm1) == true);
            ASSERT(Util::isFinite(oNorm2) == true);
            ASSERT(Util::isFinite(oZeroP) == true);
            ASSERT(Util::isFinite(oZeroN) == true);
            ASSERT(Util::isFinite(oDenmP) == true);
            ASSERT(Util::isFinite(oDenmN) == true);

            ASSERT(Util::isNormal(oNaNq)  == false);
            ASSERT(Util::isNormal(oNaNs)  == false);
            ASSERT(Util::isNormal(oInfP)  == false);
            ASSERT(Util::isNormal(oInfN)  == false);
            ASSERT(Util::isNormal(oNorm1) == true);
            ASSERT(Util::isNormal(oNorm2) == true);
            ASSERT(Util::isNormal(oZeroP) == false);
            ASSERT(Util::isNormal(oZeroN) == false);
            ASSERT(Util::isNormal(oDenmP) == false);
            ASSERT(Util::isNormal(oDenmN) == false);

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal64" << bsl::endl;
        {
            typedef BDEC::Decimal64 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DD(x)

            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            ASSERT(Util::classify(oNaNq) == FP_NAN);
            ASSERT(Util::classify(oNaNs) == FP_NAN);

            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);
            ASSERT(Util::classify(oInfP) == FP_INFINITE);
            ASSERT(Util::classify(oInfN) == FP_INFINITE);

            const Tested oNorm1(NumLim::max());
            const Tested oNorm2(NumLim::min());
            ASSERT(Util::classify(oNorm1) == FP_NORMAL);
            ASSERT(Util::classify(oNorm2) == FP_NORMAL);

            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            ASSERT(Util::classify(oZeroP) == FP_ZERO);
            ASSERT(Util::classify(oZeroN) == FP_ZERO);

            const Tested oDenmP(NumLim::denorm_min());
            const Tested oDenmN(-oDenmP);
            ASSERT(Util::classify(oDenmP) == FP_SUBNORMAL);
            ASSERT(Util::classify(oDenmN) == FP_SUBNORMAL);

            ASSERT(Util::isNan(oNaNq)  == true);
            ASSERT(Util::isNan(oNaNs)  == true);
            ASSERT(Util::isNan(oInfP)  == false);
            ASSERT(Util::isNan(oInfN)  == false);
            ASSERT(Util::isNan(oNorm1) == false);
            ASSERT(Util::isNan(oNorm2) == false);
            ASSERT(Util::isNan(oZeroP) == false);
            ASSERT(Util::isNan(oZeroN) == false);
            ASSERT(Util::isNan(oDenmP) == false);
            ASSERT(Util::isNan(oDenmN) == false);

            ASSERT(Util::isInf(oNaNq)  == false);
            ASSERT(Util::isInf(oNaNs)  == false);
            ASSERT(Util::isInf(oInfP)  == true);
            ASSERT(Util::isInf(oInfN)  == true);
            ASSERT(Util::isInf(oNorm1) == false);
            ASSERT(Util::isInf(oNorm2) == false);
            ASSERT(Util::isInf(oZeroP) == false);
            ASSERT(Util::isInf(oZeroN) == false);
            ASSERT(Util::isInf(oDenmP) == false);
            ASSERT(Util::isInf(oDenmN) == false);

            ASSERT(Util::isFinite(oNaNq)  == false);
            ASSERT(Util::isFinite(oNaNs)  == false);
            ASSERT(Util::isFinite(oInfP)  == false);
            ASSERT(Util::isFinite(oInfN)  == false);
            ASSERT(Util::isFinite(oNorm1) == true);
            ASSERT(Util::isFinite(oNorm2) == true);
            ASSERT(Util::isFinite(oZeroP) == true);
            ASSERT(Util::isFinite(oZeroN) == true);
            ASSERT(Util::isFinite(oDenmP) == true);
            ASSERT(Util::isFinite(oDenmN) == true);

            ASSERT(Util::isNormal(oNaNq)  == false);
            ASSERT(Util::isNormal(oNaNs)  == false);
            ASSERT(Util::isNormal(oInfP)  == false);
            ASSERT(Util::isNormal(oInfN)  == false);
            ASSERT(Util::isNormal(oNorm1) == true);
            ASSERT(Util::isNormal(oNorm2) == true);
            ASSERT(Util::isNormal(oZeroP) == false);
            ASSERT(Util::isNormal(oZeroN) == false);
            ASSERT(Util::isNormal(oDenmP) == false);
            ASSERT(Util::isNormal(oDenmN) == false);

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal128" << bsl::endl;
        {
            typedef BDEC::Decimal128 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DL(x)

            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            ASSERT(Util::classify(oNaNq) == FP_NAN);
            ASSERT(Util::classify(oNaNs) == FP_NAN);

            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);
            ASSERT(Util::classify(oInfP) == FP_INFINITE);
            ASSERT(Util::classify(oInfN) == FP_INFINITE);

            const Tested oNorm1(NumLim::max());
            const Tested oNorm2(NumLim::min());
            ASSERT(Util::classify(oNorm1) == FP_NORMAL);
            ASSERT(Util::classify(oNorm2) == FP_NORMAL);

            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            ASSERT(Util::classify(oZeroP) == FP_ZERO);
            ASSERT(Util::classify(oZeroN) == FP_ZERO);

            const Tested oDenmP(NumLim::denorm_min());
            const Tested oDenmN(-oDenmP);
            ASSERT(Util::classify(oDenmP) == FP_SUBNORMAL);
            ASSERT(Util::classify(oDenmN) == FP_SUBNORMAL);

            ASSERT(Util::isNan(oNaNq)  == true);
            ASSERT(Util::isNan(oNaNs)  == true);
            ASSERT(Util::isNan(oInfP)  == false);
            ASSERT(Util::isNan(oInfN)  == false);
            ASSERT(Util::isNan(oNorm1) == false);
            ASSERT(Util::isNan(oNorm2) == false);
            ASSERT(Util::isNan(oZeroP) == false);
            ASSERT(Util::isNan(oZeroN) == false);
            ASSERT(Util::isNan(oDenmP) == false);
            ASSERT(Util::isNan(oDenmN) == false);

            ASSERT(Util::isInf(oNaNq)  == false);
            ASSERT(Util::isInf(oNaNs)  == false);
            ASSERT(Util::isInf(oInfP)  == true);
            ASSERT(Util::isInf(oInfN)  == true);
            ASSERT(Util::isInf(oNorm1) == false);
            ASSERT(Util::isInf(oNorm2) == false);
            ASSERT(Util::isInf(oZeroP) == false);
            ASSERT(Util::isInf(oZeroN) == false);
            ASSERT(Util::isInf(oDenmP) == false);
            ASSERT(Util::isInf(oDenmN) == false);

            ASSERT(Util::isFinite(oNaNq)  == false);
            ASSERT(Util::isFinite(oNaNs)  == false);
            ASSERT(Util::isFinite(oInfP)  == false);
            ASSERT(Util::isFinite(oInfN)  == false);
            ASSERT(Util::isFinite(oNorm1) == true);
            ASSERT(Util::isFinite(oNorm2) == true);
            ASSERT(Util::isFinite(oZeroP) == true);
            ASSERT(Util::isFinite(oZeroN) == true);
            ASSERT(Util::isFinite(oDenmP) == true);
            ASSERT(Util::isFinite(oDenmN) == true);

            ASSERT(Util::isNormal(oNaNq)  == false);
            ASSERT(Util::isNormal(oNaNs)  == false);
            ASSERT(Util::isNormal(oInfP)  == false);
            ASSERT(Util::isNormal(oInfN)  == false);
            ASSERT(Util::isNormal(oNorm1) == true);
            ASSERT(Util::isNormal(oNorm2) == true);
            ASSERT(Util::isNormal(oZeroP) == false);
            ASSERT(Util::isNormal(oZeroN) == false);
            ASSERT(Util::isNormal(oDenmP) == false);
            ASSERT(Util::isNormal(oDenmN) == false);

            #undef DECLIT
        }

        if (verbose2) bsl::cout << "Comparison functions" << bsl::endl;

        if (verbose3) bsl::cout << "Decimal32" << bsl::endl;
        {
            typedef BDEC::Decimal32 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DF(x)

            static const unsigned unorderedValuesSize = 2;
            static const Tested unorderedValues[unorderedValuesSize] = {
                NumLim::quiet_NaN(),
                NumLim::signaling_NaN()
            };

            static const unsigned orderedValuesSize = 4;
            static const Tested orderedValues[orderedValuesSize] = {
                NumLim::infinity(),
                NumLim::max(),
                DECLIT(0.0),
                NumLim::denorm_min()
            };

            // ordered and ordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             orderedValues[j]) == false);
                }
            }

            // unordered and ordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             orderedValues[j]) == true);
                }
            }

            // ordered and unordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            // unordered and unordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal64" << bsl::endl;
        {
            typedef BDEC::Decimal64 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DD(x)

            static const unsigned unorderedValuesSize = 2;
            static const Tested unorderedValues[unorderedValuesSize] = {
                NumLim::quiet_NaN(),
                NumLim::signaling_NaN()
            };

            static const unsigned orderedValuesSize = 4;
            static const Tested orderedValues[orderedValuesSize] = {
                NumLim::infinity(),
                NumLim::max(),
                DECLIT(0.0),
                NumLim::denorm_min()
            };

            // ordered and ordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             orderedValues[j]) == false);
                }
            }

            // unordered and ordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             orderedValues[j]) == true);
                }
            }

            // ordered and unordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            // unordered and unordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal128" << bsl::endl;
        {
            typedef BDEC::Decimal128 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DL(x)

            static const unsigned unorderedValuesSize = 2;
            static const Tested unorderedValues[unorderedValuesSize] = {
                NumLim::quiet_NaN(),
                NumLim::signaling_NaN()
            };

            static const unsigned orderedValuesSize = 4;
            static const Tested orderedValues[orderedValuesSize] = {
                NumLim::infinity(),
                NumLim::max(),
                DECLIT(0.0),
                NumLim::denorm_min()
            };

            // ordered and ordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             orderedValues[j]) == false);
                }
            }

            // unordered and ordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < orderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             orderedValues[j]) == true);
                }
            }

            // ordered and unordered
            for (unsigned i = 0; i < orderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(orderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            // unordered and unordered
            for (unsigned i = 0; i < unorderedValuesSize; ++i) {
                for (unsigned j = 0; j < unorderedValuesSize; ++j) {
                    ASSERT(Util::isUnordered(unorderedValues[i],
                                             unorderedValues[j]) == true);
                }
            }

            #undef DECLIT
        }

        if (verbose2) bsl::cout << "Rounding functions" << bsl::endl;

        if (verbose3) bsl::cout << "Decimal32" << bsl::endl;
        {
            typedef BDEC::Decimal32 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DF(x)

            const Tested anInt(1234567);
            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);

            ASSERT(Util::ceil(DECLIT( 0.5)) == DECLIT(1.0));
            ASSERT(Util::ceil(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::ceil(anInt) == anInt);
            ASSERT(Util::isNan(Util::ceil(oNaNq)));
            ASSERT(Util::isNan(Util::ceil(oNaNs)));
            ASSERT(Util::ceil(oZeroP) == oZeroP);
            ASSERT(Util::ceil(oZeroN) == oZeroN);
            ASSERT(Util::ceil(oInfP) == oInfP);
            ASSERT(Util::ceil(oInfN) == oInfN);

            ASSERT(Util::floor(DECLIT( 0.5)) == DECLIT( 0.0));
            ASSERT(Util::floor(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::floor(anInt) == anInt);
            ASSERT(Util::isNan(Util::floor(oNaNq)));
            ASSERT(Util::isNan(Util::floor(oNaNs)));
            ASSERT(Util::floor(oZeroP) == oZeroP);
            ASSERT(Util::floor(oZeroN) == oZeroN);
            ASSERT(Util::floor(oInfP) == oInfP);
            ASSERT(Util::floor(oInfN) == oInfN);

            ASSERT(Util::trunc(DECLIT( 0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(anInt) == anInt);
            ASSERT(Util::isNan(Util::trunc(oNaNq)));
            ASSERT(Util::isNan(Util::trunc(oNaNs)));
            ASSERT(Util::trunc(oZeroP) == oZeroP);
            ASSERT(Util::trunc(oZeroN) == oZeroN);
            ASSERT(Util::trunc(oInfP) == oInfP);
            ASSERT(Util::trunc(oInfN) == oInfN);

            ASSERT(Util::round(DECLIT( 0.5)) == DECLIT( 1.0));
            ASSERT(Util::round(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::round(anInt) == anInt);
            ASSERT(Util::isNan(Util::round(oNaNq)));
            ASSERT(Util::isNan(Util::round(oNaNs)));
            ASSERT(Util::round(oZeroP) == oZeroP);
            ASSERT(Util::round(oZeroN) == oZeroN);
            ASSERT(Util::round(oInfP) == oInfP);
            ASSERT(Util::round(oInfN) == oInfN);

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal64" << bsl::endl;
        {
            typedef BDEC::Decimal64 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DD(x)

            const Tested anInt(1234567890123456ull);
            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);

            ASSERT(Util::ceil(DECLIT( 0.5)) == DECLIT(1.0));
            ASSERT(Util::ceil(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::ceil(anInt) == anInt);
            ASSERT(Util::isNan(Util::ceil(oNaNq)));
            ASSERT(Util::isNan(Util::ceil(oNaNs)));
            ASSERT(Util::ceil(oZeroP) == oZeroP);
            ASSERT(Util::ceil(oZeroN) == oZeroN);
            ASSERT(Util::ceil(oInfP) == oInfP);
            ASSERT(Util::ceil(oInfN) == oInfN);

            ASSERT(Util::floor(DECLIT( 0.5)) == DECLIT( 0.0));
            ASSERT(Util::floor(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::floor(anInt) == anInt);
            ASSERT(Util::isNan(Util::floor(oNaNq)));
            ASSERT(Util::isNan(Util::floor(oNaNs)));
            ASSERT(Util::floor(oZeroP) == oZeroP);
            ASSERT(Util::floor(oZeroN) == oZeroN);
            ASSERT(Util::floor(oInfP) == oInfP);
            ASSERT(Util::floor(oInfN) == oInfN);

            ASSERT(Util::trunc(DECLIT( 0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(anInt) == anInt);
            ASSERT(Util::isNan(Util::trunc(oNaNq)));
            ASSERT(Util::isNan(Util::trunc(oNaNs)));
            ASSERT(Util::trunc(oZeroP) == oZeroP);
            ASSERT(Util::trunc(oZeroN) == oZeroN);
            ASSERT(Util::trunc(oInfP) == oInfP);
            ASSERT(Util::trunc(oInfN) == oInfN);

            ASSERT(Util::round(DECLIT( 0.5)) == DECLIT( 1.0));
            ASSERT(Util::round(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::round(anInt) == anInt);
            ASSERT(Util::isNan(Util::round(oNaNq)));
            ASSERT(Util::isNan(Util::round(oNaNs)));
            ASSERT(Util::round(oZeroP) == oZeroP);
            ASSERT(Util::round(oZeroN) == oZeroN);
            ASSERT(Util::round(oInfP) == oInfP);
            ASSERT(Util::round(oInfN) == oInfN);

            #undef DECLIT
        }

        if (verbose3) bsl::cout << "Decimal128" << bsl::endl;
        {
            typedef BDEC::Decimal128 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DL(x)

            const Tested anInt(DECLIT(1234567890123456789012345678901234.0));
            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);

            ASSERT(Util::ceil(DECLIT( 0.5)) == DECLIT(1.0));
            ASSERT(Util::ceil(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::ceil(anInt) == anInt);
            ASSERT(Util::isNan(Util::ceil(oNaNq)));
            ASSERT(Util::isNan(Util::ceil(oNaNs)));
            ASSERT(Util::ceil(oZeroP) == oZeroP);
            ASSERT(Util::ceil(oZeroN) == oZeroN);
            ASSERT(Util::ceil(oInfP) == oInfP);
            ASSERT(Util::ceil(oInfN) == oInfN);

            ASSERT(Util::floor(DECLIT( 0.5)) == DECLIT( 0.0));
            ASSERT(Util::floor(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::floor(anInt) == anInt);
            ASSERT(Util::isNan(Util::floor(oNaNq)));
            ASSERT(Util::isNan(Util::floor(oNaNs)));
            ASSERT(Util::floor(oZeroP) == oZeroP);
            ASSERT(Util::floor(oZeroN) == oZeroN);
            ASSERT(Util::floor(oInfP) == oInfP);
            ASSERT(Util::floor(oInfN) == oInfN);

            ASSERT(Util::trunc(DECLIT( 0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(DECLIT(-0.5)) == DECLIT(0.0));
            ASSERT(Util::trunc(anInt) == anInt);
            ASSERT(Util::isNan(Util::trunc(oNaNq)));
            ASSERT(Util::isNan(Util::trunc(oNaNs)));
            ASSERT(Util::trunc(oZeroP) == oZeroP);
            ASSERT(Util::trunc(oZeroN) == oZeroN);
            ASSERT(Util::trunc(oInfP) == oInfP);
            ASSERT(Util::trunc(oInfN) == oInfN);

            ASSERT(Util::round(DECLIT( 0.5)) == DECLIT( 1.0));
            ASSERT(Util::round(DECLIT(-0.5)) == DECLIT(-1.0));
            ASSERT(Util::round(anInt) == anInt);
            ASSERT(Util::isNan(Util::round(oNaNq)));
            ASSERT(Util::isNan(Util::round(oNaNs)));
            ASSERT(Util::round(oZeroP) == oZeroP);
            ASSERT(Util::round(oZeroN) == oZeroN);
            ASSERT(Util::round(oInfP) == oInfP);
            ASSERT(Util::round(oInfN) == oInfN);

            #undef DECLIT
        }


        if (verbose1) bsl::cout << "sameQuantum 64 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal64 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DL(x)

            const Tested anInt(DECLIT(1234567890123456789012345678901234.0));
            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);

            ASSERT(Util::sameQuantum(anInt, anInt));

            ASSERT(Util::sameQuantum(oNaNq, oNaNq));
            ASSERT(Util::sameQuantum(oNaNs, oNaNq));
            ASSERT(Util::sameQuantum(oNaNq, oNaNs));
            ASSERT(Util::sameQuantum(oNaNs, oNaNs));

            ASSERT(Util::sameQuantum(oZeroP, oZeroP));
            ASSERT(Util::sameQuantum(oZeroN, oZeroP));
            ASSERT(Util::sameQuantum(oZeroP, oZeroN));
            ASSERT(Util::sameQuantum(oZeroN, oZeroN));

            ASSERT(Util::sameQuantum(oInfP, oInfP));
            ASSERT(Util::sameQuantum(oInfN, oInfP));
            ASSERT(Util::sameQuantum(oInfP, oInfN));
            ASSERT(Util::sameQuantum(oInfN, oInfN));

            ASSERT(!Util::sameQuantum(oInfP,  oNaNq));
            ASSERT(!Util::sameQuantum(oNaNq,  oInfP));
            ASSERT(!Util::sameQuantum(oZeroP, oInfP));
            ASSERT(!Util::sameQuantum(oInfP,  oZeroP));
            ASSERT(!Util::sameQuantum(oNaNq,  anInt));

            ASSERT(Util::sameQuantum(Util::multiplyByPowerOf10(anInt, 4),
                                     Util::multiplyByPowerOf10(anInt, 4)));

            ASSERT(!Util::sameQuantum(Util::multiplyByPowerOf10(anInt, 5),
                                      Util::multiplyByPowerOf10(anInt, 4)));

            ASSERT(Util::quantum(Util::multiplyByPowerOf10(anInt, 5)) ==
                   Util::quantum(anInt) + 5);



        }

        if (verbose1) bsl::cout << "sameQuantum 128 tests..." << bsl::endl;
        {
            typedef BDEC::Decimal128 Tested;
            typedef bsl::numeric_limits<Tested> NumLim;
            #define DECLIT(x) BDLDFP_DECIMAL_DL(x)

            const Tested anInt(DECLIT(1234567890123456789012345678901234.0));
            const Tested oNaNq(NumLim::quiet_NaN());
            const Tested oNaNs(NumLim::signaling_NaN());
            const Tested oZeroP(DECLIT(0.0));
            const Tested oZeroN(DECLIT(-0.0));
            const Tested oInfP(NumLim::infinity());
            const Tested oInfN(-oInfP);

            ASSERT(Util::sameQuantum(anInt, anInt));

            ASSERT(Util::sameQuantum(oNaNq, oNaNq));
            ASSERT(Util::sameQuantum(oNaNs, oNaNq));
            ASSERT(Util::sameQuantum(oNaNq, oNaNs));
            ASSERT(Util::sameQuantum(oNaNs, oNaNs));

            ASSERT(Util::sameQuantum(oZeroP, oZeroP));
            ASSERT(Util::sameQuantum(oZeroN, oZeroP));
            ASSERT(Util::sameQuantum(oZeroP, oZeroN));
            ASSERT(Util::sameQuantum(oZeroN, oZeroN));

            ASSERT(Util::sameQuantum(oInfP, oInfP));
            ASSERT(Util::sameQuantum(oInfN, oInfP));
            ASSERT(Util::sameQuantum(oInfP, oInfN));
            ASSERT(Util::sameQuantum(oInfN, oInfN));

            ASSERT(!Util::sameQuantum(oInfP,  oNaNq));
            ASSERT(!Util::sameQuantum(oNaNq,  oInfP));
            ASSERT(!Util::sameQuantum(oZeroP, oInfP));
            ASSERT(!Util::sameQuantum(oInfP,  oZeroP));
            ASSERT(!Util::sameQuantum(oNaNq,  anInt));

            ASSERT(Util::sameQuantum(Util::multiplyByPowerOf10(anInt, 4),
                                     Util::multiplyByPowerOf10(anInt, 4)));

            ASSERT(!Util::sameQuantum(Util::multiplyByPowerOf10(anInt, 5),
                                      Util::multiplyByPowerOf10(anInt, 4)));

            ASSERT(Util::quantum(Util::multiplyByPowerOf10(anInt, 5)) ==
                   Util::quantum(anInt) + 5);



        }
    } break;

    default: {
        cerr << "WARNING: CASE '" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    // CONCERN: No memory came from the global or default allocator.

    LOOP2_ASSERT(test, globalAllocator.numBlocksTotal(),
                 0 == globalAllocator.numBlocksTotal());
    LOOP2_ASSERT(test, defaultAllocator.numBlocksTotal(),
                 0 == defaultAllocator.numBlocksTotal());

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }

    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
