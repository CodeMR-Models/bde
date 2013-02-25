// bslmf_assert.t.cpp                                                 -*-C++-*-

#include <bslmf_assert.h>
#include <bsls_compilerfeatures.h>
#include <bsls_platform.h>

#include <cstdlib>     // atoi()
#include <cstring>     // strcmp()
#include <iostream>

using namespace std;

//=============================================================================
//                                TEST PLAN
//-----------------------------------------------------------------------------
//                                Overview
//                                --------
//-----------------------------------------------------------------------------
// [ 1] BSLMF_ASSERT(expr)
//=============================================================================
//                  STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i) {
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}
#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
    if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__);}}

#define LOOP2_ASSERT(I,J,X) { \
    if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
        << J << "\n"; aSsErT(1, #X, __LINE__); } }

//=============================================================================
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number
#define T_() cout << '\t' << flush;           // Print tab w/o linefeed.

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

// namespace scope

BSLMF_ASSERT(sizeof(int) >= sizeof(char));

// un-named namespace
namespace {
    BSLMF_ASSERT(1);
#if defined(BSLS_PLATFORM_CMP_SUN)
    BSLMF_ASSERT(1);
#else
    BSLMF_ASSERT(1); BSLMF_ASSERT(1); // not class scope
#endif
}

namespace {
    BSLMF_ASSERT(1);
}

namespace Bar {

    BSLMF_ASSERT(1);
#if defined(BSLS_PLATFORM_CMP_SUN)
    BSLMF_ASSERT(1);
#else
    BSLMF_ASSERT(1); BSLMF_ASSERT(1); // not class scope
#endif
}

class MyType {
    // class scope

    BSLMF_ASSERT(sizeof(int) >= sizeof(char));
    BSLMF_ASSERT(sizeof(int) >= sizeof(char));

  public:
    int d_data;
    void foo();
};

void MyType::foo()
{
    // function scope

    BSLMF_ASSERT(sizeof(int) >= sizeof(char));
    BSLMF_ASSERT(1);

    using namespace BloombergLP;
    BSLMF_ASSERT(1);
}

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    // int veryVerbose = argc > 3;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:  // Zero is always the leading case.
      case 1: {
        // --------------------------------------------------------------------
        // TESTING BSLMF_ASSERT MACRO
        // We have the following concerns:
        //   1. Works for all non-zero integral values.
        //   2. Works in and out of class scope.
        //
        // Plan:
        //   Invoke the macro at namespace, class, and function scope (above)
        //   and verify that it does not cause a compiler error.  Please see
        //   the 'ttt' test package group for test cases where the macro
        //   should cause a compile-time error.
        //
        // Testing:
        //   BSLMF_ASSERT(expr)
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BSLMF_ASSERT Macro" << endl
                          << "==================" << endl;

        BSLMF_ASSERT(sizeof(int) >= sizeof(char));
        BSLMF_ASSERT(sizeof(int) >= sizeof(char));
        BSLMF_ASSERT(1);  ASSERT(138 == __LINE__);
        BSLMF_ASSERT(1);  ASSERT(139 == __LINE__);
        BSLMF_ASSERT(1 > 0 && 1);

// MSVC: __LINE__ macro breaks when /ZI is used (see Q199057 or KB199057)
// SUN:  BSLMF_ASSERT is defined the way that breaks this test
#if !defined(BSLS_PLATFORM_CMP_MSVC) &&                    \
    !defined(BSLS_PLATFORM_CMP_SUN)  &&                    \
    !defined(BSLS_COMPILERFEATURES_SUPPORT_STATIC_ASSERT)
        bslmf_Assert_138 t1; // test typedef name creation; matches above line
        bslmf_Assert_139 t2; // test typedef name creation; matches above line
        ASSERT(sizeof t1 == sizeof t2);  // use t1 and t2
#endif

        BSLMF_ASSERT(2);
        BSLMF_ASSERT(-1);

#if defined(BSLS_PLATFORM_CMP_SUN)
        BSLMF_ASSERT(1);
#else
        BSLMF_ASSERT(1); BSLMF_ASSERT(1); // not class scope
#endif

      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = "
             << testStatus << "." << endl;
    }
    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2013 Bloomberg L.P.
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
