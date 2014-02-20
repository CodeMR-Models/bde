// bdeut_nullablevalue.t.cpp    -*-C++-*-

#include <bdeut_nullablevalue.h>

#include <bslalg_constructorproxy.h>
#include <bslma_testallocator.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_testallocatormonitor.h>
#include <bslma_usesbslmaallocator.h>
#include <bsls_asserttest.h>

#include <bdex_testinstream.h>
#include <bdex_testoutstream.h>

#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_string.h>

#include <bsltf_templatetestfacility.h>
#include <bsltf_testvaluesarray.h>

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
// This component implements a wrapper for a value-semantic type, and itself
// exhibits value-semantic properties.
//
// Global Concerns:
//   o No memory is allocated from the global-allocator
//
//-----------------------------------------------------------------------------
// TYPEDEFS
// [ 3] typedef TYPE ValueType;
//
// CREATORS
// [ 3] bdeut_NullableValue();
// [ 3] bdeut_NullableValue(bslma::Allocator *basicAllocator);
// [ 6] bdeut_NullableValue(const bdeut_NullableValue& original);
// [ 6] bdeut_NullableValue(const bdeut_NullableValue& original, *ba);
// [ 9] bdeut_NullableValue(const TYPE& value);
// [ 9] bdeut_NullableValue(const TYPE& value, *ba);
// [11] bdeut_NullableValue(const bdeut_NullableValue<ORIGINAL>&o);
// [11] bdeut_NullableValue(const bdeut_NullableValue<ORIGINAL>&o,*ba);
// [ 3] ~bdeut_NullableValue();
//
// MANIPULATORS
// [ 7] operator=(const bdeut_NullableValue& rhs);
// [12] operator=(const bdeut_NullableValue<OTHER_TYPE>& rhs);
// [10] operator=(const TYPE& rhs);
// [12] operator=(const OTHER_TYPE& rhs);
// [13] void swap(bdeut_NullableValue<TYPE>& other);
// [ 3] TYPE& makeValue(const TYPE& value);
// [12] TYPE& makeValue(const OTHER_TYPE& value);
// [10] TYPE& makeValue();
// [ 8] STREAM& bdexStreamIn(STREAM& stream, int version);
// [10] void reset();
// [10] TYPE& value();
// [14] TYPE valueOr(const TYPE& ) const;
// [15] const TYPE *valueOr(const TYPE *) const;
// [16] const TYPE* valueOrNull() const;
//
// ACCESSORS
// [ 8] STREAM& bdexStreamOut(STREAM& stream, int version) const;
// [ 3] bool isNull() const;
// [ 8] int maxSupportedBdexVersion() const;
// [ 4] print(bsl::ostream& s,int l=0,int spl=4) const;
// [ 3] const TYPE& value() const;
//
// FREE OPERATORS
// [ 5] operator==(const bdeut_NullableValue<LHS_TYPE>&,<RHS_TYPE>&);
// [ 5] operator!=(const bdeut_NullableValue<LHS_TYPE>&,<RHS_TYPE>&);
// [17] operator==(const bdeut_NullableValue<TYPE>&,const TYPE&);
// [17] operator==(const TYPE&,const bdeut_NullableValue<TYPE>&,);
// [17] operator!=(const bdeut_NullableValue<TYPE>&,const TYPE&);
// [17] operator!=(const TYPE&,const bdeut_NullableValue<TYPE>&,);
// [ 4] operator<<(bsl::ostream&,const bdeut_NullableValue<TYPE>&);
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST 1: Using 'bsl::string'
// [ 2] BREATHING TEST 2: Using 'int'
// [18] USAGE EXAMPLE
// ----------------------------------------------------------------------------


//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

}  // close unnamed namespace

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

#define RUN_EACH_TYPE BSLTF_TEMPLATETESTFACILITY_RUN_EACH_TYPE
#define TEST_TYPES                                                            \
        signed char,                                                          \
        size_t,                                                               \
        bsltf::TemplateTestFacility::ObjectPtr,                               \
        bsltf::TemplateTestFacility::FunctionPtr,                             \
        bsltf::TemplateTestFacility::MethodPtr,                               \
        bsltf::EnumeratedTestType::Enum,                                      \
        bsltf::UnionTestType,                                                 \
        bsltf::SimpleTestType,                                                \
        bsltf::AllocTestType,                                                 \
        bsltf::BitwiseMoveableTestType,                                       \
        bsltf::AllocBitwiseMoveableTestType
    // This list of test types is a combination of
    // BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_PRIMITIVE and *_USER_DEFINED,
    // minus 'NonTypicalOverloadsTestType' which does not work with
    // 'bslalg::ConstructorProxy'.
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

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)

// ============================================================================
//                       GLOBAL TEST VALUES
// ----------------------------------------------------------------------------

static bool         verbose;
static bool     veryVerbose;
static bool veryVeryVerbose;
static bool testAllocatorVerbosity;

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

enum MessageType {
    // Type used for testing 'makeValue' in case 12.

    JUNK,
    IMPORTANT
};

class Recipient {
    // This 'class' is used for testing 'makeValue' in case 12.  The 'class'
    // has an explicit conversion constructor such that if the implementation
    // of 'makeValue' relies on implicit conversion from 'OTHER_TYPE' to
    // 'TYPE', the code will not compile.

    // DATA
    MessageType d_msgType;

    // FRIENDS
    friend bool operator==(const Recipient&, const Recipient&);

  public:
    explicit Recipient(const MessageType& msgType) : d_msgType(msgType)
        // Explicitly constructs a 'Recipient' object using the specified
        // 'msgType'.
    {
    }

    void operator=(const MessageType& rhs)
    {
        d_msgType = rhs;
    }

    MessageType getMsgType()
    {
        return d_msgType;
    }
};

bool operator==(const Recipient& lhs, const Recipient& rhs)
{
    return lhs.d_msgType == rhs.d_msgType;
}

struct Swappable {
    int d_value;
    static int d_swap_called;

    static void swap_reset()
    {
        d_swap_called = 0;
    }

    static int swap_called()
    {
        return d_swap_called;
    }

    explicit
    Swappable(int v)
        : d_value(v)
    {}

    bool operator==(const Swappable& rhs) const
    {
        return d_value == rhs.d_value;
    }
};

int Swappable::d_swap_called = 0;

void swap(Swappable& lhs, Swappable& rhs)
{
    ++Swappable::d_swap_called;

    bsl::swap(lhs.d_value, rhs.d_value);
}


template <class TEST_TYPE>
class TestDriver {
    // This templatized struct provide a namespace for testing
    // 'bdeut_NullableValue'.  The parameterized 'TEST_TYPE' specifies the type
    // contained in the nullable-value.


  private:
    // PRIVATE TYPES
    typedef bdeut_NullableValue<TEST_TYPE> Obj;
        // Type under test.

    typedef bslalg::ConstructorProxy<Obj> ObjWithAllocator;
        // Wrapper for 'Obj' whose constructor takes an allocator.

    typedef bsltf::TestValuesArray<TEST_TYPE> TestValues;
        // Array of test values of 'TEST_TYPE'.

  public:
    static void testCase14();
        // Test 'T valueOr(const T&)'

    static void testCase15();
        // Test 'const T *valueOr(const T*)'

    static void testCase16();
        // Test 'valueOrNull'

    static void testCase17();
        // Test comparisons with the contained 'TYPE'.
};

