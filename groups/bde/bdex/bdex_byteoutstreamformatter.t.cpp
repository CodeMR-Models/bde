// bdex_byteoutstreamformatter.t.cpp             -*-C++-*-

#include <bdex_byteoutstreamformatter.h>

#include <bdex_outstreamfunctions.h>            // for testing only
#include <bdex_instreamfunctions.h>             // for testing only

#include <bdesb_fixedmemoutstreambuf.h>
#include <bdesb_fixedmemoutput.h>

#include <bslma_testallocator.h>
#include <bsls_stopwatch.h>
#include <bsls_types.h>

#include <bsl_cstdlib.h>     // atoi()
#include <bsl_cstring.h>     // memcpy(), memcmp(), strlen()
#include <bsl_iostream.h>
#include <bsl_strstream.h>

using namespace BloombergLP;
using namespace bsl;  // automatically added by script


//=============================================================================
//                              TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// For all output methods in 'bdex_ByteOutStreamFormatter', the formatting of
// the input value to its correct byte representation is delegated to another
// component.  We assume that this formatting has been rigorously tested and
// verified.  Therefore, we are concerned only with the proper placement and
// alignment of bytes in the output stream.  We verify these properties by
// inserting chosen "marker" bytes between each output method call, and ensure
// that the new output bytes are properly interleaved between the "marker"
// bytes.
//
// We have chosen the primary black-box manipulator for
// 'bdex_ByteOutStreamFormatter' to be 'putInt8'.
//-----------------------------------------------------------------------------
// [ 2] bdex_ByteOutStreamFormatter(bsl::streambuf *);
// [ 2] ~bdex_ByteOutStreamFormatter();
// [24] bdex_ByteOutStreamFormatter& flush();
// [23] void invalidate();
// [22] putLength(int length);
// [22] putVersion(int version);
// [ 9] putInt64(bsls::Types::Int64 value);
// [ 9] putUint64(bsls::Types::Int64 value);
// [ 8] putInt56(bsls::Types::Int64 value);
// [ 8] putUint56(bsls::Types::Int64 value);
// [ 7] putInt48(bsls::Types::Int64 value);
// [ 8] putUint48(bsls::Types::Int64 value);
// [ 7] putInt40(bsls::Types::Int64 value);
// [ 7] putUint40(bsls::Types::Int64 value);
// [ 5] putInt32(int value);
// [ 5] putUint32(int value);
// [ 4] putInt24(int value);
// [ 4] putUint24(int value);
// [ 3] putInt16(int value);
// [ 3] putUint16(int value);
// [ 2] putInt8(int value);
// [ 2] putUint8(int value);
// [11] putFloat64(double value);
// [10] putFloat32(float value);
// [19] putArrayInt64(const bsls::Types::Int64 *array, int count);
// [19] putArrayUint64(const bsls::Types::Uint64 *array, int count);
// [18] putArrayInt56(const bsls::Types::Int64 *array, int count);
// [18] putArrayUint56(const bsls::Types::Uint64 *array, int count);
// [17] putArrayInt48(const bsls::Types::Int64 *array, int count);
// [17] putArrayUint48(const bsls::Types::Uint64 *array, int count);
// [16] putArrayInt40(const bsls::Types::Int64 *array, int count);
// [16] putArrayUint40(const bsls::Types::Uint64 *array, int count);
// [15] putArrayInt32(const int *array, int count);
// [15] putArrayUint32(const unsigned int *array, int count);
// [14] putArrayInt24(const int *array, int count);
// [14] putArrayUint24(const unsigned int *array, int count);
// [13] putArrayInt16(const short *array, int count);
// [13] putArrayUint16(const unsigned short *array, int count);
// [12] putArrayInt8(const char *array, int count);
// [12] putArrayInt8(const signed char *array, int count);
// [12] putArrayUint8(const char *array, int count);
// [12] putArrayUint8(const unsigned char *array, int count);
// [21] putArrayFloat64(const double *array,int count);
// [20] putArrayFloat32(const float *array, int count);
// [23] operator const void *() const;
//-----------------------------------------------------------------------------
// [-1] PERFORMANCE
// [ 1] BREATHING TEST
// [25] USAGE
//=============================================================================

//=============================================================================
//                    STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
    if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__);}}

