// bdes_platform.t.cpp                                                -*-C++-*-

#include <bdes_platform.h>

#include <bsl_cstdlib.h>     // atoi()
#include <bsl_iostream.h>

using namespace BloombergLP;
using namespace bsl;  // automatically added by script

//==========================================================================
//                  STANDARD BDE ASSERT TEST MACRO
//--------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }
//--------------------------------------------------------------------------

//==========================================================================
//                             TEST PLAN
//--------------------------------------------------------------------------
//                            * Overview *
// Since this component implements CPP macro's, which may or may not be
// defined, there is not too much to test in this driver.  Since correctness
// will be affected by compile-time switches during the build process,
// any compile-time tests we come up with should probably reside directly in
// the header or implementation file.
//--------------------------------------------------------------------------
// [ 1] Ensure that exactly one of each CMP type is set.
// [ 1] Ensure that exactly one of each OS type is set.
// [ 1] For each category, ensure MINOR_NUMBER set -> MAJOR_NUMBER set.
// [ 1] For the OS, type ensure MAJOR_NUMBER set -> SUBTYPE set.
//==========================================================================

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 1: {
        // --------------------------------------------------------
        // MINIMAL DEFINITION TEST:
        //   We want to make sure that exactly one each of OS, PROCESSOR, and
        //   COMPILER type is set.  We also want to make sure that at most one
        //   subtype of OS is set.  Finally we want to make sure that a minor
        //   version implies a major version, and that for OS, a major version
        //   implies a subtype.
        // Testing:
        //   Ensure that exactly one of each OS type is set.
        //   Ensure that exactly one of each CMP type is set.
        //   Ensure that exactly one of each CPU type is set.
        //   Ensure that at most one of each CPU subtype is set.
        //   For each category, ensure MINOR_NUMBER set -> MAJOR_NUMBER set.
        //   For the OS type, ensure MAJOR_NUMBER set -> SUBTYPE set.
        // --------------------------------------------------------

        if (verbose) cout << endl << "Minimal Definition Test" <<
                             endl << "-----------------------" << endl;

        // Compile-time tests for this case are located in the validation
        // section of this component's header file.

        struct OsCpu {
            bsls::Platform::Os  osType;
            bsls::Platform::Cpu cpuType;
        };

        if (verbose) {

            cout << endl << "Print Defined Symbols:" << endl;

            cout << endl << "Print CMP-related Symbols:" << endl;

#define MACRO_TESTEQ(X, Y) \
        ASSERT(Y == X);\
        cout << "\t"#X" = " << X << endl;

#define MACRO_TESTGT(X, Y) \
        ASSERT(Y <= X);\
        cout << "\t"#X" = " << X << endl;

            #if defined(BDES_PLATFORM__CMP_AIX)
                MACRO_TESTGT(BDES_PLATFORM__CMP_AIX, 0);
            #endif
            #if defined(BDES_PLATFORM__CMP_GNU)
                MACRO_TESTGT(BDES_PLATFORM__CMP_GNU, 0);
            #endif
            #if defined(BDES_PLATFORM__CMP_HP)
                MACRO_TESTGT(BDES_PLATFORM__CMP_HP, 0);
            #endif
            #if defined(BDES_PLATFORM__CMP_MSVC)
                MACRO_TESTGT(BDES_PLATFORM__CMP_MSVC, 0);
            #endif
            #if defined(BDES_PLATFORM__CMP_SUN)
                MACRO_TESTGT(BDES_PLATFORM__CMP_SUN, 0);
            #endif

            cout << endl << "Print OS-related Symbols:" << endl;

            #if defined(BDES_PLATFORM__OS_UNIX)
                MACRO_TESTEQ(BDES_PLATFORM__OS_UNIX, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_AIX)
                MACRO_TESTEQ(BDES_PLATFORM__OS_AIX, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_HPUX)
                MACRO_TESTEQ(BDES_PLATFORM__OS_HPUX, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_LINUX)
                MACRO_TESTEQ(BDES_PLATFORM__OS_LINUX, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_SOLARIS)
                MACRO_TESTEQ(BDES_PLATFORM__OS_SOLARIS, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_SUNOS)
                MACRO_TESTEQ(BDES_PLATFORM__OS_SUNOS, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_WINDOWS)
                MACRO_TESTEQ(BDES_PLATFORM__OS_WINDOWS, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_WIN9X)
                MACRO_TESTEQ(BDES_PLATFORM__OS_WIN9X, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_WINNT)
                MACRO_TESTEQ(BDES_PLATFORM__OS_WINNT, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_WIN2K)
                MACRO_TESTEQ(BDES_PLATFORM__OS_WIN2K, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_WINXP)
                MACRO_TESTEQ(BDES_PLATFORM__OS_WINXP, 1);
            #endif
            #if defined(BDES_PLATFORM__OS_VER_MAJOR)
                MACRO_TESTGT(BDES_PLATFORM__OS_VER_MAJOR, 0);
            #endif
            #if defined(BDES_PLATFORM__OS_VER_MINOR)
                MACRO_TESTGT(BDES_PLATFORM__OS_VER_MINOR, 0);
            #endif
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
//      Copyright (C) Bloomberg L.P., 2002
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