template <class TEST_TYPE>
void TestDriver<TEST_TYPE>::testCase14()
{
    // ------------------------------------------------------------------------
    // TESTING: 'T valueOr(const T&)'
    // Concerns:
    //: 1 'valueOr' returns the supplied value if the nullable value is null
    //:
    //: 2 'valueOr' returns the contained value value if the nullable
    //:   value is not-null
    //:
    //: 3 'valueOr' returns by value.
    //:
    //: 4 'valueOr' can be called on a 'const' object.
    //
    // Plan:
    //: 1 Create a member-function pointer matching the expected signature,
    //:   and assign 'valueOr' to the function (C-3)
    //:
    //: 2 Call 'valueOr' for a null nullable value and verify that it
    //:   returns the supplied value. (C-2)
    //:
    //: 3 For a series of test values, assign the nullable value to the test
    //:   value, and call 'valueOr' and verify the return value is the test
    //:   value (C-2, C-4)
    //
    // Testing:
    //   TYPE valueOr(const TYPE&) const;
    // ------------------------------------------------------------------------

    const TestValues VALUES;
    const int        NUM_VALUES = VALUES.size();

    bslma::TestAllocator da("default", testAllocatorVerbosity);
    bslma::TestAllocator oa("object", testAllocatorVerbosity);

    bslma::DefaultAllocatorGuard dag(&da);

    if (veryVerbose) {
        cout << "\tCompile time verification the function returns by value\n";
    }
    {
        typedef TEST_TYPE (Obj::* MemberFunction)(const TEST_TYPE& type) const;
        MemberFunction memberFunction = &Obj::valueOr;
        (void **)&memberFunction;
    }

    if (veryVerbose) {
        cout << "\tVerify null nullable-values return 0\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

        for (int i = 0; i < NUM_VALUES; ++i) {

            ASSERT(VALUES[i] == x.valueOr(VALUES[i]));
            ASSERT(VALUES[i] == X.valueOr(VALUES[i]));

            ASSERT(true == X.isNull());

            ASSERT(0 == oa.numBlocksInUse());
        }
    }

    if (veryVerbose) {
        cout << "\tVerify non-null nullable-values return underlying value\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        for (int i = 0; i < NUM_VALUES; ++i) {
            ASSERT(0 == oa.numBlocksInUse());
            ASSERT(0 == da.numBlocksInUse());

            ASSERT(VALUES[i] == x.valueOr(VALUES[i]));
            ASSERT(VALUES[i] == X.valueOr(VALUES[i]));

            x = VALUES[0];

            bslma::TestAllocatorMonitor oam(&oa);
            bslma::TestAllocatorMonitor dam(&da);

            ASSERT(VALUES[0] == x.valueOr(VALUES[i]));
            ASSERT(VALUES[0] == X.valueOr(VALUES[i]));

            ASSERT(i == 0 || VALUES[i] != x.valueOr(VALUES[i]));
            ASSERT(i == 0 || VALUES[i] != X.valueOr(VALUES[i]));

            bool usesAllocator = bslma::UsesBslmaAllocator<TEST_TYPE>::value;
            ASSERT(usesAllocator ? dam.isTotalUp() : dam.isTotalSame());
            ASSERT(oam.isInUseSame());

            x.reset();
        }

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

    }
}

template <class TEST_TYPE>
void TestDriver<TEST_TYPE>::testCase15()
{
    // ------------------------------------------------------------------------
    // TESTING: 'const T* valueOr(const T*)'
    // Concerns:
    //: 1 'valueOr' returns the supplied value if the nullable value is null
    //:
    //: 2 'valueOr' returns the contained value value if the nullable
    //:   value is not-null
    //:
    //: 3 'valueOr' returns an address.
    //:
    //: 4 'valueOr' can be called on a 'const' object.
    //:
    //: 5 No memory is requested of any allocator (global, default, this
    //:   object, supplied object)
    //
    // Plan:
    //: 1 Create a member-function pointer matching the expected signature,
    //:   and assign 'valueOr' to the function (C-3)
    //:
    //: 2 Call 'valueOr' for a null nullable value and verify that it
    //:   returns a reference to the supplied value. (C-2)
    //:
    //: 3 For a series of test values, assign the nullable value to the test
    //:   value, and call 'valueOr' and verify the return value is a
    //:   reference to the contained value (C-2, C-4)
    //
    // Testing:
    //   const TYPE *valueOr(const TYPE*) const;
    // ------------------------------------------------------------------------

    const TestValues VALUES;
    const int        NUM_VALUES = VALUES.size();

    bslma::TestAllocator da("default", testAllocatorVerbosity);
    bslma::TestAllocator oa("object", testAllocatorVerbosity);

    bslma::DefaultAllocatorGuard dag(&da);

    if (veryVerbose) {
        cout << "\tCompile time verify the function returns an address\n";
    }
    {
        typedef const TEST_TYPE *
                             (Obj::* MemberFunction)(const TEST_TYPE *) const;
        MemberFunction memberFunction = &Obj::valueOr;
        (void **)&memberFunction;
    }

    if (veryVerbose) {
        cout << "\tVerify null nullable-values return 0\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

        for (int i = 0; i < NUM_VALUES; ++i) {

            ASSERT(VALUES[i] == *x.valueOr(&VALUES[i]));
            ASSERT(VALUES[i] == *X.valueOr(&VALUES[i]));

            ASSERT(&VALUES[i] == x.valueOr(&VALUES[i]));
            ASSERT(&VALUES[i] == X.valueOr(&VALUES[i]));

            ASSERT(true == X.isNull());

            ASSERT(0 == oa.numBlocksInUse());
        }
    }

    if (veryVerbose) {
        cout << "\tVerify non-null nullable-values return underlying value\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        for (int i = 0; i < NUM_VALUES; ++i) {
            ASSERT(0 == oa.numBlocksInUse());
            ASSERT(0 == da.numBlocksInUse());

            ASSERT(VALUES[i] == *x.valueOr(&VALUES[i]));
            ASSERT(VALUES[i] == *X.valueOr(&VALUES[i]));

            ASSERT(&VALUES[i] == x.valueOr(&VALUES[i]));
            ASSERT(&VALUES[i] == X.valueOr(&VALUES[i]));

            x = VALUES[0];

            bslma::TestAllocatorMonitor oam(&oa);

            ASSERT(VALUES[0] == *x.valueOr(&VALUES[i]));
            ASSERT(VALUES[0] == *X.valueOr(&VALUES[i]));

            ASSERT(i == 0 || VALUES[i] != *x.valueOr(&VALUES[i]));
            ASSERT(i == 0 || VALUES[i] != *X.valueOr(&VALUES[i]));

            ASSERT(&VALUES[i] != x.valueOr(&VALUES[i]));
            ASSERT(&VALUES[i] != X.valueOr(&VALUES[i]));

            ASSERT(&X.value() == x.valueOr(&VALUES[i]));
            ASSERT(&X.value() == X.valueOr(&VALUES[i]));


            ASSERT(oam.isInUseSame());
            ASSERT(0 == da.numBlocksInUse());

            x.reset();
        }

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

    }
}

template <class TEST_TYPE>
void TestDriver<TEST_TYPE>::testCase16()
{
    // ------------------------------------------------------------------------
    // TESTING: 'valueOrNull'
    // Concerns:
    //: 1 'valueOrNull' returns 0 if the nullable value is null
    //:
    //: 2 'valueOrNull' returns a the address of the non-modifiable value if
    //:   the nullable value is not-null
    //:
    //: 3 The returned address, if not 0, remains valid until the
    //:   nullable-value is destroyed.
    //:
    //: 4 No memory allocation is performed.
    //:
    //: 5 'valueOrNull' can be called on a 'const' object.
    //
    // Plan:
    //: 1 Call 'valueOrNull' for a null nullable value and verify that it
    //:   returns 0. (C-1)
    //:
    //: 2 For a series of test values, assign the nullable value to the test
    //:   value, and call 'valueOrNull' and verify the return value. (C-2,
    //:   C-3, C-4, C-5)
    //
    // Testing:
    //   const TYPE* valueOrNull() const;
    // ------------------------------------------------------------------------

    const TestValues VALUES;
    const int        NUM_VALUES = VALUES.size();

    bslma::TestAllocator da("default", testAllocatorVerbosity);
    bslma::TestAllocator oa("object", testAllocatorVerbosity);

    bslma::DefaultAllocatorGuard dag(&da);
    if (veryVerbose) {
        cout << "\tVerify null nullable-values return 0\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        ASSERT(0 == x.valueOrNull());
        ASSERT(0 == X.valueOrNull());

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

        x = VALUES[0];

        ASSERT(0 != x.valueOrNull());
        ASSERT(0 != X.valueOrNull());

        x.reset();

        ASSERT(0 == x.valueOrNull());
        ASSERT(0 == X.valueOrNull());

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());


    }

    if (veryVerbose) {
        cout << "\tVerify non-null nullable-values return underlying value\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;


        for (int i = 0; i < NUM_VALUES; ++i) {
            ASSERT(0 == x.valueOrNull());
            ASSERT(0 == X.valueOrNull());

            ASSERT(0 == oa.numBlocksInUse());
            ASSERT(0 == da.numBlocksInUse());


            x = VALUES[i];

            ASSERT(0 != x.valueOrNull());
            ASSERT(0 != X.valueOrNull());

            bslma::TestAllocatorMonitor oam(&oa);

            ASSERT(X.valueOrNull() == x.valueOrNull());
            ASSERT(&X.value()      == X.valueOrNull());

            const TEST_TYPE *valuePtr = X.valueOrNull();

            ASSERT(VALUES[i] == *valuePtr);


            ASSERT(oam.isInUseSame());
            ASSERT(0 == da.numBlocksInUse());

            x.reset();
        }
    }
}