#define LOOP2_ASSERT(I,J,X) { \
    if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
        << J << "\n"; aSsErT(1, #X, __LINE__); } }

//=============================================================================
//                    STANDARD BDE OUTPUT TEST MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef bdex_ByteOutStreamFormatter Obj;

const int SIZEOF_INT64   = 8;
const int SIZEOF_INT56   = 7;
const int SIZEOF_INT48   = 6;
const int SIZEOF_INT40   = 5;
const int SIZEOF_INT32   = 4;
const int SIZEOF_INT24   = 3;
const int SIZEOF_INT16   = 2;
const int SIZEOF_INT8    = 1;
const int SIZEOF_FLOAT64 = 8;
const int SIZEOF_FLOAT32 = 4;

//=============================================================================
//                      SUPPLEMENTARY TEST FUNCTIONALITY
//-----------------------------------------------------------------------------

static int eq(const char *lhs, const char *rhs, int numBits)
    // Return 1 if the specified leading 'numBits' are the same
    // for the specified 'lhs' and 'rhs' byte arrays, and 0 otherwise.
    // Note that it is the most significant bits in a partial byte that
    // are compared.
{
    ASSERT(0 <= numBits);
    int wholeBytes = numBits / 8;
    int extraBits = numBits % 8;

    for (int i = 0; i < wholeBytes; ++i) {
        if (lhs[i] ^ rhs[i]) {
            return 0;   // different
        }
    }

    if (extraBits) {
        int diff = lhs[wholeBytes] ^ rhs[wholeBytes];
        int remaingBits = 8 - extraBits;
        int mask = 0xff >> remaingBits << remaingBits;
        diff &= mask;   // if 0 != diff they're not equal
        return 0 == diff;
    }

    return 1;   // same
}

class my_FixedMemOutStreamBuf : public bdesb_FixedMemOutStreamBuf
{
    int d_flushCount;  // number of times 'sync' is called

  protected:
    // PROTECTED MANIPULATORS
    int sync();

  public:
    // CREATORS
    my_FixedMemOutStreamBuf(char            *buffer,
                            bsl::streamsize  length);
    ~my_FixedMemOutStreamBuf();

    // ACCESSORS
    int flushCount() const;
        // Return the number of times 'sync' was called since the creation of
        // this stream buffer.
};

// PROTECTED MANIPULATORS
int my_FixedMemOutStreamBuf::sync()
{
    ++d_flushCount;
    return bdesb_FixedMemOutStreamBuf::sync();
}

// CREATORS
my_FixedMemOutStreamBuf::my_FixedMemOutStreamBuf(char            *buffer,
                                                 bsl::streamsize  length)
: bdesb_FixedMemOutStreamBuf(buffer, length)
, d_flushCount(0)
{
}

my_FixedMemOutStreamBuf::~my_FixedMemOutStreamBuf()
{
}

// ACCESSORS
int my_FixedMemOutStreamBuf::flushCount() const
{
    return d_flushCount;
}

//=============================================================================
//                                  MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    // int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    bslma::TestAllocator ta(veryVeryVerbose);

    switch (test) { case 0:
      case 25: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, and replace 'assert' with 'ASSERT'.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

// The 'bdex_ByteOutStreamFormatter' implementation of the 'bdex_OutStream'
// protocol can be used to externalize values in a platform-neutral way.
// Writing out fundamental C++ types and 'bsl::string' requires no additional
// work on the part of the client; the client can simply use the stream
// directly.  The following code serializes a few representative values using a
// 'bdex_ByteOutStreamFormatter', and then writes the contents of the 'bdex'
// stream's buffer to 'stdout'.
//..
// int main(int argc, char **argv)
// {
//
    // Specify the buffer to write to
    char buffer[4096];
    bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);

    // Create a stream and write out some values.
    bdex_ByteOutStreamFormatter outStream(&sb);
    bdex_OutStreamFunctions::streamOut(outStream, 1, 0);
    bdex_OutStreamFunctions::streamOut(outStream, 2, 0);
    bdex_OutStreamFunctions::streamOut(outStream, 'c', 0);
    bdex_OutStreamFunctions::streamOut(outStream, bsl::string("hello"), 0);

    // Verify the results on 'stdout'.
    const int length = 15;
    for(int i = 0; i < length; ++i) {
        if (isalnum(buffer[i])) {
            cout << "nextByte (char): " << buffer[i] << endl;
        }
        else {
            cout << "nextByte (int): " << (int)buffer[i] << endl;
        }
    }
//     return 0;
// }
      } break;
      case 24: {
        // --------------------------------------------------------------------
        // FLUSH MANIPULATORS
        //
        // Testing:
        //   operator<<(bdex_BdexByteOutStreamFormatter&, bdexFlush);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "FLUSH" << endl
                          << "=====" << endl;

        char workSpace[100];
        my_FixedMemOutStreamBuf sb(workSpace, 100);
        Obj x(&sb);

        ASSERT(0 == sb.flushCount());
        x << bdexFlush;
        ASSERT(1 == sb.flushCount());

      } break;
      case 23: {
        // --------------------------------------------------------------------
        // STREAM VALIDITY METHODS
        //
        // Testing:
        //   void invalidate();
        //   operator const void *() const;
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "INVALIDATE" << endl
                          << "==========" << endl;

        if (verbose) cout << "\nTesting invalidate." << endl;
        {
            {
                char buffer[4096];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.invalidate();                  ASSERT(!X);
                mX.invalidate();                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putLength." << endl;
        {
            const int ONE_BYTE_LENGTH  = 127;
            const int FOUR_BYTE_LENGTH = 128;
            {
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putLength(ONE_BYTE_LENGTH);    ASSERT( X);
                mX.putLength(ONE_BYTE_LENGTH);    ASSERT(!X);
            }
            {
                char buffer[sizeof(int)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putLength(FOUR_BYTE_LENGTH);   ASSERT( X);
            }
            {
                char buffer[sizeof(int) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putLength(FOUR_BYTE_LENGTH);   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putString." << endl;
        {
            const bsl::string s("hello");
            {
                char buffer[4096];  // clearly big enough
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putString(s);                  ASSERT( X);
            }
            {
                char buffer[4];     // clearly too small
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putString(s);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putVersion." << endl;
        {
            {
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putVersion(0);                 ASSERT( X);
                mX.putVersion(0);                 ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt64." << endl;
        {
            typedef bsls::Types::Int64 T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt64(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt64(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint64." << endl;
        {
            typedef bsls::Types::Uint64 T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint64(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint64(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt56." << endl;
        {
            typedef bsls::Types::Int64 T;
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt56(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt56(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint56." << endl;
        {
            typedef bsls::Types::Uint64 T;
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint56(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint56(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt48." << endl;
        {
            typedef bsls::Types::Int64 T;
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt48(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt48(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint48." << endl;
        {
            typedef bsls::Types::Uint64 T;
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint48(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint48(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt40." << endl;
        {
            typedef bsls::Types::Int64 T;
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt40(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 4];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt40(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint40." << endl;
        {
            typedef bsls::Types::Uint64 T;
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint40(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 4];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint40(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt32." << endl;
        {
            typedef int T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt32(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt32(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint32." << endl;
        {
            typedef unsigned int T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint32(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint32(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt24." << endl;
        {
            typedef int T;
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt24(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt24(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint24." << endl;
        {
            typedef unsigned int T;
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint24(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint24(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt16." << endl;
        {
            typedef int T;
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt16(0);                   ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt16(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint16." << endl;
        {
            typedef unsigned int T;
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint16(0);                  ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint16(0);                  ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putInt8." << endl;
        {
            {
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putInt8(0);                    ASSERT( X);
                mX.putInt8(0);                    ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putUint8." << endl;
        {
            {
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putUint8(0);                   ASSERT( X);
                mX.putUint8(0);                   ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putFloat64." << endl;
        {
            typedef double T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putFloat64(0.0);               ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putFloat64(0.0);               ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putFloat32." << endl;
        {
            typedef float T;
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putFloat32(0.0f);              ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putFloat32(0.0f);              ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt64." << endl;
        {
            typedef bsls::Types::Int64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt64(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt64(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint64." << endl;
        {
            typedef bsls::Types::Uint64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint64(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint64(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt56." << endl;
        {
            typedef bsls::Types::Int64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt56(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt56(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint56." << endl;
        {
            typedef bsls::Types::Uint64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint56(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint56(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt48." << endl;
        {
            typedef bsls::Types::Int64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt48(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt48(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint48." << endl;
        {
            typedef bsls::Types::Uint64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint48(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint48(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt40." << endl;
        {
            typedef bsls::Types::Int64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt40(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 4];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt40(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint40." << endl;
        {
            typedef bsls::Types::Uint64 T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 3];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint40(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 4];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint40(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt32." << endl;
        {
            typedef int T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt32(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt32(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint32." << endl;
        {
            typedef unsigned int T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint32(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint32(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt24." << endl;
        {
            typedef int T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt24(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt24(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint24." << endl;
        {
            typedef unsigned int T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint24(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 2];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint24(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt16." << endl;
        {
            typedef short T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt16(mA, 1);          ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt16(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint16." << endl;
        {
            typedef unsigned short T;
            T mA[1] = { 0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint16(mA, 1);         ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint16(mA, 1);         ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayInt8." << endl;
        {
            {
                typedef char T;
                T mA[1] = { 0 };
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt8(mA, 1);           ASSERT( X);
                mX.putArrayInt8(mA, 1);           ASSERT(!X);
            }
            {
                typedef signed char T;
                T mA[1] = { 0 };
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayInt8(mA, 1);           ASSERT( X);
                mX.putArrayInt8(mA, 1);           ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayUint8." << endl;
        {
            {
                typedef char T;
                T mA[1] = { 0 };
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint8(mA, 1);          ASSERT( X);
                mX.putArrayUint8(mA, 1);          ASSERT(!X);
            }
            {
                typedef unsigned char T;
                T mA[1] = { 0 };
                char buffer[1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayUint8(mA, 1);          ASSERT( X);
                mX.putArrayUint8(mA, 1);          ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayFloat64." << endl;
        {
            typedef double T;
            T mA[1] = { 0.0 };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayFloat64(mA, 1);        ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayFloat64(mA, 1);        ASSERT(!X);
            }
        }
        if (verbose) cout << "\nTesting putArrayFloat32." << endl;
        {
            typedef float T;
            T mA[1] = { 0.0f };
            {
                char buffer[sizeof(T)];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayFloat32(mA, 1);        ASSERT( X);
            }
            {
                char buffer[sizeof(T) - 1];
                bdesb_FixedMemOutStreamBuf sb(buffer, sizeof buffer);
                Obj mX(&sb);  const Obj& X = mX;  ASSERT( X);
                mX.putArrayFloat32(mA, 1);        ASSERT(!X);
            }
        }
      } break;
      case 22: {
        // --------------------------------------------------------------------
        // PUT LENGTH AND VERSION TEST:
        //
        // Testing:
        //   putLength(int value);
        //   putVersion(int value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT LENGTH AND VERSION TEST" << endl
                          << "===========================" << endl;
        {
            if (verbose) cout << "\nTesting putLength." << endl;
            {
                char workSpace[100];
                bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
                Obj x(&sb);
                                    x.putInt8(0xff);
                x.putLength(  1);     x.putInt8(0xfe);
                x.putLength(128);     x.putInt8(0xfd);
                x.putLength(127);     x.putInt8(0xfc);
                x.putLength(256);     x.putInt8(0xfb);
                const int NUM_BYTES = 7 * SIZEOF_INT8 + 2 * SIZEOF_INT32;
                const int NUM_BITS = 8 * NUM_BYTES;
                ASSERT(1 == eq(workSpace,
                               "\xff" "\x01" "\xfe"
                               "\x80\x00\x00\x80" "\xfd"
                               "\x7f" "\xfc"
                               "\x80\x00\x01\x00" "\xfb", NUM_BITS));
            }
        }

        {
            const int SIZE = SIZEOF_INT8;

            if (verbose) cout << "\nTesting putVersion." << endl;
            {
                char workSpace[100];
                bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
                Obj x(&sb);
                x.putVersion(1);
                x.putVersion(2);
                x.putVersion(3);
                x.putVersion(4);
                const int NUM_BYTES = 4 * SIZE;
                const int NUM_BITS = 8 * NUM_BYTES;
                ASSERT(1 == eq(workSpace,
                               "\x01\x02\x03\x04",
                               NUM_BITS));
            }
            {
                char workSpace[100];
                bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
                Obj x(&sb);
                x.putVersion(252);
                x.putVersion(253);
                x.putVersion(254);
                x.putVersion(255);
                const int NUM_BYTES = 4 * SIZE;
                const int NUM_BITS = 8 * NUM_BYTES;
                ASSERT(1 == eq(workSpace,
                               "\xfc\xfd\xfe\xff",
                               NUM_BITS));
            }
        }
      } break;
      case 21: {
        // --------------------------------------------------------------------
        // PUT 64-BIT FLOAT ARRAYS TEST:
        //
        // Testing:
        //   putArrayFloat64(const double *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 64-BIT FLOAT ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_FLOAT64;

        if (verbose) cout << "\nTesting putArrayFloat64." << endl;
        {
            const double DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayFloat64(DATA, 0);     x.putInt8(0xff);
            x.putArrayFloat64(DATA, 1);     x.putInt8(0xfe);
            x.putArrayFloat64(DATA, 2);     x.putInt8(0xfd);
            x.putArrayFloat64(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xff"
                   "\x3f\xf0\x00\x00\x00\x00\x00\x00" "\xfe"
                   "\x3f\xf0\x00\x00\x00\x00\x00\x00"
                   "\x40\x00\x00\x00\x00\x00\x00\x00" "\xfd"
                   "\x3f\xf0\x00\x00\x00\x00\x00\x00"
                   "\x40\x00\x00\x00\x00\x00\x00\x00"
                   "\x40\x08\x00\x00\x00\x00\x00\x00" "\xfc", NUM_BITS));
        }
        {
            const double DATA[] = {1.5, 2.5, 3.5};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayFloat64(DATA, 0);     x.putInt8(0xfc);
            x.putArrayFloat64(DATA, 1);     x.putInt8(0xfd);
            x.putArrayFloat64(DATA, 2);     x.putInt8(0xfe);
            x.putArrayFloat64(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xfc"
                   "\x3f\xf8\x00\x00\x00\x00\x00\x00" "\xfd"
                   "\x3f\xf8\x00\x00\x00\x00\x00\x00"
                   "\x40\x04\x00\x00\x00\x00\x00\x00" "\xfe"
                   "\x3f\xf8\x00\x00\x00\x00\x00\x00"
                   "\x40\x04\x00\x00\x00\x00\x00\x00"
                   "\x40\x0c\x00\x00\x00\x00\x00\x00" "\xff", NUM_BITS));
        }
      } break;
      case 20: {
        // --------------------------------------------------------------------
        // PUT 32-BIT FLOAT ARRAYS TEST:
        //
        // Testing:
        //   putArrayFloat32(const float *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 32-BIT FLOAT ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_FLOAT32;

        if (verbose) cout << "\nTesting putArrayFloat32." << endl;
        {
            const float DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayFloat32(DATA, 0);     x.putInt8(0xff);
            x.putArrayFloat32(DATA, 1);     x.putInt8(0xfe);
            x.putArrayFloat32(DATA, 2);     x.putInt8(0xfd);
            x.putArrayFloat32(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xff"
                   "\x3f\x80\x00\x00" "\xfe"
                   "\x3f\x80\x00\x00"
                   "\x40\x00\x00\x00" "\xfd"
                   "\x3f\x80\x00\x00"
                   "\x40\x00\x00\x00"
                   "\x40\x40\x00\x00" "\xfc", NUM_BITS));
        }
        {
            const float DATA[] = {1.5, 2.5, 3.5};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayFloat32(DATA, 0);     x.putInt8(0xfc);
            x.putArrayFloat32(DATA, 1);     x.putInt8(0xfd);
            x.putArrayFloat32(DATA, 2);     x.putInt8(0xfe);
            x.putArrayFloat32(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xfc"
                   "\x3f\xc0\x00\x00" "\xfd"
                   "\x3f\xc0\x00\x00"
                   "\x40\x20\x00\x00" "\xfe"
                   "\x3f\xc0\x00\x00"
                   "\x40\x20\x00\x00"
                   "\x40\x60\x00\x00" "\xff", NUM_BITS));
        }
      } break;
      case 19: {
        // --------------------------------------------------------------------
        // PUT 64-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt64(const int *array, int count);
        //   putArrayUint64(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 64-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT64;

        if (verbose) cout << "\nTesting putArrayInt64." << endl;
        {
            const bsls::Types::Int64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt64(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt64(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt64(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt64(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xff"
                   "\x00\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Int64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt64(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt64(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt64(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt64(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xfc"
                   "\x00\x00\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint64." << endl;
        {
            const bsls::Types::Uint64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint64(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint64(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint64(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint64(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xff"
                   "\x00\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Uint64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint64(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint64(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint64(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint64(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                                 "\xfc"
                   "\x00\x00\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 18: {
        // --------------------------------------------------------------------
        // PUT 56-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt56(const int *array, int count);
        //   putArrayUint56(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 56-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT56;

        if (verbose) cout << "\nTesting putArrayInt56." << endl;
        {
            const bsls::Types::Int64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt56(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt56(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt56(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt56(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                             "\xff"
                   "\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Int64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt56(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt56(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt56(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt56(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                             "\xfc"
                   "\x00\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint56." << endl;
        {
            const bsls::Types::Uint64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint56(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint56(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint56(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint56(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                             "\xff"
                   "\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Uint64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint56(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint56(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint56(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint56(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                             "\xfc"
                   "\x00\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // PUT 48-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt48(const int *array, int count);
        //   putArrayUint48(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 48-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT48;

        if (verbose) cout << "\nTesting putArrayInt48." << endl;
        {
            const bsls::Types::Int64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt48(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt48(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt48(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt48(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                         "\xff"
                   "\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Int64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt48(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt48(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt48(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt48(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                         "\xfc"
                   "\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint48." << endl;
        {
            const bsls::Types::Uint64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint48(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint48(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint48(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint48(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                         "\xff"
                   "\x00\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Uint64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint48(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint48(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint48(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint48(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                         "\xfc"
                   "\x00\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 16: {
        // --------------------------------------------------------------------
        // PUT 40-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt40(const int *array, int count);
        //   putArrayUint40(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 40-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT40;

        if (verbose) cout << "\nTesting putArrayInt40." << endl;
        {
            const bsls::Types::Int64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt40(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt40(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt40(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt40(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                     "\xff"
                   "\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Int64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt40(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt40(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt40(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt40(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                     "\xfc"
                   "\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint40." << endl;
        {
            const bsls::Types::Uint64 DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint40(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint40(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint40(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint40(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                     "\xff"
                   "\x00\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x00\x01"
                   "\x00\x00\x00\x00\x02"
                   "\x00\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const bsls::Types::Uint64 DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint40(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint40(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint40(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint40(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                     "\xfc"
                   "\x00\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x00\x04"
                   "\x00\x00\x00\x00\x05"
                   "\x00\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 15: {
        // --------------------------------------------------------------------
        // PUT 32-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt32(const int *array, int count);
        //   putArrayUint32(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 32-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT32;

        if (verbose) cout << "\nTesting putArrayInt32." << endl;
        {
            const int DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt32(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt32(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt32(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt32(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xff"
                   "\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x01"
                   "\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x01"
                   "\x00\x00\x00\x02"
                   "\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const int DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt32(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt32(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt32(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt32(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xfc"
                   "\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x04"
                   "\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x04"
                   "\x00\x00\x00\x05"
                   "\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint32." << endl;
        {
            const unsigned int DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint32(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint32(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint32(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint32(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xff"
                   "\x00\x00\x00\x01" "\xfe"
                   "\x00\x00\x00\x01"
                   "\x00\x00\x00\x02" "\xfd"
                   "\x00\x00\x00\x01"
                   "\x00\x00\x00\x02"
                   "\x00\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const unsigned int DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint32(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint32(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint32(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint32(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                 "\xfc"
                   "\x00\x00\x00\x04" "\xfd"
                   "\x00\x00\x00\x04"
                   "\x00\x00\x00\x05" "\xfe"
                   "\x00\x00\x00\x04"
                   "\x00\x00\x00\x05"
                   "\x00\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 14: {
        // --------------------------------------------------------------------
        // PUT 24-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt24(const int *array, int count);
        //   putArrayUint24(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 24-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT24;

        if (verbose) cout << "\nTesting putArrayInt24." << endl;
        {
            const int DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt24(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt24(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt24(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt24(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""             "\xff"
                   "\x00\x00\x01" "\xfe"
                   "\x00\x00\x01"
                   "\x00\x00\x02" "\xfd"
                   "\x00\x00\x01"
                   "\x00\x00\x02"
                   "\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const int DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt24(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt24(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt24(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt24(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""             "\xfc"
                   "\x00\x00\x04" "\xfd"
                   "\x00\x00\x04"
                   "\x00\x00\x05" "\xfe"
                   "\x00\x00\x04"
                   "\x00\x00\x05"
                   "\x00\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint24." << endl;
        {
            const unsigned int DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint24(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint24(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint24(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint24(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""             "\xff"
                   "\x00\x00\x01" "\xfe"
                   "\x00\x00\x01"
                   "\x00\x00\x02" "\xfd"
                   "\x00\x00\x01"
                   "\x00\x00\x02"
                   "\x00\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const unsigned int DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint24(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint24(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint24(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint24(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""             "\xfc"
                   "\x00\x00\x04" "\xfd"
                   "\x00\x00\x04"
                   "\x00\x00\x05" "\xfe"
                   "\x00\x00\x04"
                   "\x00\x00\x05"
                   "\x00\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // PUT 16-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt16(const int *array, int count);
        //   putArrayUint16(const unsigned int *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 16-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT16;

        if (verbose) cout << "\nTesting putArrayInt16." << endl;
        {
            const short DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt16(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt16(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt16(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt16(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                               "\xff"
                   "\x00\x01"                       "\xfe"
                   "\x00\x01" "\x00\x02"            "\xfd"
                   "\x00\x01" "\x00\x02" "\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const short DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt16(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt16(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt16(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt16(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                               "\xfc"
                   "\x00\x04"                       "\xfd"
                   "\x00\x04" "\x00\x05"            "\xfe"
                   "\x00\x04" "\x00\x05" "\x00\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint16." << endl;
        {
            const unsigned short DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint16(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint16(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint16(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint16(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                               "\xff"
                   "\x00\x01"                       "\xfe"
                   "\x00\x01" "\x00\x02"            "\xfd"
                   "\x00\x01" "\x00\x02" "\x00\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const unsigned short DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint16(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint16(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint16(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint16(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                               "\xfc"
                   "\x00\x04"                       "\xfd"
                   "\x00\x04" "\x00\x05"            "\xfe"
                   "\x00\x04" "\x00\x05" "\x00\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // PUT 8-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   putArrayInt8(const char *array, int count);
        //   putArrayInt8(const signed char *array, int count);
        //   putArrayUint8(const char *array, int count);
        //   putArrayUint8(const unsigned char *array, int count);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 8-BIT INTEGER ARRAYS TEST" << endl
                          << "==============================" << endl;

        const int SIZE = SIZEOF_INT8;

        if (verbose) cout << "\nTesting putArrayInt8(char *)." << endl;
        {
            const char DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt8(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt8(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt8(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt8(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xff"
                   "\x01"               "\xfe"
                   "\x01" "\x02"        "\xfd"
                   "\x01" "\x02" "\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const char DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt8(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt8(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt8(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt8(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                  "\xfc"
                   "\x04"              "\xfd"
                   "\x04" "\x05"       "\xfe"
                   "\x04" "\x05""\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayInt8(signed char *)." << endl;
        {
            const signed char DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt8(DATA, 0);     x.putInt8(0xff);
            x.putArrayInt8(DATA, 1);     x.putInt8(0xfe);
            x.putArrayInt8(DATA, 2);     x.putInt8(0xfd);
            x.putArrayInt8(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xff"
                   "\x01"               "\xfe"
                   "\x01" "\x02"        "\xfd"
                   "\x01" "\x02" "\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const signed char DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayInt8(DATA, 0);     x.putInt8(0xfc);
            x.putArrayInt8(DATA, 1);     x.putInt8(0xfd);
            x.putArrayInt8(DATA, 2);     x.putInt8(0xfe);
            x.putArrayInt8(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xfc"
                   "\x04"               "\xfd"
                   "\x04" "\x05"        "\xfe"
                   "\x04" "\x05" "\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint8(char *)." << endl;
        {
            const char DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint8(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint8(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint8(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint8(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xff"
                   "\x01"               "\xfe"
                   "\x01" "\x02"        "\xfd"
                   "\x01" "\x02" "\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const char DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint8(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint8(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint8(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint8(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xfc"
                   "\x04"               "\xfd"
                   "\x04" "\x05"        "\xfe"
                   "\x04" "\x05" "\x06" "\xff",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putArrayUint8(unsigned char *)."
                          << endl;
        {
            const unsigned char DATA[] = {1, 2, 3};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint8(DATA, 0);     x.putInt8(0xff);
            x.putArrayUint8(DATA, 1);     x.putInt8(0xfe);
            x.putArrayUint8(DATA, 2);     x.putInt8(0xfd);
            x.putArrayUint8(DATA, 3);     x.putInt8(0xfc);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xff"
                   "\x01"               "\xfe"
                   "\x01" "\x02"        "\xfd"
                   "\x01" "\x02" "\x03" "\xfc",
                           NUM_BITS));
        }
        {
            const unsigned char DATA[] = {4, 5, 6};
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putArrayUint8(DATA, 0);     x.putInt8(0xfc);
            x.putArrayUint8(DATA, 1);     x.putInt8(0xfd);
            x.putArrayUint8(DATA, 2);     x.putInt8(0xfe);
            x.putArrayUint8(DATA, 3);     x.putInt8(0xff);
            const int NUM_BYTES = 4 * SIZEOF_INT8 + 6 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                   ""                   "\xfc"
                   "\x04"               "\xfd"
                   "\x04" "\x05"        "\xfe"
                   "\x04" "\x05" "\x06" "\xff",
                           NUM_BITS));
        }
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // PUT 64-BIT FLOATS TEST:
        //
        // Testing:
        //   putFloat64(float value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 64-BIT FLOATS TEST" << endl
                          << "=======================" << endl;

        const int SIZE = SIZEOF_FLOAT64;

        if (verbose) cout << "\nTesting putFloat64." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                 x.putInt8(0xff);
            x.putFloat64(1);     x.putInt8(0xfe);
            x.putFloat64(2);     x.putInt8(0xfd);
            x.putFloat64(3);     x.putInt8(0xfc);
            x.putFloat64(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x3f\xf0\x00\x00\x00\x00\x00\x00" "\xfe"
                       "\x40\x00\x00\x00\x00\x00\x00\x00" "\xfd"
                       "\x40\x08\x00\x00\x00\x00\x00\x00" "\xfc"
                       "\x40\x10\x00\x00\x00\x00\x00\x00" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                   x.putInt8(0xfb);
            x.putFloat64(1.5);     x.putInt8(0xfc);
            x.putFloat64(2.5);     x.putInt8(0xfd);
            x.putFloat64(3.5);     x.putInt8(0xfe);
            x.putFloat64(5.0);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x3f\xf8\x00\x00\x00\x00\x00\x00" "\xfc"
                       "\x40\x04\x00\x00\x00\x00\x00\x00" "\xfd"
                       "\x40\x0c\x00\x00\x00\x00\x00\x00" "\xfe"
                       "\x40\x14\x00\x00\x00\x00\x00\x00" "\xff", NUM_BITS));
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // PUT 32-BIT FLOATS TEST:
        //
        // Testing:
        //   putFloat32(float value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 32-BIT FLOATS TEST" << endl
                          << "=======================" << endl;

        const int SIZE = SIZEOF_FLOAT32;

        if (verbose) cout << "\nTesting putFloat32." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                 x.putInt8(0xff);
            x.putFloat32(1);     x.putInt8(0xfe);
            x.putFloat32(2);     x.putInt8(0xfd);
            x.putFloat32(3);     x.putInt8(0xfc);
            x.putFloat32(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x3f\x80\x00\x00" "\xfe"
                       "\x40\x00\x00\x00" "\xfd"
                       "\x40\x40\x00\x00" "\xfc"
                       "\x40\x80\x00\x00" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                   x.putInt8(0xfb);
            x.putFloat32(1.5);     x.putInt8(0xfc);
            x.putFloat32(2.5);     x.putInt8(0xfd);
            x.putFloat32(3.5);     x.putInt8(0xfe);
            x.putFloat32(5.0);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x3f\xc0\x00\x00" "\xfc"
                       "\x40\x20\x00\x00" "\xfd"
                       "\x40\x60\x00\x00" "\xfe"
                       "\x40\xa0\x00\x00" "\xff", NUM_BITS));
        }
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // PUT 64-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt64(bsls::Types::Int64 value);
        //   putUint64(bsls::Types::Uint64 value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 64-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT64;

        if (verbose) cout << "\nTesting putInt64." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt64(1);     x.putInt8(0xfe);
            x.putInt64(2);     x.putInt8(0xfd);
            x.putInt64(3);     x.putInt8(0xfc);
            x.putInt64(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt64(5);     x.putInt8(0xfc);
            x.putInt64(6);     x.putInt8(0xfd);
            x.putInt64(7);     x.putInt8(0xfe);
            x.putInt64(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint64." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint64(1);     x.putInt8(0xfe);
            x.putUint64(2);     x.putInt8(0xfd);
            x.putUint64(3);     x.putInt8(0xfc);
            x.putUint64(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint64(5);     x.putInt8(0xfc);
            x.putUint64(6);     x.putInt8(0xfd);
            x.putUint64(7);     x.putInt8(0xfe);
            x.putUint64(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // PUT 56-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt56(bsls::Types::Int64 value);
        //   putUint56(bsls::Types::Uint64 value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 56-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT56;

        if (verbose) cout << "\nTesting putInt56." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt56(1);     x.putInt8(0xfe);
            x.putInt56(2);     x.putInt8(0xfd);
            x.putInt56(3);     x.putInt8(0xfc);
            x.putInt56(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt56(5);     x.putInt8(0xfc);
            x.putInt56(6);     x.putInt8(0xfd);
            x.putInt56(7);     x.putInt8(0xfe);
            x.putInt56(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint56." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint56(1);     x.putInt8(0xfe);
            x.putUint56(2);     x.putInt8(0xfd);
            x.putUint56(3);     x.putInt8(0xfc);
            x.putUint56(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint56(5);     x.putInt8(0xfc);
            x.putUint56(6);     x.putInt8(0xfd);
            x.putUint56(7);     x.putInt8(0xfe);
            x.putUint56(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // PUT 48-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt48(bsls::Types::Int64 value);
        //   putUint48(bsls::Types::Uint64 value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 48-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT48;

        if (verbose) cout << "\nTesting putInt48." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt48(1);     x.putInt8(0xfe);
            x.putInt48(2);     x.putInt8(0xfd);
            x.putInt48(3);     x.putInt8(0xfc);
            x.putInt48(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt48(5);     x.putInt8(0xfc);
            x.putInt48(6);     x.putInt8(0xfd);
            x.putInt48(7);     x.putInt8(0xfe);
            x.putInt48(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint48." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint48(1);     x.putInt8(0xfe);
            x.putUint48(2);     x.putInt8(0xfd);
            x.putUint48(3);     x.putInt8(0xfc);
            x.putUint48(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint48(5);     x.putInt8(0xfc);
            x.putUint48(6);     x.putInt8(0xfd);
            x.putUint48(7);     x.putInt8(0xfe);
            x.putUint48(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // PUT 40-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt40(bsls::Types::Int64 value);
        //   putUint40(bsls::Types::Uint64 value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 40-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT40;

        if (verbose) cout << "\nTesting putInt40." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt40(1);     x.putInt8(0xfe);
            x.putInt40(2);     x.putInt8(0xfd);
            x.putInt40(3);     x.putInt8(0xfc);
            x.putInt40(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt40(5);     x.putInt8(0xfc);
            x.putInt40(6);     x.putInt8(0xfd);
            x.putInt40(7);     x.putInt8(0xfe);
            x.putInt40(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint40." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint40(1);     x.putInt8(0xfe);
            x.putUint40(2);     x.putInt8(0xfd);
            x.putUint40(3);     x.putInt8(0xfc);
            x.putUint40(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint40(5);     x.putInt8(0xfc);
            x.putUint40(6);     x.putInt8(0xfd);
            x.putUint40(7);     x.putInt8(0xfe);
            x.putUint40(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // PUT 32-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt32(int value);
        //   putUint32(unsigned int value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 32-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT32;

        if (verbose) cout << "\nTesting putInt32." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt32(1);     x.putInt8(0xfe);
            x.putInt32(2);     x.putInt8(0xfd);
            x.putInt32(3);     x.putInt8(0xfc);
            x.putInt32(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt32(5);     x.putInt8(0xfc);
            x.putInt32(6);     x.putInt8(0xfd);
            x.putInt32(7);     x.putInt8(0xfe);
            x.putInt32(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint32." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint32(1);     x.putInt8(0xfe);
            x.putUint32(2);     x.putInt8(0xfd);
            x.putUint32(3);     x.putInt8(0xfc);
            x.putUint32(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x00\x01" "\xfe"
                       "\x00\x00\x00\x02" "\xfd"
                       "\x00\x00\x00\x03" "\xfc"
                       "\x00\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint32(5);     x.putInt8(0xfc);
            x.putUint32(6);     x.putInt8(0xfd);
            x.putUint32(7);     x.putInt8(0xfe);
            x.putUint32(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x00\x05" "\xfc"
                       "\x00\x00\x00\x06" "\xfd"
                       "\x00\x00\x00\x07" "\xfe"
                       "\x00\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // PUT 24-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt24(int value);
        //   putUint24(unsigned int value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 24-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT24;

        if (verbose) cout << "\nTesting putInt24." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt24(1);     x.putInt8(0xfe);
            x.putInt24(2);     x.putInt8(0xfd);
            x.putInt24(3);     x.putInt8(0xfc);
            x.putInt24(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x01" "\xfe"
                       "\x00\x00\x02" "\xfd"
                       "\x00\x00\x03" "\xfc"
                       "\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt24(5);     x.putInt8(0xfc);
            x.putInt24(6);     x.putInt8(0xfd);
            x.putInt24(7);     x.putInt8(0xfe);
            x.putInt24(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x05" "\xfc"
                       "\x00\x00\x06" "\xfd"
                       "\x00\x00\x07" "\xfe"
                       "\x00\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint24." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint24(1);     x.putInt8(0xfe);
            x.putUint24(2);     x.putInt8(0xfd);
            x.putUint24(3);     x.putInt8(0xfc);
            x.putUint24(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x00\x01" "\xfe"
                       "\x00\x00\x02" "\xfd"
                       "\x00\x00\x03" "\xfc"
                       "\x00\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint24(5);     x.putInt8(0xfc);
            x.putUint24(6);     x.putInt8(0xfd);
            x.putUint24(7);     x.putInt8(0xfe);
            x.putUint24(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x00\x05" "\xfc"
                       "\x00\x00\x06" "\xfd"
                       "\x00\x00\x07" "\xfe"
                       "\x00\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // PUT 16-BIT INTEGERS TEST:
        //
        // Testing:
        //   putInt16(short value);
        //   putUint16(unsigned short value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PUT 16-BIT INTEGERS TEST" << endl
                          << "========================" << endl;

        const int SIZE = SIZEOF_INT16;

        if (verbose) cout << "\nTesting putInt16." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xff);
            x.putInt16(1);     x.putInt8(0xfe);
            x.putInt16(2);     x.putInt8(0xfd);
            x.putInt16(3);     x.putInt8(0xfc);
            x.putInt16(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x01" "\xfe"
                       "\x00\x02" "\xfd"
                       "\x00\x03" "\xfc"
                       "\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                               x.putInt8(0xfb);
            x.putInt16(5);     x.putInt8(0xfc);
            x.putInt16(6);     x.putInt8(0xfd);
            x.putInt16(7);     x.putInt8(0xfe);
            x.putInt16(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x05" "\xfc"
                       "\x00\x06" "\xfd"
                       "\x00\x07" "\xfe"
                       "\x00\x08" "\xff", NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint16." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xff);
            x.putUint16(1);     x.putInt8(0xfe);
            x.putUint16(2);     x.putInt8(0xfd);
            x.putUint16(3);     x.putInt8(0xfc);
            x.putUint16(4);     x.putInt8(0xfb);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xff" "\x00\x01" "\xfe"
                       "\x00\x02" "\xfd"
                       "\x00\x03" "\xfc"
                       "\x00\x04" "\xfb", NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
                                x.putInt8(0xfb);
            x.putUint16(5);     x.putInt8(0xfc);
            x.putUint16(6);     x.putInt8(0xfd);
            x.putUint16(7);     x.putInt8(0xfe);
            x.putUint16(8);     x.putInt8(0xff);
            const int NUM_BYTES = 5 * SIZEOF_INT8 + 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                "\xfb" "\x00\x05" "\xfc"
                       "\x00\x06" "\xfd"
                       "\x00\x07" "\xfe"
                       "\x00\x08" "\xff", NUM_BITS));
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // PRIMARY MANIPULATORS TEST:
        //    that the destructor is exercised on each configuration as the
        //    object being tested leaves scope.  We also make sure that initial
        //    size was set and no more allocations were necessary during the
        //    test.
        //
        //    We have chosen to test function 'putUint8' here for compactness
        //    and relevancy, though it is not needed until later.
        //
        // Testing:
        //   bdex_ByteOutStreamFormatter(bsl::streambuf *);
        //   ~bdex_ByteOutStreamFormatter();  // via purify
        //   putInt8(int value);
        //   putUint8(int value);
        //   removeAll();
        //   reserveCapacity(int newCapacity);
        // --------------------------------------------------------------------
        if (verbose) cout << endl
                          << "PRIMARY MANIPULATORS TEST" << endl
                          << "=========================" << endl;

        const int SIZE = SIZEOF_INT8;

        if (verbose) cout << "\nTesting putInt8, reserveCapacity, and "
                          << "constructor." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putInt8(1);
            x.putInt8(2);
            x.putInt8(3);
            x.putInt8(4);
            const int NUM_BYTES = 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                           "\x01\x02\x03\x04",
                           NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putInt8(5);
            x.putInt8(6);
            x.putInt8(7);
            x.putInt8(8);
            const int NUM_BYTES = 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                           "\x05\x06\x07\x08",
                           NUM_BITS));
        }

        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting putUint8." << endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putUint8(1);
            x.putUint8(2);
            x.putUint8(3);
            x.putUint8(4);
            const int NUM_BYTES = 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                           "\x01\x02\x03\x04",
                           NUM_BITS));
        }
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x(&sb);
            x.putUint8(5);
            x.putUint8(6);
            x.putUint8(7);
            x.putUint8(8);
            const int NUM_BYTES = 4 * SIZE;
            const int NUM_BITS = 8 * NUM_BYTES;
            ASSERT(1 == eq(workSpace,
                           "\x05\x06\x07\x08",
                           NUM_BITS));
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //   Create 'bdex_ByteOutStreamFormatter' objects using default and
        //   copy constructors.  Exercise these objects using some "put"
        //   methods, basic accessors, equality operators, and the assignment
        //   operator.  Display object values frequently in verbose mode.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BREATHING TEST" << endl
                          << "==============" << endl;

        if (verbose)
            cout << "\nCreate object x1 using default ctor."<< endl;
        {
            char workSpace[100];
            bdesb_FixedMemOutStreamBuf sb(workSpace, 100);
            Obj x1(&sb);

            if (verbose) cout << "\nTry putInt32 with x1." << endl;
            x1.putInt32(1);
            ASSERT(1 == eq("\x00\x00\x00\x01", workSpace, SIZEOF_INT32 * 8));


            if (verbose) cout << "\nTry putArrayInt8 with x1.\n";
            const char data[] = {0x01, 0x02, 0x03, 0x04};
            const int size = sizeof data / sizeof *data;
            sb.pubsetbuf(workSpace, 100);
            x1.putArrayInt8(data, size);
            ASSERT(1 == eq("\x01\x02\x03\x04", workSpace, size * 8));
        }

      } break;
      case -1: {
        // --------------------------------------------------------------------
        // PERFORMANCE
        // --------------------------------------------------------------------

        if (verbose)
            cout << "\nPERFORMANCE TEST FOR 'bdex_BdexByteOutStreamFormatter'"
                 << "\n======================================================"
                 << endl;

        const int NUM_ITERS = (argc > 2) ? atoi(argv[2]) : 1000;
        const int NUM_OUT_ITERS = (argc > 3) ? atoi(argv[3]) : 10000000;

        if (NUM_ITERS < 1 || NUM_OUT_ITERS < 1) {
            cout << "Usage: test -1 numInIters numOutIters\n"
                 << "\twhere numInIters = number of putInt8 in inner loop\n"
                 << "\tand numOutIters  = number of ctor+putInt8+dtor in"
                                                            " outer loop\n"
                 << "\t(default values to 1000 and 1000000, resp.)\n";
            return -1;
        }

        char *workSpace = new char[(NUM_ITERS+1) * sizeof(int)];
        const int SIZE = NUM_ITERS+1;

        if (verbose) cout << "\nWith bdesb_FixedMemOutput" << endl;

        if (verbose) cout << "\nTest performance on ctor+putInt8+dtor.\n";
        {
            for (int i = 0; i < SIZE; ++i) {
                workSpace[i] = (char)(i * (i + 1) / 2);
            }

            bsls::Stopwatch timer; timer.start();
            for (int i = 0; i <= NUM_OUT_ITERS; ++i) {
                bdesb_FixedMemOutStreamBuf sb(workSpace, SIZE);
                Obj x1(&sb);
                for (int j = 0; j < NUM_ITERS; ++j) {
                    x1.putInt8(1);
                }
            }
            double elapsed = timer.elapsedTime();
            ASSERT(1 == workSpace[0]);
            if (verbose) cout << "\tin " << elapsed << "seconds\n";
        }

        if (verbose) cout << "\nTest performance on ctor+putInt32+dtor.\n";
        {
            for (int i = 0; i < SIZE; ++i) {
                workSpace[i] = (char)(i * (i + 1) / 2);
            }

            bsls::Stopwatch timer; timer.start();
            for (int i = 0; i <= NUM_OUT_ITERS; ++i) {
                bdesb_FixedMemOutStreamBuf sb(workSpace, SIZE);
                Obj x1(&sb);
                for (int j = 0; j < NUM_ITERS; ++j) {
                    x1.putInt32(1);
                }
            }
            double elapsed = timer.elapsedTime();
            ASSERT(0 == workSpace[0]);
            ASSERT(0 == workSpace[1]);
            ASSERT(0 == workSpace[2]);
            ASSERT(1 == workSpace[3]);
            if (verbose) cout << "\tin " << elapsed << "seconds\n";
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
