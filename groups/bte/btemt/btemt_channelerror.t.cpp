// btemt_channelerror.t.cpp                                           -*-C++-*-

#include <btemt_channelerror.h>

#include <bcema_blob.h>
#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_testallocator.h>

#include <bsl_cstdlib.h>       // 'atoi'
#include <bsl_cstring.h>       // 'strcmp', 'memcmp', 'memcpy'
#include <bsl_ios.h>
#include <bsl_iostream.h>
#include <bsl_strstream.h>

#include <bsls_assert.h>
#include <bsls_asserttest.h>

using namespace BloombergLP;
using namespace bsl;
using namespace BloombergLP::btemt;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// The component under test implements a set of enumerations listing status
// codes returned from certain channel operations.  The enumerators in each
// enumeration do not start from 0 and are not sequential, because the status
// codes represented by the enumerators maybe written before their definition.
//
// We will therefore follow our standard 3-step approach to testing enumeration
// types, with certain exceptions:
//
//: o The test for 'BDEX' streaming ommitted because these operations are not
//:   defined.
//:
//: o The test for enumerator values does not verify that they are sequential.
// ----------------------------------------------------------------------------
// struct ChannelErrorRead
//
// TYPES
// [ 1] enum ChannelErrorRead::Enum { ... };
//
// CLASS METHODS
// [ 5] ostream& ChannelErrorRead::print(s, val, level = 0, sPL = 4);
// [ 1] const char *ChannelErrorRead::toAscii(val);
//
// FREE OPERATORS
// [ 3] operator<<(ostream& s, ChannelErrorRead::Enum val);
//
// struct ChannelErrorWrite
//
// TYPES
// [ 2] enum ChannelErrorRead::Enum { ... };
//
// CLASS METHODS
// [ 6] ostream& ChannelErrorRead::print(s, val, level = 0, sPL = 4);
// [ 2] const char *ChannelErrorRead::toAscii(val);
//
// FREE OPERATORS
// [ 4] operator<<(ostream& s, ChannelErrorWrite::Enum val);
// ----------------------------------------------------------------------------
// [ 7] USAGE EXAMPLE

// ============================================================================
//                    STANDARD BDE ASSERT TEST MACROS
// ----------------------------------------------------------------------------

static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}
# define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

// ============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
// ----------------------------------------------------------------------------

#define LOOP_ASSERT(I,X) {                                                    \
    if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__);}}