template <class TEST_TYPE>
void TestDriver<TEST_TYPE>::testCase17()
{
    // ------------------------------------------------------------------------
    // TESTING: Comparison with the contained 'TYPE'
    // Concerns:
    //: 1 Comparing a value with a null value always returns that
    //:   the values are not the same.
    //:
    //: 2 Comparing a value with a nullable value having the same value
    //:   returns the values are the same.
    //:
    //: 3 Comparing a value with a nullable value having a different value
    //:   returns the values are not the same.
    //:
    //: 4 Both operators can be called for 'const' objects.
    //:
    //: 5 No memory is allocated
    //
    // Plan:
    //: 1 Create a null nullable-value and verify it does not compare equal
    //:   (using the 4 different operator variants) to any test-value. (C-1)
    //:
    //: 2 Create a non-null nullable-value for a series of test values and
    //:   verify it does not compares equal (using the 4 different operator
    //:   variants) only to the same test-value. (C-2, C-3)
    //
    // Testing:
    //   bool operator==(const bdeut_NullableValue<TYPE>& , const TYPE&);
    //   bool operator==(const TYPE& , const bdeut_NullableValue<TYPE>&);
    //   bool operator!=(const bdeut_NullableValue<TYPE>& , const TYPE&);
    //   bool operator!=(const TYPE& , const bdeut_NullableValue<TYPE>&);
    // ------------------------------------------------------------------------

    const TestValues VALUES;
    const int        NUM_VALUES = VALUES.size();

    bslma::TestAllocator da("default", testAllocatorVerbosity);
    bslma::TestAllocator oa("object", testAllocatorVerbosity);

    bslma::DefaultAllocatorGuard dag(&da);

    if (veryVerbose) {
        cout << "\tVerify null nullable-values are different from any value\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());

        for (int i = 0; i < NUM_VALUES; ++i) {

            ASSERT(!(VALUES[i] == X));
            ASSERT( (VALUES[i] != X));

            ASSERT(!(X == VALUES[i]));
            ASSERT( (X != VALUES[i]));
        }
    }

    if (veryVerbose) {
        cout << "\tVerify non-null nullable-values compare equal "
             << "to a value of the contained type\n";
    }
    {
        ObjWithAllocator object(&oa);
        Obj& x = object.object(); const Obj& X = x;

        for (int i = 0; i < NUM_VALUES; ++i) {
            for (int j = 0; j < NUM_VALUES; ++j) {

            ASSERT(0 == oa.numBlocksInUse());
            ASSERT(0 == da.numBlocksInUse());

            x = VALUES[i];

            bslma::TestAllocatorMonitor oam(&oa);

            bool areSame = i == j;

            ASSERT( areSame == (VALUES[j] == X));
            ASSERT(!areSame == (VALUES[j] != X));

            ASSERT( areSame == (X == VALUES[j]));
            ASSERT(!areSame == (X != VALUES[j]));

            ASSERT(oam.isInUseSame());
            ASSERT(0 == da.numBlocksInUse());

            x.reset();
            }
        }
        ASSERT(0 == oa.numBlocksInUse());
        ASSERT(0 == da.numBlocksInUse());
    }
}


//=============================================================================
//                               USAGE EXAMPLE
//-----------------------------------------------------------------------------

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;

    verbose = argc > 2;
    veryVerbose = argc > 3;
    veryVeryVerbose = argc > 4;
    testAllocatorVerbosity = argc > 5;  // always the last

    cout << "TEST " << __FILE__ << " CASE " << test << endl;;

    bslma::TestAllocator  testAllocator(testAllocatorVerbosity);
    bslma::TestAllocator *ALLOC = &testAllocator;
    bslma::TestAllocator globalAllocator(testAllocatorVerbosity);

    bslma::Default::setGlobalAllocator(&globalAllocator);

    switch (test) { case 0:  // Zero is always the leading case.
      case 19: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //
        // Concerns:
        //   - That it works.
        //
        // Plan:
        //   Paste it here, replace assert with ASSERT.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << "\nUSAGE EXAMPLE"
                          << "\n=============" << endl;

///Usage Example
///-------------
// The following snippet of codes illustrates the usage of this component:
//..
    bdeut_NullableValue<int> nullableInt;
    ASSERT(nullableInt.isNull());

    nullableInt.makeValue(123);
    ASSERT(!nullableInt.isNull());
    ASSERT(123 == nullableInt.value());

    nullableInt.reset();
    ASSERT(nullableInt.isNull());
//..

      } break;
      case 18: {
        // --------------------------------------------------------------------
        // NEGATIVE TESTING OF ASSERT ON INVALID USE OF NULL VALUE
        // --------------------------------------------------------------------

        typedef int                            ValueType;
        typedef bdeut_NullableValue<ValueType> Obj;

        Obj mX1;    const Obj& X1 = mX1;

        bsls::AssertTestHandlerGuard guard;

        ASSERT(X1.isNull());

        ASSERT_SAFE_FAIL(0 == X1.value());

        mX1 = 5;

        ASSERT_SAFE_PASS(5 == X1.value());
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // Comparison with the contained 'TYPE'
        // --------------------------------------------------------------------

          if (verbose) cout << "\nTesting: Comparison with contained 'TYPE'"
                            << "\n=========================================\n";

        RUN_EACH_TYPE(TestDriver, testCase17, TEST_TYPES);

      } break;

      case 16: {
        // --------------------------------------------------------------------
        // valueOrNull
        // --------------------------------------------------------------------

          if (verbose) cout << "\nTesting: valueOrNull"
                            << "\n====================\n";

        RUN_EACH_TYPE(TestDriver, testCase16, TEST_TYPES);


      } break;
      case 15: {
        // --------------------------------------------------------------------
        // valueOr
        // --------------------------------------------------------------------

          if (verbose) cout << "\nTesting: const T *valueOr(const T*)"
                            << "\n===================================\n";

          RUN_EACH_TYPE(TestDriver, testCase15, TEST_TYPES);

      } break;
      case 14: {
        // --------------------------------------------------------------------
        // valueOr
        // --------------------------------------------------------------------

          if (verbose) cout << "\nTesting: T valueOr(const T&)"
                            << "\n===================================\n";

        RUN_EACH_TYPE(TestDriver, testCase14, TEST_TYPES);

      } break;
      case 13: {
        // --------------------------------------------------------------------
        // TESTING SWAP METHOD
        //
        // Concerns:
        //   1. swap for two null objects is a no-op,
        //   2. swap for null and non-null moves the value from one object to
        //      another without calling swap for the value type,
        //   3. swap for two non-null objects calls swap for the value type.
        //
        // Plan:
        //   Create a value type class, 'Swappable', with a swap method
        //   instrumented to track swap calls.  Instantiate bdeut_NullableValue
        //   with that type and execute operations needed to verify the
        //   concerns.
        //
        // Testing:
        //   void swap(bdeut_NullableValue<TYPE>& other);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Swap Method"
                          << "\n==================="
                          << endl;

        using bsl::swap;

        {
            // swap for two null objects is a no-op
            bdeut_NullableValue<Swappable> nullObj1;
            bdeut_NullableValue<Swappable> nullObj2;

            Swappable::swap_reset();
            swap(nullObj1, nullObj2);

            ASSERT(!Swappable::swap_called());
            ASSERT(nullObj1.isNull());
            ASSERT(nullObj2.isNull());
        }

        {
            // swap for null and non-null moves the value from one object to
            // another without calling swap for the value type.
            bdeut_NullableValue<Swappable> nonNullObj(Swappable(10));
            bdeut_NullableValue<Swappable> nonNullObjCopy(nonNullObj);
            bdeut_NullableValue<Swappable> nullObj;

            Swappable::swap_reset();
            swap(nonNullObj, nullObj);

            ASSERT(!Swappable::swap_called());
            ASSERT(nonNullObjCopy == nullObj);
            ASSERT(nonNullObj.isNull());
        }

        {
            // swap for two non-null objects calls swap for the value type.
            bdeut_NullableValue<Swappable> obj1(Swappable(10));
            bdeut_NullableValue<Swappable> obj2(Swappable(20));

            bdeut_NullableValue<Swappable> obj1Copy(obj1);
            bdeut_NullableValue<Swappable> obj2Copy(obj2);

            Swappable::swap_reset();
            swap(obj1, obj2);

            ASSERT(Swappable::swap_called());
            ASSERT(obj1 == obj2Copy);
            ASSERT(obj2 == obj1Copy);
        }
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // TESTING CONVERSION ASSIGNMENT OPERATIONS
        //
        // Concerns:
        //   - That convertible underlying types convert.
        //   - That types with explicit conversion constructor will properly
        //     compile and work with 'bdeut_NullableValue'.
        //   - That types for which there is no conversion do not compile
        //     (we will do this test by hand for now, but could use template
        //     magic later, perhaps, to ensure that non-compile is enforced
        //     by the compiler).
        //
        // Plan:
        //   Conduct the regular test using 'int' and 'double'.  Then try
        //   'bsl::string' and 'char *' to observe with allocators involved.
        //   Then try 'Recipient' and 'Message' to test for explicit conversion
        //   constructors.  Finally, try try 'int' and 'bsl::string' by hand
        //   and watch it not compile.
        //
        // Testing:
        //   operator=(const bdeut_NullableValue<OTHER_TYPE>& rhs);
        //   operator=(const OTHER_TYPE& rhs);
        //   TYPE& makeValue(const OTHER_TYPE& value);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Conversion Assignment Operations"
                          << "\n========================================"
                          << endl;

        if (verbose) cout << "\nUsing 'int' and 'double." << endl;

        {
            typedef int    ValueType1;
            typedef double ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;
            const ValueType2 VALUE2 = 456;

            if (verbose) cout << "\tcopy assignment" << endl;

            const ObjType1 OBJ1(VALUE1);
                  ObjType2 obj2(VALUE2);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = OBJ1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tvalue assignment" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = VALUE1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tmake value" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2.makeValue(VALUE1);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());
        }

        if (verbose) cout << "\nUsing 'double' and 'int." << endl;

        {
            typedef double ValueType1;
            typedef int    ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;
            const ValueType2 VALUE2 = 456;

            if (verbose) cout << "\tcopy assignment" << endl;

            const ObjType1 OBJ1(VALUE1);
                  ObjType2 obj2(VALUE2);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = OBJ1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tvalue assignment" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = VALUE1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tmake value" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2.makeValue(VALUE1);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());
        }


        if (verbose) cout
                << "\nUsing 'bsl::string' and 'char *' + ALLOC." << endl;

        {
            typedef const char *ValueType1;
            typedef bsl::string ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = "abc";
            const ValueType2 VALUE2 = "def";

            if (verbose) cout << "\tcopy assignment" << endl;

            const ObjType1 OBJ1(VALUE1);
                  ObjType2 obj2(VALUE2, ALLOC);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = OBJ1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tvalue assignment" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = VALUE1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tmake value" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2.makeValue(VALUE1);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value());
        }

        // Making sure makeValue works with explicit constructors.

        if (verbose) cout
                << "\nUsing 'Recipient' and 'MessageType'." << endl;

        {
            typedef MessageType ValueType1;
            typedef Recipient   ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = IMPORTANT;
            const ValueType2 VALUE2(JUNK);

            if (verbose) cout << "\tcopy assignment" << endl;

            const ObjType1 OBJ1(VALUE1);
                  ObjType2 obj2(VALUE2);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = OBJ1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value().getMsgType());

            if (verbose) cout << "\tvalue assignment" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = VALUE1;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value().getMsgType());

            if (verbose) cout << "\tmake value" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2.makeValue(VALUE1);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE1 == obj2.value().getMsgType());
        }

//#define SOMETHING_ELSE_THAT_SHOULD_NOT_WORK
#ifdef SOMETHING_ELSE_THAT_SHOULD_NOT_WORK
        if (verbose) cout
                << "\nUsing 'bsl::string' and 'int'." << endl;

        {
            typedef int         ValueType1;
            typedef bsl::string ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;
            const ValueType2 VALUE2 = "def";

            if (verbose) cout << "\tcopy assignment" << endl;

            const ObjType1 OBJ1(VALUE1);
                  ObjType2 obj2(VALUE2, ALLOC);

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = OBJ1;

            ASSERT(VALUE1 == OBJ1.value());
            //ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tvalue assignment" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2 = VALUE1;

            ASSERT(VALUE1 == OBJ1.value());
            //ASSERT(VALUE1 == obj2.value());

            if (verbose) cout << "\tmake value" << endl;

            obj2 = VALUE2;

            ASSERT(VALUE1 == OBJ1.value());
            ASSERT(VALUE2 == obj2.value());

            obj2.makeValue(VALUE1);

            ASSERT(VALUE1 == OBJ1.value());
            //ASSERT(VALUE1 == obj2.value());
        }

#endif

      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING CONVERSION CONSTRUCTORS
        //
        // Concerns:
        //   - That convertible underlying types convert.
        //   - That types for which there is no conversion do not compile
        //     (we will do this test by hand for now, but could use template
        //     magic later, perhaps, to ensure that non-compile is enforced
        //     by the compiler).
        //
        // Plan:
        //   Conduct the regular test using 'int' and 'double'.  Then try
        //   'bsl::string' and 'char *' to observe with allocators involved
        //   Finally, try try 'int' and 'bsl::string' by hand and watch it not
        //   compile.
        //
        // Testing:
        //   bdeut_NullableValue(const bdeut_NullableValue<ORIGINAL>&o);
        //   bdeut_NullableValue(const bdeut_NullableValue<ORIGINAL>&o,*ba);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Conversion Constructors"
                          << "\n===============================" << endl;

        if (verbose) cout << "\nUsing 'int' and 'double." << endl;

        {
            typedef int                            ValueType1;
            typedef double                         ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;

            const ObjType1 OBJ1(VALUE1);
            const ObjType2 OBJ2(OBJ1);

            ASSERT(VALUE1             == OBJ1.value());
            ASSERT(ValueType2(VALUE1) == OBJ2.value());
        }

        if (verbose) cout << "\nUsing 'double' and 'int'." << endl;

        {
            typedef double                          ValueType1;
            typedef int                             ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;

            const ObjType1 OBJ1(VALUE1);
            const ObjType2 OBJ2(OBJ1);

            ASSERT(VALUE1             == OBJ1.value());
            ASSERT(ValueType2(VALUE1) == OBJ2.value());
        }

        if (verbose) cout
                << "\nUsing 'bsl::string' and 'char *' + ALLOC." << endl;

        {
            typedef char *                          ValueType1;
            typedef bsl::string                     ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            char p[] = "Hello";

            const ValueType1 VALUE1 = p;

            const ObjType1 OBJ1(VALUE1);
            const ObjType2 OBJ2(OBJ1, ALLOC);  // <<<=== ALLOC !!!

            ASSERT(VALUE1             == OBJ1.value());
            ASSERT(ValueType2(VALUE1) == OBJ2.value());
        }


//#define SOMETHING_THAT_SHOULD_NOT_WORK
#ifdef SOMETHING_THAT_SHOULD_NOT_WORK

        if (verbose) cout << "\nUsing 'bsl::string' and 'int'." << endl;

        {
            typedef int                             ValueType1;
            typedef bsl::string                     ValueType2;

            typedef bdeut_NullableValue<ValueType1> ObjType1;
            typedef bdeut_NullableValue<ValueType2> ObjType2;

            const ValueType1 VALUE1 = 123;

            const ObjType1 OBJ1(VALUE1);
            const ObjType2 OBJ2(OBJ1);
            const ObjType2 OBJ3(OBJ1, ALLOC);

            ASSERT(VALUE1             == OBJ1.value());
            //ASSERT(ValueType2(VALUE1) == OBJ2.value());
        }