#define LOOP2_ASSERT(I,J,X) {                                                 \
    if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": "                 \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) {                                               \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t"     \
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) {                                             \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" <<  \
       #K << ": " << K << "\t" << #L << ": " << L << "\n";                    \
       aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) {                                           \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" <<  \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" <<                  \
       #M << ": " << M << "\n";                                               \
       aSsErT(1, #X, __LINE__); } }

#define LOOP0_ASSERT ASSERT
#define LOOP1_ASSERT LOOP_ASSERT

//=============================================================================
//                  STANDARD BDE VARIADIC ASSERT TEST MACROS
//-----------------------------------------------------------------------------

#define NUM_ARGS_IMPL(X5, X4, X3, X2, X1, X0, N, ...)   N
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0, "")

#define LOOPN_ASSERT_IMPL(N, ...) LOOP ## N ## _ASSERT(__VA_ARGS__)
#define LOOPN_ASSERT(N, ...)      LOOPN_ASSERT_IMPL(N, __VA_ARGS__)

#define ASSERTV(...) LOOPN_ASSERT(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

// ============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
// ----------------------------------------------------------------------------

#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // 'P(X)' without '\n'
#define T_ cout << "\t" << flush;             // Print tab w/o newline.
#define L_ __LINE__                           // current Line number

// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

// =========================================================================
//                       GLOBAL CONSTANTS FOR TESTING
// -------------------------------------------------------------------------

#define UNKNOWN_FORMAT "(* UNKNOWN *)"

// ============================================================================
//                     GLOBAL TYPEDEFS FOR TESTING
// ----------------------------------------------------------------------------

namespace {

class MyChannel {
    // This class represents a channel over which data may be sent and
    // received.

  public:
    //...
    int write(const bcema_Blob& blob);
        // Enqueue the specified 'blob' message to be written to this channel.
        // Return 0 on success, and a non-zero value otherwise.  On error, the
        // return value *may* equal to one of the enumerators in
        // 'btemt::ChannelErrorWrite::Enum'.
    //...
};

int MyChannel::write(const bcema_Blob& blob)
{
    (void)blob;
    return btemt::ChannelErrorWrite::BTEMT_SUCCESS;
}

}  // close unnamed namespace

// ============================================================================
//                            MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int                 test = argc > 1 ? atoi(argv[1]) : 0;
    bool             verbose = argc > 2;
    // bool         veryVerbose = argc > 3;
    // bool     veryVeryVerbose = argc > 4;
    bool veryVeryVeryVerbose = argc > 5;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 7: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Incorporate usage example from header into test driver, remove
        //:   leading comment characters, and replace 'assert' with 'ASSERT'.
        //:   (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "USAGE EXAMPLE" << endl
                          << "=============" << endl;

///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic Syntax
///- - - - - - - - - - - -
// Suppose that we have a class 'MyChannel' that defines the following elided
// interface:
//..

//..
// Further suppose that we have a 'bcema_Blob' object, 'blob', and a
// 'MyChannel' object, 'channel'.
        MyChannel channel;
        bslma::TestAllocator scratch("scratch", veryVeryVeryVerbose);
        bcema_Blob blob(&scratch);
//
// First, we use the 'write' method to write 'blob' into 'channel':
//..
        int rc = channel.write(blob);
//..
// Now, we compare the return code the enumerators in
// 'btemt::ChannelErrorRead::Enum' to handle the different error conditions:
//..
        switch(rc) {
          case btemt::ChannelErrorWrite::BTEMT_SUCCESS:  // Success
          {
              // ...
          } break;

          // We handle the different types of failures in the cases below.

          case btemt::ChannelErrorWrite::BTEMT_CACHE_HIWAT:
          {
              // ...
          } break;
          case btemt::ChannelErrorWrite::BTEMT_HIT_CACHE_HIWAT:
          {
              // ...
          } break;
          case btemt::ChannelErrorWrite::BTEMT_CHANNEL_DOWN:
          {
              // ...
          } break;
          default:  // Handle other failures.
          {
              //...
          }
        };
//..
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // 'ChannelErrorWrite::print'
        //
        // Concerns:
        //: 1 The 'print' method writes the output to the specified stream.
        //:
        //: 2 The 'print' method writes the string representation of each
        //:   enumerator in the intended format.
        //:
        //: 3 The 'print' method writes a distinguished string when passed an
        //:   out-of-band value.
        //:
        //: 4 There is no output when the stream is invalid.
        //:
        //: 5 The 'print' method has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator, some out-of-band
        //:   values, and a range of level and spacing per level values that
        //:   can be specified to the 'print' method:
        //:
        //:   1 Verify that the 'print' method produces the expected results
        //:     (C-1..3)
        //:
        //:   2 Verify that there is no output when the stream is invalid.
        //:     (C-4)
        //:
        //: 2 Take the address of the 'print' (class) method and use the
        //:   result to initialize a variable of the appropriate type.  (C-5)
        //
        // Testing:
        //   ostream& ChannelErrorWrite::print(s, val, level = 0, sPL = 4);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "'ChannelErrorWrite::print'" << endl
                          << "==========================" << endl;

        typedef ChannelErrorWrite Obj;

        static const struct {
            int         d_lineNum;        // source line number
            int         d_level;          // level
            int         d_spacePerLevel;  // spaces per level
            Obj::Enum   d_enumerator;     // enumerator
            const char *d_expString;      // expected ascii value
        } DATA[] = {
#define NL "\n"
          // line level spl enumerator                  result
          // ---- ----- --- --------------------------- -------------------
          {  L_,     0,  4, Obj::BTEMT_SUCCESS,         "SUCCESS"NL          },
          {  L_,     0,  4, Obj::BTEMT_CACHE_HIWAT,     "CACHE_HIWAT"NL      },
          {  L_,     0,  4, Obj::BTEMT_HIT_CACHE_HIWAT, "HIT_CACHE_HIWAT"NL  },
          {  L_,     0,  4, Obj::BTEMT_CHANNEL_DOWN,    "CHANNEL_DOWN"NL     },
          {  L_,     0,  4, Obj::BTEMT_ENQUEUE_WAT,     "ENQUEUE_WAT"NL      },
          {  L_,     0,  4, Obj::BTEMT_UNKNOWN_ID,      "UNKNOWN_ID"NL       },
          {  L_,     0,  4, (Obj::Enum) 1,              UNKNOWN_FORMAT NL    },

          {  L_,     0,  0, Obj::BTEMT_SUCCESS,         "SUCCESS"NL          },
          {  L_,     0, -1, Obj::BTEMT_SUCCESS,         "SUCCESS"            },
          {  L_,     0,  2, Obj::BTEMT_SUCCESS,         "SUCCESS"NL          },
          {  L_,     1,  1, Obj::BTEMT_SUCCESS,         " SUCCESS"NL         },
          {  L_,     1,  2, Obj::BTEMT_SUCCESS,         "  SUCCESS"NL        },
          {  L_,    -1,  2, Obj::BTEMT_SUCCESS,         "SUCCESS"NL          },
          {  L_,    -2,  1, Obj::BTEMT_SUCCESS,         "SUCCESS"NL          },
          {  L_,     2,  1, Obj::BTEMT_SUCCESS,         "  SUCCESS"NL        },
          {  L_,     1,  3, Obj::BTEMT_SUCCESS,         "   SUCCESS"NL       },
#undef NL
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;         // big enough to hold output string
        const char  XX   = (char)0xFF;  // value of an unset 'char'
              char  buf[SIZE];          // output buffer

              char  mCtrl[SIZE];  memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE  = DATA[ti].d_lineNum;
            const int        LEVEL = DATA[ti].d_level;
            const int        SPL   = DATA[ti].d_spacePerLevel;
            const Obj::Enum  ENUM  = DATA[ti].d_enumerator;
            const char      *STR   = DATA[ti].d_expString;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);

            Obj::print(out, ENUM, LEVEL, SPL) << ends;


            const int SZ = strlen(STR) + 1;

            // Verify that the buffer is large enough.
            ASSERTV(LINE, ti, SZ  < SIZE);

            ASSERTV(LINE, ti, buf, STR, 0 == memcmp(buf, STR, SZ));

            // Check for overrun.
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
            ASSERTV(LINE, ti,  0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));

            // Repeat for 'print' default arguments.
            if (0 == LEVEL && 4 == SPL) {

                memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char'
                                          // values.

                ostrstream out(buf, sizeof buf);
                Obj::print(out, ENUM) << ends;

                ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

                // Check for overrun.
                ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
                ASSERTV(LINE, ti,
                        0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));
            }
        }

        // Verify that nothing is written to a bad stream.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE  = DATA[ti].d_lineNum;
            const int        LEVEL = DATA[ti].d_level;
            const int        SPL   = DATA[ti].d_spacePerLevel;
            const Obj::Enum  ENUM  = DATA[ti].d_enumerator;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);  out.setstate(ios::badbit);
            Obj::print(out, ENUM, LEVEL, SPL);

            LOOP2_ASSERT(LINE, ti, 0 == memcmp(buf, CTRL, SIZE));
        }

        // Verify 'print' signature.
        {
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&,
                                             Obj::Enum,
                                             int,
                                             int);

            const FuncPtr FP = &Obj::print;
            (void) FP;  // quash potential compiler warning
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // 'ChannelErrorRead::print'
        //
        // Concerns:
        //: 1 The 'print' method writes the output to the specified stream.
        //:
        //: 2 The 'print' method writes the string representation of each
        //:   enumerator in the intended format.
        //:
        //: 3 The 'print' method writes a distinguished string when passed an
        //:   out-of-band value.
        //:
        //: 4 There is no output when the stream is invalid.
        //:
        //: 5 The 'print' method has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator, some out-of-band
        //:   values, and a range of level and spacing per level values that
        //:   can be specified to the 'print' method:
        //:
        //:   1 Verify that the 'print' method produces the expected results
        //:     (C-1..3)
        //:
        //:   2 Verify that there is no output when the stream is invalid.
        //:     (C-4)
        //:
        //: 2 Take the address of the 'print' (class) method and use the
        //:   result to initialize a variable of the appropriate type.  (C-5)
        //
        // Testing:
        //   ostream& ChannelErrorRead::print(s, val, level = 0, sPL = 4);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "'ChannelErrorRead::print'" << endl
                          << "=========================" << endl;

        typedef ChannelErrorRead Obj;

        static const struct {
            int         d_lineNum;        // source line number
            int         d_level;          // level
            int         d_spacePerLevel;  // spaces per level
            Obj::Enum   d_enumerator;     // enumerator
            const char *d_expString;      // expected ascii value
        } DATA[] = {
#define NL "\n"
            // line level spl enumerator                 result
            // ---- ----- --- -------------------------- ------------------
            {  L_,     0,  4, Obj::BTEMT_SUCCESS,        "SUCCESS"NL         },
            {  L_,     0,  4, Obj::BTEMT_CHANNEL_CLOSED, "CHANNEL_CLOSED"NL  },
            {  L_,     0,  4, (Obj::Enum) 1,             UNKNOWN_FORMAT NL   },

            {  L_,     0,  0, Obj::BTEMT_SUCCESS,        "SUCCESS"NL         },
            {  L_,     0, -1, Obj::BTEMT_SUCCESS,        "SUCCESS"           },
            {  L_,     0,  2, Obj::BTEMT_SUCCESS,        "SUCCESS"NL         },
            {  L_,     1,  1, Obj::BTEMT_SUCCESS,        " SUCCESS"NL        },
            {  L_,     1,  2, Obj::BTEMT_SUCCESS,        "  SUCCESS"NL       },
            {  L_,    -1,  2, Obj::BTEMT_SUCCESS,        "SUCCESS"NL         },
            {  L_,    -2,  1, Obj::BTEMT_SUCCESS,        "SUCCESS"NL         },
            {  L_,     2,  1, Obj::BTEMT_SUCCESS,        "  SUCCESS"NL       },
            {  L_,     1,  3, Obj::BTEMT_SUCCESS,        "   SUCCESS"NL      },
#undef NL
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;         // big enough to hold output string
        const char  XX   = (char)0xFF;  // value of an unset 'char'
              char  buf[SIZE];          // output buffer

              char  mCtrl[SIZE];  memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE  = DATA[ti].d_lineNum;
            const int        LEVEL = DATA[ti].d_level;
            const int        SPL   = DATA[ti].d_spacePerLevel;
            const Obj::Enum  ENUM  = DATA[ti].d_enumerator;
            const char      *STR   = DATA[ti].d_expString;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);

            Obj::print(out, ENUM, LEVEL, SPL) << ends;


            const int SZ = strlen(STR) + 1;

            // Verify that the buffer is large enough.
            ASSERTV(LINE, ti, SZ  < SIZE);

            ASSERTV(LINE, ti, buf, STR, 0 == memcmp(buf, STR, SZ));

            // Check for overrun.
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
            ASSERTV(LINE, ti,  0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));

            // Repeat for 'print' default arguments.
            if (0 == LEVEL && 4 == SPL) {

                memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char'
                                          // values.

                ostrstream out(buf, sizeof buf);
                Obj::print(out, ENUM) << ends;

                ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

                // Check for overrun.
                ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
                ASSERTV(LINE, ti,
                        0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));
            }
        }

        // Verify that nothing is written to a bad stream.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE  = DATA[ti].d_lineNum;
            const int        LEVEL = DATA[ti].d_level;
            const int        SPL   = DATA[ti].d_spacePerLevel;
            const Obj::Enum  ENUM  = DATA[ti].d_enumerator;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);  out.setstate(ios::badbit);
            Obj::print(out, ENUM, LEVEL, SPL);

            LOOP2_ASSERT(LINE, ti, 0 == memcmp(buf, CTRL, SIZE));
        }

        // Verify 'print' signature.
        {
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&,
                                             Obj::Enum,
                                             int,
                                             int);

            const FuncPtr FP = &Obj::print;
            (void) FP;  // quash potential compiler warning
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // '<<' OPERATOR FOR 'ChannelErrorWrite::Enum'
        //
        // Concerns:
        //: 1 The '<<' operator writes the output to the specified stream.
        //:   (P-1)
        //:
        //: 2 The '<<' operator writes the string representation of each
        //:   enumerator in the intended format.
        //:
        //: 3 The '<<' operator writes a distinguished string when passed an
        //:   out-of-band value.
        //:
        //: 4 The output produced by 'stream << value' is the same as that
        //:   produced by 'Obj::print(stream, value, 0, -1)'.
        //:
        //: 5 There is no output when the stream is invalid.
        //:
        //: 6 The '<<' operator has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator and some
        //:   out-of-band values:
        //:
        //:   1 Verify that the '<<' operator produces the expected results.
        //:     (C-1..3)
        //:
        //:   2 Verify that 'Obj::print(stream, value, 0, -1)' produces the
        //:     same results.  (C-4)
        //:
        //:   3 Verify that there is no output when stream is invalid.  (C-5)
        //:
        //: 5 Take the address of the '<<' (free) operator and use the result
        //:   to initialize a variable of the appropriate type.  (C-6)
        //
        // Testing:
        //   operator<<(ostream& s, ChannelErrorWrite::Enum val);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                      << "'<<' OPERATOR FOR 'ChannelErrorWrite::Enum'" << endl
                      << "===========================================" << endl;

        typedef ChannelErrorWrite Obj;

        static const struct {
            int         d_lineNum;     // source line number
            Obj::Enum   d_enumerator;  // enumerator
            const char *d_expString;   // expected ascii value
        } DATA[] = {
            // line  enumerator                   result
            // ----  ---------------------------  -----------------
            {  L_,   Obj::BTEMT_SUCCESS,          "SUCCESS"          },
            {  L_,   Obj::BTEMT_CACHE_HIWAT,      "CACHE_HIWAT"      },
            {  L_,   Obj::BTEMT_HIT_CACHE_HIWAT,  "HIT_CACHE_HIWAT"  },
            {  L_,   Obj::BTEMT_CHANNEL_DOWN,     "CHANNEL_DOWN"     },
            {  L_,   Obj::BTEMT_ENQUEUE_WAT,      "ENQUEUE_WAT"      },
            {  L_,   Obj::BTEMT_UNKNOWN_ID,       "UNKNOWN_ID"       },

            {  L_,   (Obj::Enum) 1,              UNKNOWN_FORMAT    },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;         // big enough to hold output string
        const char  XX   = (char)0xFF;  // value of an unset 'char'
              char  buf[SIZE];          // output buffer

              char  mCtrl[SIZE];  memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE = DATA[ti].d_lineNum;
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;
            const char      *STR  = DATA[ti].d_expString;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);

            out << ENUM << ends;

            const int SZ = strlen(STR) + 1;

            // Verify that the buffer is large enough.
            ASSERTV(LINE, ti, SZ  < SIZE);

            ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

            // Check for overrun.
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
            ASSERTV(LINE, ti,  0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));


            // Verify that 'Obj::print(stream, value, 0, -1)' produces the same
            // results.
            {
                memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char'
                                          // values.

                ostrstream out(buf, sizeof buf);
                Obj::print(out, ENUM, 0, -1) << ends;

                ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

                // Check for overrun.
                ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
                ASSERTV(LINE, ti,
                        0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));
            }
        }

        // Verify that nothing is written to a bad stream.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE = DATA[ti].d_lineNum;
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);  out.setstate(ios::badbit);
            out << ENUM;

            LOOP2_ASSERT(LINE, ti, 0 == memcmp(buf, CTRL, SIZE));
        }

        // Verify '<<' operator signature.
        {
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&, Obj::Enum);

            const FuncPtr FP = &operator<<;
            (void) FP;  // quash potential compiler warning
        }

      } break;
      case 3: {
        // --------------------------------------------------------------------
        // '<<' OPERATOR FOR 'ChannelErrorRead::Enum'
        //
        // Concerns:
        //: 1 The '<<' operator writes the output to the specified stream.
        //:   (P-1)
        //:
        //: 2 The '<<' operator writes the string representation of each
        //:   enumerator in the intended format.
        //:
        //: 3 The '<<' operator writes a distinguished string when passed an
        //:   out-of-band value.
        //:
        //: 4 The output produced by 'stream << value' is the same as that
        //:   produced by 'Obj::print(stream, value, 0, -1)'.
        //:
        //: 5 There is no output when the stream is invalid.
        //:
        //: 6 The '<<' operator has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator and some
        //:   out-of-band values:
        //:
        //:   1 Verify that the '<<' operator produces the expected results.
        //:     (C-1..3)
        //:
        //:   2 Verify that 'Obj::print(stream, value, 0, -1)' produces the
        //:     same results.  (C-4)
        //:
        //:   3 Verify that there is no output when stream is invalid.  (C-5)
        //:
        //: 5 Take the address of the '<<' (free) operator and use the result
        //:   to initialize a variable of the appropriate type.  (C-6)
        //
        // Testing:
        //   operator<<(ostream& s, ChannelErrorRead::Enum val);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                       << "'<<' OPERATOR FOR 'ChannelErrorRead::Enum'" << endl
                       << "==========================================" << endl;

        typedef ChannelErrorRead Obj;

        static const struct {
            int         d_lineNum;     // source line number
            Obj::Enum   d_enumerator;  // enumerator
            const char *d_expString;   // expected ascii value
        } DATA[] = {
            // line  enumerator                  result
            // ----  --------------------------  ----------------
            {  L_,   Obj::BTEMT_SUCCESS,         "SUCCESS"         },
            {  L_,   Obj::BTEMT_CHANNEL_CLOSED,  "CHANNEL_CLOSED"  },
            {  L_,   (Obj::Enum) 1,              UNKNOWN_FORMAT    },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;         // big enough to hold output string
        const char  XX   = (char)0xFF;  // value of an unset 'char'
              char  buf[SIZE];          // output buffer

              char  mCtrl[SIZE];  memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE = DATA[ti].d_lineNum;
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;
            const char      *STR  = DATA[ti].d_expString;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);

            out << ENUM << ends;

            const int SZ = strlen(STR) + 1;

            // Verify that the buffer is large enough.
            ASSERTV(LINE, ti, SZ  < SIZE);

            ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

            // Check for overrun.
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
            ASSERTV(LINE, ti,  0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));


            // Verify that 'Obj::print(stream, value, 0, -1)' produces the same
            // results.
            {
                memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char'
                                          // values.

                ostrstream out(buf, sizeof buf);
                Obj::print(out, ENUM, 0, -1) << ends;

                ASSERTV(LINE, ti,  0 == memcmp(buf, STR, SZ));

                // Check for overrun.
                ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
                ASSERTV(LINE, ti,
                        0 == memcmp(buf + SZ, CTRL + SZ, SIZE - SZ));
            }
        }

        // Verify that nothing is written to a bad stream.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const int        LINE = DATA[ti].d_lineNum;
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;

            memcpy(buf, CTRL, SIZE);  // Preset 'buf' to unset 'char' values.

            ostrstream out(buf, sizeof buf);  out.setstate(ios::badbit);
            out << ENUM;

            LOOP2_ASSERT(LINE, ti, 0 == memcmp(buf, CTRL, SIZE));
        }

        // Verify '<<' operator signature.
        {
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&, Obj::Enum);

            const FuncPtr FP = &operator<<;
            (void) FP;  // quash potential compiler warning
        }

      } break;
      case 2: {
        // --------------------------------------------------------------------
        // 'ChannelErrorWrite::Enum' AND 'ChannelErrorWrite::toAscii'
        //
        // Concerns:
        //: 1 The enumerators have the correct values.
        //:
        //: 2 The 'toAscii' method returns the expected string representation
        //:   for each enumerator.
        //:
        //: 3 The 'toAscii' method returns a distinguished string when passed
        //:   an out-of-band value.
        //:
        //: 4 The string returned by 'toAscii' is non-modifiable.
        //:
        //: 5 The 'toAscii' method has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator:
        //:
        //:   1 Verify that it has the correct value.  (C-1)
        //:
        //:   2 Verify that the 'toAscii' method returns the expected string.
        //:     (C-2)
        //:
        //: 3 Verify that the 'toAscii' method returns a distinguished string
        //:   when passed an out-of-band value.  (C-3)
        //:
        //: 4 Take the address of the 'toAscii' (class) method and use the
        //:   result to initialize a variable of the appropriate type.
        //:   (C-4, C-5)
        //
        // Testing:
        //   enum ChannelErrorWrite::Enum { ... };
        //   const char *ChannelErrorWrite::toAscii(val);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                << "'ChannelErrorWrite::Enum' AND 'ChannelErrorWrite::toAscii'"
                << endl
                << "=========================================================="
                << endl;

        typedef ChannelErrorWrite Obj;

        static const struct {
            int         d_lineNum;     // source line number
            Obj::Enum   d_enumerator;  // enumerator
            int         d_expValue;    // expected value
            const char *d_expString;   // expected ascii value
        } DATA[] = {
            // line  enumerator                   value  ascii
            // ----  ---------------------------  -----  -----------------
            {  L_,   Obj::BTEMT_SUCCESS,             0,  "SUCCESS"          },
            {  L_,   Obj::BTEMT_CACHE_HIWAT,        -1,  "CACHE_HIWAT"      },
            {  L_,   Obj::BTEMT_HIT_CACHE_HIWAT,    -2,  "HIT_CACHE_HIWAT"  },
            {  L_,   Obj::BTEMT_CHANNEL_DOWN,       -3,  "CHANNEL_DOWN"     },
            {  L_,   Obj::BTEMT_ENQUEUE_WAT,        -4,  "ENQUEUE_WAT"      },
            {  L_,   Obj::BTEMT_UNKNOWN_ID,         -5,  "UNKNOWN_ID"       },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;
            const int        VAL  = DATA[ti].d_expValue;
            const char      *STR  = DATA[ti].d_expString;

            ASSERTV(ENUM, VAL, VAL == ENUM);

            const char *result = Obj::toAscii(ENUM);

            ASSERTV(STR, result, 0 == strcmp(result, STR));
        }

        // Verify out-of-band values.
        const char *result = Obj::toAscii(static_cast<Obj::Enum>(1));
        ASSERTV(result, 0 == strcmp(UNKNOWN_FORMAT, result));


        // Verify 'toAscii' signature.
        {
            typedef const char *(*FuncPtr)(Obj::Enum);

            const FuncPtr FP = &Obj::toAscii;
            (void) FP;  // quash potential compiler warning
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // 'ChannelErrorRead::Enum' AND 'ChannelErrorRead::toAscii'
        //
        // Concerns:
        //: 1 The enumerators have the correct values.
        //:
        //: 2 The 'toAscii' method returns the expected string representation
        //:   for each enumerator.
        //:
        //: 3 The 'toAscii' method returns a distinguished string when passed
        //:   an out-of-band value.
        //:
        //: 4 The string returned by 'toAscii' is non-modifiable.
        //:
        //: 5 The 'toAscii' method has the expected signature.
        //
        // Plan:
        //: 1 Use table-based approach, for each enumerator:
        //:
        //:   1 Verify that it has the correct value.  (C-1)
        //:
        //:   2 Verify that the 'toAscii' method returns the expected string.
        //:     (C-2)
        //:
        //: 3 Verify that the 'toAscii' method returns a distinguished string
        //:   when passed an out-of-band value.  (C-3)
        //:
        //: 4 Take the address of the 'toAscii' (class) method and use the
        //:   result to initialize a variable of the appropriate type.
        //:   (C-4, C-5)
        //
        // Testing:
        //   enum ChannelErrorRead::Enum { ... };
        //   const char *ChannelErrorRead::toAscii(val);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                  << "'ChannelErrorRead::Enum' AND 'ChannelErrorRead::toAscii'"
                  << endl
                  << "========================================================"
                  << endl;

        typedef ChannelErrorRead Obj;

        static const struct {
            int         d_lineNum;     // source line number
            Obj::Enum   d_enumerator;  // enumerator
            int         d_expValue;    // expected value
            const char *d_expString;   // expected ascii value
        } DATA[] = {
            // line  enumerator                  value  ascii
            // ----  --------------------------  -----  ----------------
            {  L_,   Obj::BTEMT_SUCCESS,            0,  "SUCCESS"         },
            {  L_,   Obj::BTEMT_CHANNEL_CLOSED,    -2,  "CHANNEL_CLOSED"  },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        // Verify enumerators.
        for (int ti = 0; ti < NUM_DATA; ++ti)
        {
            const Obj::Enum  ENUM = DATA[ti].d_enumerator;
            const int        VAL  = DATA[ti].d_expValue;
            const char      *STR  = DATA[ti].d_expString;

            ASSERTV(ENUM, VAL, VAL == ENUM);

            const char *result = Obj::toAscii(ENUM);

            ASSERTV(STR, result, 0 == strcmp(result, STR));
        }

        // Verify out-of-band values.
        const char *result = Obj::toAscii(static_cast<Obj::Enum>(1));
        ASSERTV(result, 0 == strcmp(UNKNOWN_FORMAT, result));


        // Verify 'toAscii' signature.
        {
            typedef const char *(*FuncPtr)(Obj::Enum);

            const FuncPtr FP = &Obj::toAscii;
            (void) FP;  // quash potential compiler warning
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

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2011
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