#endif

      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING VALUE ASSIGNMENT METHODS
        //
        // Concerns:
        //   - Make sure we can change values.
        //
        // Plan:
        //   Conduct the test using 'int' (does not use allocator) and
        //   'bsl::string' (uses allocator) for 'TYPE'.
        //
        // Testing:
        //   TYPE& operator=(const TYPE& rhs);
        //   TYPE& makeValue();
        //   void reset();
        //   TYPE& value();
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Value Assignement Methods"
                          << "\n=================================" << endl;

        if (verbose) cout << "\nUsing 'bdeut_NullableValue<int>." << endl;

        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const ValueType VALUE1 = 123;
            const ValueType VALUE2 = 456;

            bdeut_NullableValue<ValueType> mA;
            bdeut_NullableValue<ValueType>& A = mA;
            ASSERT (A.isNull());

            mA = VALUE1;
            ASSERT (!A.isNull());
            ASSERT (VALUE1 == A.value());

            mA.makeValue();
            ASSERT (!A.isNull());
            ASSERT (ValueType() == A.value());

            mA.value() = VALUE2;
            ASSERT (!A.isNull());
            ASSERT (VALUE2 == A.value());

            mA.reset();
            ASSERT (A.isNull());
        }

        if (verbose) cout << "\nUsing bdeut_NullableValue<bsl::string>."
                          << endl;

        {
            typedef bsl::string                    ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const ValueType VALUE1 = "abc";
            const ValueType VALUE2 = "def";

            bdeut_NullableValue<ValueType> mA;
            bdeut_NullableValue<ValueType>& A = mA;
            ASSERT (A.isNull());

            mA = VALUE1;
            ASSERT (!A.isNull());
            ASSERT (VALUE1 == A.value());

            mA.makeValue();
            ASSERT (!A.isNull());
            ASSERT (ValueType() == A.value());

            mA.value() = VALUE2;
            ASSERT (!A.isNull());
            ASSERT (VALUE2 == A.value());

            mA.reset();
            ASSERT (A.isNull());
        }

      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING VALUE CONSTRUCTORS
        //
        // Concerns:
        //   - Any value must be able to be copy constructed without affecting
        //     its argument.
        //   - Should work with and without a supplied allocator.
        //
        // Plan:
        //   Conduct the test using 'int' (does not use allocator) and
        //   'bsl::string' (uses allocator) for 'TYPE'.
        //
        // Testing:
        //   bdeut_NullableValue(const TYPE& value);
        //   bdeut_NullableValue(const TYPE& value, *ba);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Value Constructors"
                          << "\n==========================" << endl;

        if (verbose) cout << "\nUsing 'bdeut_NullableValue<int>." << endl;

        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const ValueType VALUE1 = 123;
            const ValueType VALUE2 = 456;

            const bdeut_NullableValue<ValueType> A(VALUE1);
            ASSERT(VALUE1 == A.value());

            const bdeut_NullableValue<ValueType> B(VALUE2);
            ASSERT(VALUE2 == B.value());
        }

        if (verbose) cout << "\nUsing bdeut_NullableValue<bsl::string>."
                          << endl;

        {
            typedef bsl::string                    ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const ValueType VALUE1 = "abc";
            const ValueType VALUE2 = "def";

            const bdeut_NullableValue<ValueType> A(VALUE1, ALLOC);
            ASSERT(VALUE1 == A.value());

            const bdeut_NullableValue<ValueType> B(VALUE2, ALLOC);
            ASSERT(VALUE2 == B.value());
        }

      } break;
      case 8: {
        // --------------------------------------------------------------------
        // TESTING BDEX_STREAMING OPERATIONS
        //
        // Concerns:
        //   That the stream operations work.
        //
        // Plan:
        //   Use 'int' for 'TYPE' - stream it out and stream it back.
        //
        // Testing:
        //   STREAM& bdexStreamIn(STREAM& stream, int version);
        //   STREAM& bdexStreamOut(STREAM& stream, int version) const;
        //   int maxSupportedBdexVersion() const;
        // --------------------------------------------------------------------
        typedef bdex_TestInStream  In;
        typedef bdex_TestOutStream Out;

        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const Obj X(123);

            Out       out;
            const int VERSION = X.maxSupportedBdexVersion();

            X.bdexStreamOut(out, VERSION);

            const char *const OD  = out.data();
            const int         LOD = out.length();
            In                in(OD, LOD);              ASSERT(in);
                                                        ASSERT(!in.isEmpty());
            Obj               t;                        ASSERT(X != t);

            in.setSuppressVersionCheck(1);  // needed for direct method test
            t.bdexStreamIn(in, VERSION);                ASSERT(X == t);
            ASSERT(in);                                 ASSERT(in.isEmpty());
        }
        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const Obj X;
            Out       out;
            const int VERSION = X.maxSupportedBdexVersion();

            X.bdexStreamOut(out, VERSION);

            const char *const OD  = out.data();
            const int         LOD = out.length();
            In                in(OD, LOD);              ASSERT(in);
                                                        ASSERT(!in.isEmpty());
            Obj               t(123);                   ASSERT(X != t);

            in.setSuppressVersionCheck(1);  // needed for direct method test
            t.bdexStreamIn(in, VERSION);                ASSERT(X == t);
            ASSERT(in);                                 ASSERT(in.isEmpty());
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING COPY ASSIGNMENT OPERATOR
        //
        // Concerns:
        //   Any value must be assignable to an object having any initial value
        //   without affecting the rhs operand value.  Also, any object must be
        //   assignable to itself.
        //
        // Plan:
        //   Use 'bsl::string' for 'TYPE'.
        //
        //   Specify a set of unique values.  Construct and initialize all
        //   combinations (u, v) in the cross product.  Copy construct a
        //   control w from v, assign v to u, and assert that w == u and
        //   w == v.  Then test aliasing by copy constructing a control w from
        //   each u, assigning u to itself, and verifying that w == u.
        //
        // Testing:
        //    operator=(const bdeut_NullableValue& rhs);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Copy Assignment Operator"
                          << "\n================================" << endl;

        {
            typedef bsl::string                    ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const int NUM_VALUES = 3;

            Obj mX[NUM_VALUES];

            const ValueType VALUE1 = "123";
            const ValueType VALUE2 = "456";

            mX[1].makeValue(VALUE1);
            mX[2].makeValue(VALUE2);

            for (int i = 0; i < NUM_VALUES; ++i) {
                Obj mU(mX[i], ALLOC);  const Obj& U = mU;

                for (int j = 0; j < NUM_VALUES; ++j) {
                    Obj mV(mX[j], ALLOC);  const Obj& V = mV;

                    Obj mW(V, ALLOC);  const Obj& W = mW;

                    mU = V;

                    LOOP2_ASSERT(U, W, U == W);
                    LOOP2_ASSERT(V, W, V == W);
                }
            }

            for (int i = 0; i < NUM_VALUES; ++i) {
                Obj mU(mX[i], ALLOC);  const Obj& U = mU;
                Obj mW(U,     ALLOC);  const Obj& W = mW;

                mU = U;

                LOOP2_ASSERT(U, W, U == W);
            }
        }

        if (verbose) cout << "\nEnd of Test." << endl;
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING COPY CONSTRUCTOR
        //   This will test the copy constructor.
        //
        // Concerns:
        //   Any value must be able to be copy constructed without affecting
        //   its argument.
        //
        // Plan:
        //   Conduct the test using 'int' (does not use allocator) and
        //   'bsl::string' (uses allocator) for 'TYPE'.
        //
        //   Specify a set whose elements have substantial and varied
        //   differences in value.  For each element in S, construct and
        //   initialize identical objects W and X using tested methods.  Then
        //   copy construct Y from X and use the equality operator to assert
        //   that both X and Y have the same value as W.
        //
        // Testing:
        //   bdeut_NullableValue(const bdeut_NullableValue& original);
        //   bdeut_NullableValue(const bdeut_NullableValue& original, *ba);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Copy Constructor"
                          << "\n========================" << endl;

        if (verbose) cout << "\nUsing 'bdeut_NullableValue<int>." << endl;

        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const int NUM_VALUES = 3;

            Obj mX[NUM_VALUES];
            Obj mW[NUM_VALUES];

            const ValueType VALUE1 = 123;
            const ValueType VALUE2 = 456;

            mX[1].makeValue(VALUE1);
            mW[1].makeValue(VALUE1);

            mX[2].makeValue(VALUE2);
            mW[2].makeValue(VALUE2);

            for (int i = 0; i < NUM_VALUES; ++i) {
                const Obj& X = mX[i];
                const Obj& W = mW[i];

                Obj mY(X);  const Obj& Y = mY;

                if (veryVerbose) {
                    T_ P_(i) P_(W) P_(X) P(Y)
                }

                LOOP2_ASSERT(X, W, X == W);
                LOOP2_ASSERT(Y, W, Y == W);
            }
        }

        if (verbose) cout << "\nUsing bdeut_NullableValue<bsl::string>."
                          << endl;

        {
            typedef bsl::string                    ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            const int NUM_VALUES = 3;

            Obj mX[NUM_VALUES];
            Obj mW[NUM_VALUES];

            const ValueType VALUE1 = "123";
            const ValueType VALUE2 = "456";

            mX[1].makeValue(VALUE1);
            mW[1].makeValue(VALUE1);

            mX[2].makeValue(VALUE2);
            mW[2].makeValue(VALUE2);

            for (int i = 0; i < NUM_VALUES; ++i) {
                const Obj& X = mX[i];
                const Obj& W = mW[i];

                Obj mY(X, ALLOC);  const Obj& Y = mY;

                if (veryVerbose) {
                    T_ P_(i) P_(W) P_(X) P(Y)
                }

                LOOP2_ASSERT(X, W, X == W);
                LOOP2_ASSERT(Y, W, Y == W);
            }
        }

        if (verbose) cout << "\nEnd of Test." << endl;
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING EQUALITY OPERATORS
        //   This will test the equality operations ('operator==' and
        //   'operator!=').
        //
        // Concerns:
        //   The '==' operator must return 0 for objects that are very similar
        //   but still different, but must return 1 for objects that are
        //   exactly the same.  Likewise, 'operator!=' must return 1 for
        //   objects that are very similar but still different, but must return
        //   0 for objects that are exactly the same.
        //
        //   That objects of different, but comparable types that have the
        //   same value compare equal.
        //
        // Plan:
        //   Use 'int' for 'TYPE'.  Construct a set of objects containing
        //   similar but different date values.  Loop through the cross-product
        //   of the test data.  For each tuple, use the '==' and '!=' operators
        //   and check their return value for correctness.
        //
        // Testing:
        //   operator==(const bdeut_NullableValue<LHS_TYPE>&,<RHS_TYPE>&);
        //   operator!=(const bdeut_NullableValue<LHS_TYPE>&,<RHS_TYPE>&);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Equality Operators"
                          << "\n==========================" << endl;

        typedef int                            ValueType;
        typedef bdeut_NullableValue<ValueType> Obj;

        const int NUM_VALUES = 3;

        Obj objArray[NUM_VALUES];

        objArray[1].makeValue(123);
        objArray[2].makeValue(234);

        for (int i = 0; i < NUM_VALUES; ++i) {
            const Obj& U = objArray[i];

            if (veryVerbose) { T_ P_(i) P(U) }

            for (int j = 0; j < NUM_VALUES; ++j) {
                const Obj& V = objArray[j];

                if (veryVeryVerbose) { T_ T_ P_(j) P(V) }

                const bool isSame = (i == j);
                LOOP2_ASSERT(U, V,  isSame == (U == V));
                LOOP2_ASSERT(U, V, !isSame == (U != V));
            }
        }

        {
             const int    I1 = 1;
             const double D1 = 1;

             const int    I2 = 2;
             const double D2 = 2;

             typedef bdeut_NullableValue<int>    Ni;
             typedef bdeut_NullableValue<double> Nd;

             const Ni ni0;
             const Ni ni1(I1);
             const Ni ni2(I2);

             const Nd nd0;
             const Nd nd1(D1);
             const Ni nd2(D2);

             ASSERT(1 == (ni0 == ni0));
             ASSERT(0 == (ni0 != ni0));
             ASSERT(1 == (ni0 == nd0));
             ASSERT(0 == (ni0 != nd0));

             ASSERT(0 == (ni0 == ni1));
             ASSERT(1 == (ni0 != ni1));
             ASSERT(0 == (ni0 == nd1));
             ASSERT(1 == (ni0 != nd1));

             ASSERT(0 == (ni0 == ni2));
             ASSERT(1 == (ni0 != ni2));
             ASSERT(0 == (ni0 == nd2));
             ASSERT(1 == (ni0 != nd2));


             ASSERT(0 == (ni1 == ni0));
             ASSERT(1 == (ni1 != ni0));
             ASSERT(0 == (ni1 == nd0));
             ASSERT(1 == (ni1 != nd0));

             ASSERT(1 == (ni1 == ni1));
             ASSERT(0 == (ni1 != ni1));
             ASSERT(1 == (ni1 == nd1));
             ASSERT(0 == (ni1 != nd1));

             ASSERT(0 == (ni1 == ni2));
             ASSERT(1 == (ni1 != ni2));
             ASSERT(0 == (ni1 == nd2));
             ASSERT(1 == (ni1 != nd2));


             ASSERT(0 == (ni2 == ni0));
             ASSERT(1 == (ni2 != ni0));
             ASSERT(0 == (ni2 == nd0));
             ASSERT(1 == (ni2 != nd0));

             ASSERT(0 == (ni2 == ni1));
             ASSERT(1 == (ni2 != ni1));
             ASSERT(0 == (ni2 == nd1));
             ASSERT(1 == (ni2 != nd1));

             ASSERT(1 == (ni2 == ni2));
             ASSERT(0 == (ni2 != ni2));
             ASSERT(1 == (ni2 == nd2));
             ASSERT(0 == (ni2 != nd2));



             ASSERT(1 == (nd0 == ni0));
             ASSERT(0 == (nd0 != ni0));
             ASSERT(1 == (nd0 == nd0));
             ASSERT(0 == (nd0 != nd0));

             ASSERT(0 == (nd0 == ni1));
             ASSERT(1 == (nd0 != ni1));
             ASSERT(0 == (nd0 == nd1));
             ASSERT(1 == (nd0 != nd1));

             ASSERT(0 == (nd0 == ni2));
             ASSERT(1 == (nd0 != ni2));
             ASSERT(0 == (nd0 == nd2));
             ASSERT(1 == (nd0 != nd2));


             ASSERT(0 == (nd1 == ni0));
             ASSERT(1 == (nd1 != ni0));
             ASSERT(0 == (nd1 == nd0));
             ASSERT(1 == (nd1 != nd0));

             ASSERT(1 == (nd1 == ni1));
             ASSERT(0 == (nd1 != ni1));
             ASSERT(1 == (nd1 == nd1));
             ASSERT(0 == (nd1 != nd1));

             ASSERT(0 == (nd1 == ni2));
             ASSERT(1 == (nd1 != ni2));
             ASSERT(0 == (nd1 == nd2));
             ASSERT(1 == (nd1 != nd2));


             ASSERT(0 == (nd2 == ni0));
             ASSERT(1 == (nd2 != ni0));
             ASSERT(0 == (nd2 == nd0));
             ASSERT(1 == (nd2 != nd0));

             ASSERT(0 == (nd2 == ni1));
             ASSERT(1 == (nd2 != ni1));
             ASSERT(0 == (nd2 == nd1));
             ASSERT(1 == (nd2 != nd1));

             ASSERT(1 == (nd2 == ni2));
             ASSERT(0 == (nd2 != ni2));
             ASSERT(1 == (nd2 == nd2));
             ASSERT(0 == (nd2 != nd2));
        }

        if (verbose) cout << "\nEnd of Test." << endl;
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING PRINT METHOD AND OUTPUT (<<) OPERATOR
        //   We need to test the 'print' method and the '<<' operator.
        //
        // Concerns:
        //   The print method and output (<<) operator must work.
        //
        // Plan:
        //   Conduct the test using 'int' for 'TYPE'.
        //
        //   For a set of values, check that the 'print' and output (<<)
        //   operator work as expected.
        //
        // Testing:
        //   print(bsl::ostream& s,int l=0,int spl=4) const;
        //   operator<<(bsl::ostream&,const bdeut_NullableValue<TYPE>&);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Print Method & Output (<<) Operator"
                          << "\n==========================================="
                          << endl;

        typedef int                            ValueType;
        typedef bdeut_NullableValue<ValueType> Obj;

        if (verbose) cout << "\nTesting 'print' Method." << endl;

        {
            // TBD:
        }

        if (verbose) cout << "\nTesting Output (<<) Operator." << endl;

        {
            const ValueType VALUE1          = 123;
            const char      NULL_RESULT[]   = "NULL";
            const char      VALUE1_RESULT[] = "123";

            {
                Obj mX;  const Obj& X = mX;
                bsl::stringstream ss;
                ss << X;
                LOOP_ASSERT(ss.str(), NULL_RESULT == ss.str());
            }
            {
                Obj mX;  const Obj& X = mX;
                mX.makeValue(VALUE1);
                bsl::stringstream ss;
                ss << X;
                LOOP_ASSERT(ss.str(), VALUE1_RESULT == ss.str());
            }
        }

        if (verbose) cout << "\nEnd of Test." << endl;
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING PRIMARY MANIPULATORS AND BASIC ACCESSORS
        //   This test will verify that the primary manipulators are working as
        //   expected.  Also, we test that the basic accessors are working as
        //   expected.
        //
        // Concerns:
        //   * The default constructor must create a null object.
        //   * makeValue() must set the value to the one constructed with the
        //   default constructor.
        //   * makeValue(const TYPE&) function must set the value
        //   appropriately.
        //
        // Plan:
        //   Conduct the test using 'int' (does not use allocator) and
        //   'bsl::string' (uses allocator) for 'TYPE'.  (Check that
        //   'ValueType' is the right size in each case.)
        //
        //   First, verify the default constructor by testing that the
        //   resulting object is null.
        //
        //   Next, verify that the 'makeValue' function works by making a value
        //   equal to the value passed into 'makeValue'.
        //
        //   Note that the destructor is exercised on each configuration as the
        //   object being tested leaves scope.
        //
        // Testing:
        //   typedef TYPE ValueType;
        //   bdeut_NullableValue();
        //   bdeut_NullableValue(bslma::Allocator *basicAllocator);
        //   ~bdeut_NullableValue();
        //   TYPE& makeValue();
        //   TYPE& makeValue(const TYPE& rhs);
        //   bool isNull() const;
        //   const TYPE& value() const;
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Primary Manipulators & Basic Accessors"
                          << "\n=============================================="
                          << endl;

        if (verbose) cout << "\nUsing 'bdeut_NullableValue<int>'." << endl;

        {
            typedef int                            ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            ASSERT(sizeof(ValueType) == sizeof(Obj::ValueType));

            if (veryVerbose) cout << "\tTesting default constructor." << endl;

            {
                Obj mX;  const Obj& X = mX;
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(X.isNull());
            }

            if (veryVerbose) cout << "\tTesting 'makeValue'." << endl;

            {
                Obj mX;  const Obj& X = mX;

                mX.makeValue();
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), ValueType() == X.value());
            }

            {
                Obj mX;  const Obj& X = mX;

                mX.makeValue(3);  // set some random value
                mX.makeValue();   // reset to default
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), ValueType() == X.value());
            }

            {
                Obj mX;  const Obj& X = mX;

                const ValueType VALUE1 = 123;

                mX.makeValue(VALUE1);
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), VALUE1 == X.value());
            }

            {
                Obj mX;  const Obj& X = mX;

                const ValueType VALUE1 = 123;
                const ValueType VALUE2 = 456;

                mX.makeValue(VALUE1);
                mX.makeValue(VALUE2);
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), VALUE2 == X.value());
            }
        }

        if (verbose) cout << "\nUsing 'bdeut_NullableValue<bsl::string>'."
                          << endl;

        {
            typedef bsl::string                    ValueType;
            typedef bdeut_NullableValue<ValueType> Obj;

            ASSERT(sizeof(ValueType) == sizeof(Obj::ValueType));

            if (veryVerbose) cout << "\tTesting default constructor." << endl;

            {
                Obj mX(ALLOC);  const Obj& X = mX;
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(X.isNull());
            }

            if (veryVerbose) cout << "\tTesting 'makeValue'." << endl;

            {
                Obj mX(ALLOC);  const Obj& X = mX;

                mX.makeValue();
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), ValueType() == X.value());
            }

            {
                Obj mX(ALLOC);  const Obj& X = mX;

                mX.makeValue("3");  // set some random value
                mX.makeValue();     // reset to default
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), ValueType() == X.value());
            }

            {
                Obj mX(ALLOC);  const Obj& X = mX;

                const ValueType VALUE1 = "123";

                mX.makeValue(VALUE1);
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), VALUE1 == X.value());
            }

            {
                Obj mX(ALLOC);  const Obj& X = mX;

                const ValueType VALUE1 = "123";
                const ValueType VALUE2 = "456";

                mX.makeValue(VALUE1);
                mX.makeValue(VALUE2);
                if (veryVeryVerbose) { T_ T_ P(X) };
                ASSERT(!X.isNull());
                LOOP_ASSERT(X.value(), VALUE2 == X.value());
            }
        }

        if (verbose) cout << "\nEnd of Test." << endl;
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // BREATHING TEST 2: Using 'int'
        //   This test exercises basic functionality, but tests nothing.
        //
        // Concerns:
        //   We want to demonstrate a base-line level of correct operation of
        //   the following methods and operators:
        //     - default and copy constructors.
        //     - the assignment operator (including aliasing).
        //     - equality operators: 'operator==' and 'operator!='.
        //     - primary manipulators: 'makeValue' and 'reset'.
        //     - basic accessors: 'value' and 'isNull'.
        //
        // Plan:
        //   Create four test objects using the default, initializing, and copy
        //   constructors.  Exercise the basic value-semantic methods and the
        //   equality operators using the test objects.  Invoke the primary
        //   manipulator [5, 6, 7], copy constructor [2, 4], assignment
        //   operator without [9, 9] and with [10] aliasing.  Use the basic
        //   accessors to verify the expected results.  Display object values
        //   frequently in verbose mode.  Note that 'VA', 'VB' and 'VC' denote
        //   unique, but otherwise arbitrary, object values, while 'U' denotes
        //   the valid, but "unknown", default object value.
        //
        //    1. Create an object x1 (init. to VA)  { x1:VA                  }
        //    2. Create an object x2 (copy of x1)   { x1:VA x2:VA            }
        //    3. Create an object x3 (default ctor) { x1:VA x2:VA x3:U       }
        //    4. Create an object x4 (copy of x3)   { x1:VA x2:VA x3:U  x4:U }
        //    5. Set x3 using 'makeValue' (set to VB){ x1:VA x2:VA x3:VB x4:U }
        //    6. Change x1 using 'reset'            { x1:U  x2:VA x3:VB x4:U }
        //    7. Change x1 ('makeValue', set to VC) { x1:VC x2:VA x3:VB x4:U }
        //    8. Assign x2 = x1                     { x1:VC x2:VC x3:VB x4:U }
        //    9. Assign x2 = x3                     { x1:VC x2:VB x3:VB x4:U }
        //   10. Assign x1 = x1 (aliasing)          { x1:VC x2:VB x3:VB x4:U }
        //
        // Testing:
        //   BREATHING TEST 2: Using 'int'
        // --------------------------------------------------------------------

        if (verbose) cout << "\nBREATHING TEST 2: Using 'int'"
                          << "\n=============================" << endl;


        typedef int                            ValueType;
        typedef bdeut_NullableValue<ValueType> Obj;

        // possible values
        const ValueType VA(123);
        const ValueType VB(234);
        const ValueType VC(345);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 1. Create an object x1 (init. to VA)."
                             "\t\t{ x1:VA }" << endl;
        Obj mX1(VA);  const Obj& X1 = mX1;
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check initial state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VA == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: x1 <op> x1."
                              << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));


        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 2. Create an object x2 (copy of x1)."
                             "\t\t{ x1:VA x2:VA }" << endl;
        Obj mX2(X1);  const Obj& X2 = mX2;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check initial state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VA == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2." << endl;
        ASSERT(1 == (X2 == X1));        ASSERT(0 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));


        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 3. Create an object x3 (default ctor)."
                             "\t\t{ x1:VA x2:VA x3:U }" << endl;
        Obj mX3;  const Obj& X3 = mX3;
        if (veryVerbose) { cout << '\t'; P(X3); }

        if (veryVerbose) cout << "\ta. Check initial state of x3." << endl;
        ASSERT(X3.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x3 <op> x1, x2, x3." << endl;
        ASSERT(0 == (X3 == X1));        ASSERT(1 == (X3 != X1));
        ASSERT(0 == (X3 == X2));        ASSERT(1 == (X3 != X2));
        ASSERT(1 == (X3 == X3));        ASSERT(0 == (X3 != X3));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 4. Create an object x4 (copy of x3)."
                             "\t\t{ x1:VA x2:VA x3:U x4:U }" << endl;
        Obj mX4(X3);  const Obj& X4 = mX4;
        if (veryVerbose) { cout << '\t'; P(X4); }

        if (veryVerbose) cout << "\ta. Check initial state of x4." << endl;
        ASSERT(X4.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x4 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X4 == X1));        ASSERT(1 == (X4 != X1));
        ASSERT(0 == (X4 == X2));        ASSERT(1 == (X4 != X2));
        ASSERT(1 == (X4 == X3));        ASSERT(0 == (X4 != X3));
        ASSERT(1 == (X4 == X4));        ASSERT(0 == (X4 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 5. Set x3 using '=' (set to VB)."
                             "\t\t{ x1:VA x2:VA x3:VB x4:U }" << endl;
        mX3.makeValue(VB);
        if (veryVerbose) { cout << '\t'; P(X3); }

        if (veryVerbose) cout << "\ta. Check new state of x3." << endl;
        ASSERT(!X3.isNull());
        ASSERT(VB == X3.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x3 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X3 == X1));        ASSERT(1 == (X3 != X1));
        ASSERT(0 == (X3 == X2));        ASSERT(1 == (X3 != X2));
        ASSERT(1 == (X3 == X3));        ASSERT(0 == (X3 != X3));
        ASSERT(0 == (X3 == X4));        ASSERT(1 == (X3 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 6. Change x1 using 'reset'."
                             "\t\t\t{ x1:U x2:VA x3:VB x4:U }" << endl;
        mX1.reset();
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(X1.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(1 == (X1 == X4));        ASSERT(0 == (X1 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 7. Change x1 ('makeValue', set to VC)."
                             "\t\t{ x1:VC x2:VA x3:VB x4:U }" << endl;
        mX1.makeValue(VC);
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VC == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(0 == (X1 == X4));        ASSERT(1 == (X1 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 8. Assign x2 = x1."
                             "\t\t\t\t{ x1:VC x2:VC x3:VB x4:U }" << endl;
        mX2 = X1;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check new state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VC == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X2 == X1));        ASSERT(0 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));
        ASSERT(0 == (X2 == X3));        ASSERT(1 == (X2 != X3));
        ASSERT(0 == (X2 == X4));        ASSERT(1 == (X2 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 9. Assign x2 = x3."
                             "\t\t\t\t{ x1:VC x2:VB x3:VB x4:U }" << endl;
        mX2 = X3;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check new state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VB == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X2 == X1));        ASSERT(1 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));
        ASSERT(1 == (X2 == X3));        ASSERT(0 == (X2 != X3));
        ASSERT(0 == (X2 == X4));        ASSERT(1 == (X2 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 10. Assign x1 = x1."
                             "\t\t\t\t{ x1:VC x2:VB x3:VB x4:U }" << endl;
        mX1 = X1;
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VC == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(0 == (X1 == X4));        ASSERT(1 == (X1 != X4));

        if (verbose) cout << "\nEnd of Breathing Test." << endl;
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST 1: Using 'bsl::string'
        //   This test exercises basic functionality, but tests nothing.
        //
        // Concerns:
        //   We want to demonstrate a base-line level of correct operation of
        //   the following methods and operators:
        //     - default and copy constructors.
        //     - the assignment operator (including aliasing).
        //     - equality operators: 'operator==' and 'operator!='.
        //     - primary manipulators: 'makeValue' and 'reset'.
        //     - basic accessors: 'value' and 'isSet'.
        //
        // Plan:
        //   Create four test objects using the default, initializing, and copy
        //   constructors.  Exercise the basic value-semantic methods and the
        //   equality operators using the test objects.  Invoke the primary
        //   manipulator [5, 6, 7], copy constructor [2, 4], assignment
        //   operator without [9, 9] and with [10] aliasing.  Use the basic
        //   accessors to verify the expected results.  Display object values
        //   frequently in verbose mode.  Note that 'VA', 'VB' and 'VC' denote
        //   unique, but otherwise arbitrary, object values, while 'U' denotes
        //   the valid, but "unknown", default object value.
        //
        //    1. Create an object x1 (init. to VA)  { x1:VA                  }
        //    2. Create an object x2 (copy of x1)   { x1:VA x2:VA            }
        //    3. Create an object x3 (default ctor) { x1:VA x2:VA x3:U       }
        //    4. Create an object x4 (copy of x3)   { x1:VA x2:VA x3:U  x4:U }
        //    5. Set x3 using 'makeValue' (set to VB){ x1:VA x2:VA x3:VB x4:U }
        //    6. Change x1 using 'reset'            { x1:U  x2:VA x3:VB x4:U }
        //    7. Change x1 ('makeValue', set to VC) { x1:VC x2:VA x3:VB x4:U }
        //    8. Assign x2 = x1                     { x1:VC x2:VC x3:VB x4:U }
        //    9. Assign x2 = x3                     { x1:VC x2:VB x3:VB x4:U }
        //   10. Assign x1 = x1 (aliasing)          { x1:VC x2:VB x3:VB x4:U }
        //
        // Testing:
        //   BREATHING TEST 1: Using 'bsl::string'
        // --------------------------------------------------------------------

        if (verbose) cout << "\nBREATHING TEST 1: Using 'bsl::string'"
                          << "\n=====================================" << endl;

        typedef bsl::string                    ValueType;
        typedef bdeut_NullableValue<ValueType> Obj;

        // possible values
        const ValueType VA("The");
        const ValueType VB("Breathing");
        const ValueType VC("Test");

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 1. Create an object x1 (init. to VA)."
                             "\t\t{ x1:VA }" << endl;
        Obj mX1(VA);  const Obj& X1 = mX1;
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check initial state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VA == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: x1 <op> x1."
                              << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));


        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 2. Create an object x2 (copy of x1)."
                             "\t\t{ x1:VA x2:VA }" << endl;
        Obj mX2(X1);  const Obj& X2 = mX2;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check initial state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VA == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2." << endl;
        ASSERT(1 == (X2 == X1));        ASSERT(0 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));


        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 3. Create an object x3 (default ctor)."
                             "\t\t{ x1:VA x2:VA x3:U }" << endl;
        Obj mX3;  const Obj& X3 = mX3;
        if (veryVerbose) { cout << '\t'; P(X3); }

        if (veryVerbose) cout << "\ta. Check initial state of x3." << endl;
        ASSERT(X3.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x3 <op> x1, x2, x3." << endl;
        ASSERT(0 == (X3 == X1));        ASSERT(1 == (X3 != X1));
        ASSERT(0 == (X3 == X2));        ASSERT(1 == (X3 != X2));
        ASSERT(1 == (X3 == X3));        ASSERT(0 == (X3 != X3));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 4. Create an object x4 (copy of x3)."
                             "\t\t{ x1:VA x2:VA x3:U x4:U }" << endl;
        Obj mX4(X3);  const Obj& X4 = mX4;
        if (veryVerbose) { cout << '\t'; P(X4); }

        if (veryVerbose) cout << "\ta. Check initial state of x4." << endl;
        ASSERT(X4.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x4 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X4 == X1));        ASSERT(1 == (X4 != X1));
        ASSERT(0 == (X4 == X2));        ASSERT(1 == (X4 != X2));
        ASSERT(1 == (X4 == X3));        ASSERT(0 == (X4 != X3));
        ASSERT(1 == (X4 == X4));        ASSERT(0 == (X4 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 5. Set x3 using '=' (set to VB)."
                             "\t\t{ x1:VA x2:VA x3:VB x4:U }" << endl;
        mX3.makeValue(VB);
        if (veryVerbose) { cout << '\t'; P(X3); }

        if (veryVerbose) cout << "\ta. Check new state of x3." << endl;
        ASSERT(!X3.isNull());
        ASSERT(VB == X3.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x3 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X3 == X1));        ASSERT(1 == (X3 != X1));
        ASSERT(0 == (X3 == X2));        ASSERT(1 == (X3 != X2));
        ASSERT(1 == (X3 == X3));        ASSERT(0 == (X3 != X3));
        ASSERT(0 == (X3 == X4));        ASSERT(1 == (X3 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 6. Change x1 using 'reset'."
                             "\t\t\t{ x1:U x2:VA x3:VB x4:U }" << endl;
        mX1.reset();
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(X1.isNull());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(1 == (X1 == X4));        ASSERT(0 == (X1 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 7. Change x1 ('makeValue', set to VC)."
                             "\t\t{ x1:VC x2:VA x3:VB x4:U }" << endl;
        mX1.makeValue(VC);
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VC == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(0 == (X1 == X4));        ASSERT(1 == (X1 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 8. Assign x2 = x1."
                             "\t\t\t\t{ x1:VC x2:VC x3:VB x4:U }" << endl;
        mX2 = X1;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check new state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VC == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X2 == X1));        ASSERT(0 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));
        ASSERT(0 == (X2 == X3));        ASSERT(1 == (X2 != X3));
        ASSERT(0 == (X2 == X4));        ASSERT(1 == (X2 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 9. Assign x2 = x3."
                             "\t\t\t\t{ x1:VC x2:VB x3:VB x4:U }" << endl;
        mX2 = X3;
        if (veryVerbose) { cout << '\t'; P(X2); }

        if (veryVerbose) cout << "\ta. Check new state of x2." << endl;
        ASSERT(!X2.isNull());
        ASSERT(VB == X2.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x2 <op> x1, x2, x3, x4." << endl;
        ASSERT(0 == (X2 == X1));        ASSERT(1 == (X2 != X1));
        ASSERT(1 == (X2 == X2));        ASSERT(0 == (X2 != X2));
        ASSERT(1 == (X2 == X3));        ASSERT(0 == (X2 != X3));
        ASSERT(0 == (X2 == X4));        ASSERT(1 == (X2 != X4));

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (verbose) cout << "\n 10. Assign x1 = x1."
                             "\t\t\t\t{ x1:VC x2:VB x3:VB x4:U }" << endl;
        mX1 = X1;
        if (veryVerbose) { cout << '\t'; P(X1); }

        if (veryVerbose) cout << "\ta. Check new state of x1." << endl;
        ASSERT(!X1.isNull());
        ASSERT(VC == X1.value());

        if (veryVerbose) cout << "\tb. Try equality operators: "
                                 "x1 <op> x1, x2, x3, x4." << endl;
        ASSERT(1 == (X1 == X1));        ASSERT(0 == (X1 != X1));
        ASSERT(0 == (X1 == X2));        ASSERT(1 == (X1 != X2));
        ASSERT(0 == (X1 == X3));        ASSERT(1 == (X1 != X3));
        ASSERT(0 == (X1 == X4));        ASSERT(1 == (X1 != X4));

        if (verbose) cout << "\nEnd of Breathing Test." << endl;
      } break;

      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    ASSERT(0 == globalAllocator.numBlocksTotal());

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }

    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2004
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
