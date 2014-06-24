// bslstl_sharedptrallocateinplacerep.t.cpp                           -*-C++-*-
#include <bslstl_sharedptrallocateinplacerep.h>

#include <bslstl_allocator.h>
#include <bslstl_allocatortraits.h>

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_testallocator.h>

#include <bsls_asserttest.h>
#include <bsls_bsltestutil.h>
#include <bsls_types.h>

#include <bsltf_stdstatefulallocator.h>

#include <stdio.h>
#include <stdlib.h>             // 'atoi'

#include <new>

#ifdef BSLS_PLATFORM_CMP_MSVC  // Microsoft Compiler
#ifdef _MSC_EXTENSIONS         // Microsoft Extensions Enabled
#include <new>                 // if so, need to include new as well
#endif
#endif

using namespace BloombergLP;

//=============================================================================
//                                  TEST PLAN
//-----------------------------------------------------------------------------
//                                  Overview
//                                  --------
// This test driver tests the functionality of the in-place shared pointer
// representation.
//-----------------------------------------------------------------------------
// bslstl::SharedPtrAllocateInplaceRep
//------------------------
// [ 2] SharedPtrAllocateInplaceRep<T> makeRep(Allocator basicAllocator);
// [ 2] TYPE *ptr();
// [ 2] void disposeRep();
// [ 4] void disposeObject();
// [  ] void *getDeleter(const std::type_info& type);
// [ 2] void *originalPtr() const;
// [ 5] void releaseRef();
// [ 5] void releaseWeakRef();
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 6] USAGE EXAMPLE
//-----------------------------------------------------------------------------

//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
int testStatus = 0;

namespace {

void aSsErT(bool b, const char *s, int i)
{
    if (b) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", i, s);
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

}  // close unnamed namespace

//=============================================================================
//                      STANDARD BDE TEST DRIVER MACROS
//-----------------------------------------------------------------------------

#define ASSERT       BSLS_BSLTESTUTIL_ASSERT
#define LOOP_ASSERT  BSLS_BSLTESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT
#define ASSERTV      BSLS_BSLTESTUTIL_ASSERTV

#define Q   BSLS_BSLTESTUTIL_Q   // Quote identifier literally.
#define P   BSLS_BSLTESTUTIL_P   // Print identifier and value.
#define P_  BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_  BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_  BSLS_BSLTESTUTIL_L_  // current Line number

// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

#define ASSERT_SAFE_PASS_RAW(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS_RAW(EXPR)
#define ASSERT_SAFE_FAIL_RAW(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL_RAW(EXPR)
#define ASSERT_PASS_RAW(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS_RAW(EXPR)
#define ASSERT_FAIL_RAW(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL_RAW(EXPR)
#define ASSERT_OPT_PASS_RAW(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS_RAW(EXPR)
#define ASSERT_OPT_FAIL_RAW(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL_RAW(EXPR)

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

// 'MyTestObject' CLASS HIERARCHY (defined below)
class MyTestObject;

// OTHER TEST OBJECTS (defined below)
class MyInplaceTestObject;

// TYPEDEFS
typedef bslstl::SharedPtrAllocateInplaceRep<MyTestObject,
                                            bsl::allocator<MyTestObject> > Obj;

typedef bslstl::SharedPtrAllocateInplaceRep<MyInplaceTestObject,
                                            bsl::allocator<MyInplaceTestObject>
                                           > TCObj; // For testing constructors

typedef MyTestObject TObj;

//=============================================================================
//              GLOBAL HELPER CLASSES AND FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

                             // ==================
                             // class MyTestObject
                             // ==================

class MyTestObject {
    // This class provides a test object that keeps track of how many instances
    // have been deleted.  Optionally, also keeps track of how many instances
    // have been copied.

    // DATA
    volatile int *d_deleteCounter_p;
    volatile int *d_copyCounter_p;

  public:
    // CREATORS
    MyTestObject(const MyTestObject& orig);
    explicit MyTestObject(int *deleteCounter, int *copyCounter = 0);
    virtual ~MyTestObject();

    // ACCESSORS
    volatile int *copyCounter() const;
    volatile int *deleteCounter() const;
};

                             // ------------------
                             // class MyTestObject
                             // ------------------

// CREATORS
MyTestObject::MyTestObject(const MyTestObject& orig)
: d_deleteCounter_p(orig.d_deleteCounter_p)
, d_copyCounter_p(orig.d_copyCounter_p)
{
    if (d_copyCounter_p) ++(*d_copyCounter_p);
}

MyTestObject::MyTestObject(int *deleteCounter, int *copyCounter)
: d_deleteCounter_p(deleteCounter)
, d_copyCounter_p(copyCounter)
{
}

MyTestObject::~MyTestObject()
{
    ++(*d_deleteCounter_p);
}

// ACCESSORS
volatile int* MyTestObject::deleteCounter() const
{
    return d_deleteCounter_p;
}

volatile int* MyTestObject::copyCounter() const
{
    return d_copyCounter_p;
}

                         // =========================
                         // class MyInplaceTestObject
                         // =========================

template <int N>
class MyTestArg {
    // This class template declares a separate type for each paramaterizing 'N'
    // that wraps an integer value and provides implicit conversion to and from
    // 'int'.  Its main purpose is that having separate types allows to
    // distinguish them in function interface, thereby avoiding ambiguities or
    // accidental switching of arguments in the implementation of in-place
    // constructors.

    // DATA
    int d_value;

  public:
    // CREATORS
    explicit MyTestArg(int value = -1) : d_value(value) {}

    // MANIPULATORS
    operator int&()      { return d_value; }

    // ACCESSORS
    operator int() const { return d_value; }
};

typedef MyTestArg< 1> MyTestArg1;
typedef MyTestArg< 2> MyTestArg2;
typedef MyTestArg< 3> MyTestArg3;
typedef MyTestArg< 4> MyTestArg4;
typedef MyTestArg< 5> MyTestArg5;
typedef MyTestArg< 6> MyTestArg6;
typedef MyTestArg< 7> MyTestArg7;
typedef MyTestArg< 8> MyTestArg8;
typedef MyTestArg< 9> MyTestArg9;
typedef MyTestArg<10> MyTestArg10;
typedef MyTestArg<11> MyTestArg11;
typedef MyTestArg<12> MyTestArg12;
typedef MyTestArg<13> MyTestArg13;
typedef MyTestArg<14> MyTestArg14;
    // Define fourteen test argument types 'MyTestArg1..14' to be used with the
    // in-place constructors of 'MyInplaceTestObject'.

class MyInplaceTestObject {
    // This class provides a test object used to check that the arguments
    // passed for creating a shared pointer with an in-place representation are
    // of the correct types and values.

    // DATA
    MyTestArg1  d_a1;
    MyTestArg2  d_a2;
    MyTestArg3  d_a3;
    MyTestArg4  d_a4;
    MyTestArg5  d_a5;
    MyTestArg6  d_a6;
    MyTestArg7  d_a7;
    MyTestArg8  d_a8;
    MyTestArg9  d_a9;
    MyTestArg10 d_a10;
    MyTestArg11 d_a11;
    MyTestArg12 d_a12;
    MyTestArg13 d_a13;
    MyTestArg14 d_a14;
    static int  s_numDeletes;

  public:
    // CREATORS
    MyInplaceTestObject() {}

    explicit MyInplaceTestObject(MyTestArg1 a1) : d_a1(a1) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2)
        : d_a1(a1), d_a2(a2) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3)
        : d_a1(a1), d_a2(a2), d_a3(a3) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5, MyTestArg6 a6)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5, MyTestArg6 a6,
                        MyTestArg7 a7)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5, MyTestArg6 a6,
                        MyTestArg7 a7, MyTestArg8 a8)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5, MyTestArg6 a6,
                        MyTestArg7 a7, MyTestArg8 a8,
                        MyTestArg9 a9)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9) {}

    MyInplaceTestObject(MyTestArg1 a1, MyTestArg2 a2,
                        MyTestArg3 a3, MyTestArg4 a4,
                        MyTestArg5 a5, MyTestArg6 a6,
                        MyTestArg7 a7, MyTestArg8 a8,
                        MyTestArg9 a9, MyTestArg10 a10)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9), d_a10(a10) {}

    MyInplaceTestObject(MyTestArg1  a1, MyTestArg2  a2,
                        MyTestArg3  a3, MyTestArg4  a4,
                        MyTestArg5  a5, MyTestArg6  a6,
                        MyTestArg7  a7, MyTestArg8  a8,
                        MyTestArg9  a9, MyTestArg10 a10,
                        MyTestArg11 a11)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9), d_a10(a10)
        , d_a11(a11) {}

    MyInplaceTestObject(MyTestArg1  a1,  MyTestArg2  a2,
                        MyTestArg3  a3,  MyTestArg4  a4,
                        MyTestArg5  a5,  MyTestArg6  a6,
                        MyTestArg7  a7,  MyTestArg8  a8,
                        MyTestArg9  a9,  MyTestArg10 a10,
                        MyTestArg11 a11, MyTestArg12 a12)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9), d_a10(a10)
        , d_a11(a11), d_a12(a12) {}

    MyInplaceTestObject(MyTestArg1  a1,  MyTestArg2  a2,
                        MyTestArg3  a3,  MyTestArg4  a4,
                        MyTestArg5  a5,  MyTestArg6  a6,
                        MyTestArg7  a7,  MyTestArg8  a8,
                        MyTestArg9  a9,  MyTestArg10 a10,
                        MyTestArg11 a11, MyTestArg12 a12,
                        MyTestArg13 a13)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9), d_a10(a10)
        , d_a11(a11), d_a12(a12), d_a13(a13) {}

    MyInplaceTestObject(MyTestArg1  a1,  MyTestArg2  a2,
                        MyTestArg3  a3,  MyTestArg4  a4,
                        MyTestArg5  a5,  MyTestArg6  a6,
                        MyTestArg7  a7,  MyTestArg8  a8,
                        MyTestArg9  a9,  MyTestArg10 a10,
                        MyTestArg11 a11, MyTestArg12 a12,
                        MyTestArg13 a13, MyTestArg14 a14)
        : d_a1(a1), d_a2(a2), d_a3(a3), d_a4(a4), d_a5(a5)
        , d_a6(a6), d_a7(a7), d_a8(a8), d_a9(a9), d_a10(a10)
        , d_a11(a11), d_a12(a12), d_a13(a13), d_a14(a14) {}
    ~MyInplaceTestObject() { ++s_numDeletes; };

    // ACCESSORS
    bool operator == (MyInplaceTestObject const& rhs) const
    {
        return d_a1  == rhs.d_a1  &&
               d_a1  == rhs.d_a1  &&
               d_a2  == rhs.d_a2  &&
               d_a3  == rhs.d_a3  &&
               d_a4  == rhs.d_a4  &&
               d_a5  == rhs.d_a5  &&
               d_a6  == rhs.d_a6  &&
               d_a7  == rhs.d_a7  &&
               d_a8  == rhs.d_a8  &&
               d_a9  == rhs.d_a9  &&
               d_a10 == rhs.d_a10 &&
               d_a11 == rhs.d_a11 &&
               d_a12 == rhs.d_a12 &&
               d_a13 == rhs.d_a13 &&
               d_a14 == rhs.d_a14;
    }

    int getNumDeletes() { return s_numDeletes; }
};

int MyInplaceTestObject::s_numDeletes = 0;

#if 0  // TBD Need an appropriately levelized usage example
                              // ================
                              // MySharedDatetime
                              // ================

class MySharedDatetime {
    // This class provide a reference counted smart pointer to support shared
    // ownership of a 'bdet_Datetime' object.

  private:
    bdet_Datetime      *d_ptr_p;  // pointer to the managed object
    bslma::SharedPtrRep *d_rep_p;  // pointer to the representation object

  private:
    // NOT IMPLEMENTED
    MySharedDatetime& operator=(const MySharedDatetime&);

  public:
    // CREATORS
    MySharedDatetime();
        // Create an empty shared datetime.

    MySharedDatetime(bdet_Datetime* ptr, bslma::SharedPtrRep* rep);
        // Create a shared datetime that adopts ownership of the specified
        // 'ptr' and the specified 'rep.

    MySharedDatetime(const MySharedDatetime& original);
        // Create a shared datetime that refers to the same object managed by
        // the specified 'original'

    ~MySharedDatetime();
        // Destroy this shared datetime and release the reference any object it
        // might be referring to.  If this is the last shared reference,
        // deleted the managed object.

    // MANIPULATORS
    void createInplace(bslma::Allocator *allocator,
                       int               year,
                       int               month,
                       int               day);
        // Create a new 'MySharedDatetimeRepImpl', using the specified
        // 'allocator' to supply memory, using the specified 'year', 'month'
        // and 'day' to initialize the 'bdet_Datetime' within the newly created
        // 'MySharedDatetimeRepImpl', and make this 'MySharedDatetime' refer to
        // the 'bdet_Datetime'.

    bdet_Datetime& operator*() const;
        // Return a modifiable reference to the shared 'bdet_Datetime' object.

    bdet_Datetime *operator->() const;
        // Return the address of the modifiable 'bdet_Datetime' to which this
        // object refers.

    bdet_Datetime *ptr() const;
        // Return the address of the modifiable 'bdet_Datetime' to which this
        // object refers.
};

                              // ----------------
                              // MySharedDatetime
                              // ----------------

MySharedDatetime::MySharedDatetime()
: d_ptr_p(0)
, d_rep_p(0)
{
}

MySharedDatetime::MySharedDatetime(bdet_Datetime      *ptr,
                                   bslma::SharedPtrRep *rep)
: d_ptr_p(ptr)
, d_rep_p(rep)
{
}

MySharedDatetime::MySharedDatetime(const MySharedDatetime& original)
: d_ptr_p(original.d_ptr_p)
, d_rep_p(original.d_rep_p)
{
    if (d_ptr_p) {
        d_rep_p->acquireRef();
    } else {
        d_rep_p = 0;
    }
}

MySharedDatetime::~MySharedDatetime()
{
    if (d_rep_p) {
        d_rep_p->releaseRef();
    }
}

void MySharedDatetime::createInplace(bslma::Allocator *allocator,
                                     int               year,
                                     int               month,
                                     int               day)
{
    allocator = bslma::Default::allocator(allocator);
    bslstl::SharedPtrAllocateInplaceRep<bdet_Datetime> *rep = new (*allocator)
                        bslstl::SharedPtrAllocateInplaceRep<bdet_Datetime>(allocator,
                                                                 year,
                                                                 month,
                                                                 day);
    MySharedDatetime temp(rep->ptr(), rep);
    bsl::swap(d_ptr_p, temp.d_ptr_p);
    bsl::swap(d_rep_p, temp.d_rep_p);
}

bdet_Datetime& MySharedDatetime::operator*() const {
    return *d_ptr_p;
}

bdet_Datetime *MySharedDatetime::operator->() const {
    return d_ptr_p;
}

bdet_Datetime *MySharedDatetime::ptr() const {
    return d_ptr_p;
}
#endif
template <class ALLOCATOR>
struct TestHarness {
    static void testCase2(bool verbose,
                          bool veryVerbose,
                          bool veryVeryVerbose,
                          bool veryVeryVeryVerbose);

    static void testCase3(bool verbose,
                          bool veryVerbose,
                          bool veryVeryVerbose,
                          bool veryVeryVeryVerbose);

    static void testCase4(bool verbose,
                          bool veryVerbose,
                          bool veryVeryVerbose,
                          bool veryVeryVeryVerbose);

    static void testCase5(bool verbose,
                          bool veryVerbose,
                          bool veryVeryVerbose,
                          bool veryVeryVeryVerbose);
};

template <class ALLOCATOR>
void TestHarness<ALLOCATOR>::testCase2(bool verbose,
                                       bool veryVerbose,
                                       bool veryVeryVerbose,
                                       bool veryVeryVeryVerbose)
{
    // --------------------------------------------------------------------
    // TESTING BASIC CONSTRUCTOR
    //
    // Concerns:
    //   Verify that upon construction the object is properly initialized,
    //
    // Plan:
    //   Construct object using basic constructor and verify that that
    //   accessors return the expected values.
    //
    // Testing:
    //   bslstl::SharedPtrAllocateInplaceRep(bslma::Allocator *basicAllocator);
    //   TYPE *ptr();
    //   void disposeRep();
    //   void *originalPtr() const;
    // --------------------------------------------------------------------

    if (verbose) printf("\nTesting Constructor"
                        "\n-------------------\n");

    (void)veryVerbose;
    (void)veryVeryVerbose;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                          template rebind_traits<MyTestObject> Obj_AllocTraits;
    typedef typename Obj_AllocTraits::allocator_type Obj_Alloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyTestObject, Obj_Alloc> Obj;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                 template rebind_traits<MyInplaceTestObject> TCObj_AllocTraits;
    typedef typename TCObj_AllocTraits::allocator_type TCObj_ElementAlloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyInplaceTestObject,
                                                    TCObj_ElementAlloc> TCObj;

    typedef typename TCObj::ReboundAllocator TCObj_Alloc;

    bslma::TestAllocator ta("Tese case 2", veryVeryVeryVerbose);
    ALLOCATOR alloc_base(&ta);

    Obj_Alloc alloc1(alloc_base);
    TCObj_Alloc alloc2(&ta);

    bsls::Types::Int64 numAllocations = ta.numAllocations();
    bsls::Types::Int64 numDeallocations = ta.numDeallocations();
    {
        static const MyInplaceTestObject EXP;

        // Dynamically allocate object as the destructor is declared as
        // private.

        TCObj *xPtr = TCObj::makeRep(alloc_base);

//        TCObj* xPtr = new(ta) TCObj(&ta);
        TCObj& x = *xPtr;
        const TCObj& X = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());
//        ASSERT(EXP == *(x.ptr()));
        ASSERT(x.originalPtr() == static_cast<void*>(x.ptr()));

        // Manually deallocate the representation using 'disposeRep'.

        x.disposeRep();

        ASSERT(++numDeallocations == ta.numDeallocations());
    }

}

template <class ALLOCATOR>
void TestHarness<ALLOCATOR>::testCase3(bool verbose,
                                       bool veryVerbose,
                                       bool veryVeryVerbose,
                                       bool veryVeryVeryVerbose)
{
    // --------------------------------------------------------------------
    // TESTING CONSTRUCTOR
    //
    // Concerns:
    //   All constructor is able to initialize the object correctly.
    //
    // Plan:
    //   Call all 14 different constructors and supply it with the
    //   appropriate arguments.  Then verify that the object created inside
    //   the representation is initialized using the arguments supplied.
    //
    // Testing:
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a2);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a3);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a4);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a5);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a6);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a7);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a8);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a9);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a10);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a11);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a12);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a13);
    //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a14);
    // --------------------------------------------------------------------
    if (verbose) printf("\nTesting constructor\n"
                        "\n===================\n");

    (void)veryVerbose;
    (void)veryVeryVerbose;
    (void)veryVeryVeryVerbose;

    // TBD rewrite this test case
    if (verbose) printf("\nTBD\n");

#if 0
    typedef typename bsl::allocator_traits<ALLOCATOR>::
                          template rebind_traits<MyTestObject> Obj_AllocTraits;
    typedef typename Obj_AllocTraits::allocator_type Obj_Alloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyTestObject, Obj_Alloc>
                                                                           Obj;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                 template rebind_traits<MyInplaceTestObject> TCObj_AllocTraits;
    typedef typename TCObj_AllocTraits::allocator_type TCObj_ElementAlloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyInplaceTestObject,
                                                    TCObj_ElementAlloc> TCObj;

    typedef typename TCObj::ReboundAllocator TCObj_Alloc;

    static const MyTestArg1 V1(1);
    static const MyTestArg2 V2(20);
    static const MyTestArg3 V3(23);
    static const MyTestArg4 V4(44);
    static const MyTestArg5 V5(66);
    static const MyTestArg6 V6(176);
    static const MyTestArg7 V7(878);
    static const MyTestArg8 V8(8);
    static const MyTestArg9 V9(912);
    static const MyTestArg10 V10(102);
    static const MyTestArg11 V11(111);
    static const MyTestArg12 V12(333);
    static const MyTestArg13 V13(712);
    static const MyTestArg14 V14(1414);

    bslma::TestAllocator ta("Tese case 3", veryVeryVeryVerbose);
    ALLOCATOR alloc_base(&ta);

    Obj_Alloc alloc1(alloc_base);
    TCObj_Alloc alloc2(&ta);

    if (verbose) printf("\nTesting constructor with no arguments"
                        "\n-------------------------------------\n");

    bsls::Types::Int64 numAllocations = ta.numAllocations();
    bsls::Types::Int64 numDeallocations = ta.numDeallocations();
    {
        static const MyInplaceTestObject EXP = MyInplaceTestObject();
        TCObj* xPtr = alloc2.allocate(1);
        bsl::allocator_traits<TCObj_Alloc>::construct(alloc2, xPtr, alloc2);
        TCObj& x = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());
        ASSERT(EXP == *(x.ptr()));
        x.disposeRep();
        ASSERT(++numDeallocations == ta.numDeallocations());
    }


    if (verbose) printf("\nTesting constructor with 14 arguments"
                        "\n-------------------------------------\n");

    numAllocations = ta.numAllocations();
    numDeallocations = ta.numDeallocations();
    {
        static const MyInplaceTestObject EXP(V1, V2, V3, V4, V5, V6, V7,
                                              V8, V9, V10);
        TCObj* xPtr = new(ta) TCObj(alloc2, V1, V2, V3, V4, V5, V6, V7, V8,
                                                  V9, V104);
        TCObj& x = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());
        ASSERT(EXP == *(x.ptr()));
        x.disposeRep();
        ASSERT(++numDeallocations == ta.numDeallocations());
    }
#endif
}

template <class ALLOCATOR>
void TestHarness<ALLOCATOR>::testCase4(bool verbose,
                                       bool veryVerbose,
                                       bool veryVeryVerbose,
                                       bool veryVeryVeryVerbose)
{
    // --------------------------------------------------------------------
    // TESTING 'disposeObject'
    //
    // Concerns:
    //   The destructor of the object is called when 'disposeObject' is
    //   called.
    //
    // Plan:
    //   Call 'disposeObject' and verify that the destructor is called.
    //
    // Testing:
    //   void disposeObject();
    //
    // --------------------------------------------------------------------
    if (verbose) printf("\nTesting disposeObject"
                        "\n---------------------\n");

    (void)veryVerbose;
    (void)veryVeryVerbose;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                          template rebind_traits<MyTestObject> Obj_AllocTraits;
    typedef typename Obj_AllocTraits::allocator_type Obj_Alloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyTestObject, Obj_Alloc> Obj;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                 template rebind_traits<MyInplaceTestObject> TCObj_AllocTraits;
    typedef typename TCObj_AllocTraits::allocator_type TCObj_ElementAlloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyInplaceTestObject,
                                                    TCObj_ElementAlloc> TCObj;

    typedef typename TCObj::ReboundAllocator TCObj_Alloc;

    bslma::TestAllocator ta("Tese case 4", veryVeryVeryVerbose);
    ALLOCATOR alloc_base(&ta);

    Obj_Alloc alloc1(alloc_base);
    TCObj_Alloc alloc2(&ta);

    bsls::Types::Int64 numAllocations = ta.numAllocations();
    bsls::Types::Int64 numDeallocations = ta.numDeallocations();
    {
        int numDeletes = 0;
        Obj *xPtr = Obj::makeRep(alloc_base);
//        Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
        Obj& x = *xPtr;
        const Obj& X = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());
        ASSERT(0 == numDeletes);

        new(x.ptr()) MyTestObject(&numDeletes);

        x.disposeObject();
        ASSERT(1 == numDeletes);

        x.disposeRep();
        ASSERT(++numDeallocations == ta.numDeallocations());
    }
}

template <class ALLOCATOR>
void TestHarness<ALLOCATOR>::testCase5(bool verbose,
                                       bool veryVerbose,
                                       bool veryVeryVerbose,
                                       bool veryVeryVeryVerbose)
{
    // --------------------------------------------------------------------
    // TESTING 'releaseRef' and 'releaseWeakRef'
    //
    // Concerns:
    //   1) 'releaseRef' and 'releaseWeakRef' is decrementing the reference
    //      count correctly.
    //   2) disposeObject() is called when there is no shared reference.
    //   3) disposeRep() is called only when there is no shared reference
    //      and no weak reference.
    //
    // Plan:
    //   1) Call 'acquireRef' then 'releaseRef' and verify 'numReference'
    //      did not change.  Call 'acquireWeakRef' then 'releaseWeakRef'
    //      and verify 'numWeakReference' did not change.
    //   2) Call 'releaseRef' when there is only one reference remaining.
    //      Then verify that both 'disposeObject' and 'disposeRep' is
    //      called.
    //   3) Create another object and call 'acquireWeakRef' before calling
    //      'releaseRef'.  Verify that only 'disposeObject' is called.
    //      Then call 'releaseWeakRef' and verify that 'disposeRep' is
    //      called.
    //
    // Testing:
    //   void releaseRef();
    //   void releaseWeakRef();
    // --------------------------------------------------------------------
    if (verbose) printf("\nTesting 'releaseRef' and 'releaseWeakRef'"
                        "\n=========================================\n");

    (void)veryVerbose;
    (void)veryVeryVerbose;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                          template rebind_traits<MyTestObject> Obj_AllocTraits;
    typedef typename Obj_AllocTraits::allocator_type Obj_Alloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyTestObject, Obj_Alloc> Obj;

    typedef typename bsl::allocator_traits<ALLOCATOR>::
                 template rebind_traits<MyInplaceTestObject> TCObj_AllocTraits;
    typedef typename TCObj_AllocTraits::allocator_type TCObj_ElementAlloc;

    typedef bslstl::SharedPtrAllocateInplaceRep<MyInplaceTestObject,
                                                    TCObj_ElementAlloc> TCObj;

    typedef typename TCObj::ReboundAllocator TCObj_Alloc;

    bslma::TestAllocator ta("Tese case 5", veryVeryVeryVerbose);
    ALLOCATOR alloc_base(&ta);

    Obj_Alloc alloc1(alloc_base);
    TCObj_Alloc alloc2(&ta);

    bsls::Types::Int64 numAllocations = ta.numAllocations();
    bsls::Types::Int64 numDeallocations = ta.numDeallocations();
    {
        int numDeletes = 0;
        Obj *xPtr = Obj::makeRep(alloc_base);
//        Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
        Obj& x = *xPtr;
        const Obj& X = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());

        new(x.ptr()) MyTestObject(&numDeletes);

        x.acquireRef();
        x.releaseRef();

        ASSERT(1 == X.numReferences());
        ASSERT(0 == X.numWeakReferences());
        ASSERT(true == X.hasUniqueOwner());

        x.acquireWeakRef();
        x.releaseWeakRef();

        ASSERT(1 == X.numReferences());
        ASSERT(0 == X.numWeakReferences());
        ASSERT(true == X.hasUniqueOwner());

        if (verbose) printf(
                           "\nTesting 'releaseRef' with no weak reference'"
                           "\n--------------------------------------------\n");

        x.releaseRef();

        ASSERT(1 == numDeletes);
        ASSERT(++numDeallocations == ta.numDeallocations());
    }

    if (verbose) printf("\nTesting 'releaseRef' with weak reference'"
                        "\n-----------------------------------------\n");
    {
        int numDeletes = 0;
        Obj *xPtr = Obj::makeRep(alloc_base);
//        Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
        Obj& x = *xPtr;
        const Obj& X = *xPtr;

        ASSERT(++numAllocations == ta.numAllocations());

        new(x.ptr()) MyTestObject(&numDeletes);

        x.acquireWeakRef();
        x.releaseRef();

        ASSERT(0 == X.numReferences());
        ASSERT(1 == X.numWeakReferences());
        ASSERT(false == X.hasUniqueOwner());
        ASSERT(1 == numDeletes);
        ASSERT(numDeallocations == ta.numDeallocations());

        x.releaseWeakRef();
        ASSERT(++numDeallocations == ta.numDeallocations());
    }
}
//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int                test = argc > 1 ? atoi(argv[1]) : 0;
    int             verbose = argc > 2;
    int         veryVerbose = argc > 3;
    int     veryVeryVerbose = argc > 4;
    int veryVeryVeryVerbose = argc > 5;

    bslma::TestAllocator globalAllocator("global", veryVeryVeryVerbose);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    // Confirm no static intialization locekd the global allocator
    ASSERT(&globalAllocator == bslma::Default::globalAllocator());

    bslma::TestAllocator defaultAllocator("default", veryVeryVeryVerbose);
    bslma::Default::setDefaultAllocator(&defaultAllocator);

    // Confirm no static intialization locked the default allocator
    ASSERT(&defaultAllocator == bslma::Default::defaultAllocator());

    bslma::TestAllocator ta(veryVeryVeryVerbose);
    bsls::Types::Int64 numDeallocations;
    bsls::Types::Int64 numAllocations;

    printf("TEST " __FILE__ " CASE %d\n", test);

    switch (test) { case 0:  // Zero is always the leading case.
#if 0  // TBD Need an appropriately levelized usage example
   case 6: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //
        // Concern:
        //   Usage example described in header doc compiles and run.
        //
        // Plan:
        //   Copy the usage example and strip the comments.  Then create simple
        //   test case to use the implementation described in doc.
        //
        // Testing:
        //   Usage example
        // --------------------------------------------------------------------
        if (verbose) printf(endl
                        << "Testing Usage Example\n"
                        << "=====================\n");
        {
            MySharedDatetime dt1;
            ASSERT(0 == ta.numAllocations());

            dt1.createInplace(&ta, 2011, 1, 1);

            ASSERT(1 == ta.numAllocations());

            MySharedDatetime dt2(dt1);

            ASSERT(dt2.ptr() == dt1.ptr());
            ASSERT(*dt2 == *dt1);

            dt2->addDays(31);
            ASSERT(2 == dt2->month());
            ASSERT(*dt1 == *dt2);
            ASSERT(0 == ta.numDeallocations());
        }
        ASSERT(1 == ta.numDeallocations());
      } break;
#endif
   case 5: {
        // --------------------------------------------------------------------
        // TESTING 'releaseRef' and 'releaseWeakRef'
        //
        // Concerns:
        //   1) 'releaseRef' and 'releaseWeakRef' is decrementing the reference
        //      count correctly.
        //   2) disposeObject() is called when there is no shared reference.
        //   3) disposeRep() is called only when there is no shared reference
        //      and no weak reference.
        //
        // Plan:
        //   1) Call 'acquireRef' then 'releaseRef' and verify 'numReference'
        //      did not change.  Call 'acquireWeakRef' then 'releaseWeakRef'
        //      and verify 'numWeakReference' did not change.
        //   2) Call 'releaseRef' when there is only one reference remaining.
        //      Then verify that both 'disposeObject' and 'disposeRep' is
        //      called.
        //   3) Create another object and call 'acquireWeakRef' before calling
        //      'releaseRef'.  Verify that only 'disposeObject' is called.
        //      Then call 'releaseWeakRef' and verify that 'disposeRep' is
        //      called.
        //
        // Testing:
        //   void releaseRef();
        //   void releaseWeakRef();
        // --------------------------------------------------------------------
        if (verbose) printf("\nTesting 'releaseRef' and 'releaseWeakRef'"
                            "\n=========================================\n");

#if 0
        numAllocations = ta.numAllocations();
        numDeallocations = ta.numDeallocations();
        {
            int numDeletes = 0;
            Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
            Obj& x = *xPtr;
            Obj const& X = *xPtr;

            ASSERT(++numAllocations == ta.numAllocations());

            x.acquireRef();
            x.releaseRef();

            ASSERT(1 == X.numReferences());
            ASSERT(0 == X.numWeakReferences());
            ASSERT(true == X.hasUniqueOwner());

            x.acquireWeakRef();
            x.releaseWeakRef();

            ASSERT(1 == X.numReferences());
            ASSERT(0 == X.numWeakReferences());
            ASSERT(true == X.hasUniqueOwner());

            if (verbose) printf(
                        "\nTesting 'releaseRef' with no weak reference'"
                        "\n--------------------------------------------\n");

            x.releaseRef();

            ASSERT(1 == numDeletes);
            ASSERT(++numDeallocations == ta.numDeallocations());
        }
        if (verbose) printf("\nTesting 'releaseRef' with weak reference'"
                            "\n-----------------------------------------\n");

        {
            int numDeletes = 0;
            Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
            Obj& x = *xPtr;
            Obj const& X = *xPtr;

            ASSERT(++numAllocations == ta.numAllocations());

            x.acquireWeakRef();
            x.releaseRef();

            ASSERT(0 == X.numReferences());
            ASSERT(1 == X.numWeakReferences());
            ASSERT(false == X.hasUniqueOwner());
            ASSERT(1 == numDeletes);
            ASSERT(numDeallocations == ta.numDeallocations());

            x.releaseWeakRef();
            ASSERT(++numDeallocations == ta.numDeallocations());
        }
#else
        using BloombergLP::bsltf::StdStatefulAllocator;

        typedef bsl::allocator<int> ALLOC_1;
        typedef StdStatefulAllocator<int, true, true, true, true> ALLOC_2;
        typedef StdStatefulAllocator<int, false, false, false, false> ALLOC_3;

        typedef TestHarness<ALLOC_1> T1;
        typedef TestHarness<ALLOC_2> T2;
        typedef TestHarness<ALLOC_3> T3;

        T1::testCase5(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T2::testCase5(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T3::testCase5(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
#endif
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING 'disposeObject'
        //
        // Concerns:
        //   The destructor of the object is called when 'disposeObject' is
        //   called.
        //
        // Plan:
        //   Call 'disposeObject' and verify that the destructor is called.
        //
        // Testing:
        //   void disposeObject();
        //
        // --------------------------------------------------------------------
        if (verbose) printf("\nTesting disposeObject"
                            "\n---------------------\n");

#if 0
        numAllocations = ta.numAllocations();
        numDeallocations = ta.numDeallocations();
        {
            int numDeletes = 0;
            Obj* xPtr = new(ta) Obj(&ta, &numDeletes);
            Obj& x = *xPtr;
            Obj const& X = *xPtr;

            ASSERT(++numAllocations == ta.numAllocations());
            ASSERT(0 == numDeletes);

            x.disposeObject();
            ASSERT(1 == numDeletes);

            x.disposeRep();
            ASSERT(++numDeallocations == ta.numDeallocations());
        }
#else
        using BloombergLP::bsltf::StdStatefulAllocator;

        typedef bsl::allocator<int> ALLOC_1;
        typedef StdStatefulAllocator<int, true, true, true, true> ALLOC_2;
        typedef StdStatefulAllocator<int, false, false, false, false> ALLOC_3;

        typedef TestHarness<ALLOC_1> T1;
        typedef TestHarness<ALLOC_2> T2;
        typedef TestHarness<ALLOC_3> T3;

        T1::testCase4(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T2::testCase4(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T3::testCase4(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
#endif
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING CONSTRUCTOR
        //
        // Concerns:
        //   All constructor is able to initialize the object correctly.
        //
        // Plan:
        //   Call all 14 different constructors and supply it with the
        //   appropriate arguments.  Then verify that the object created inside
        //   the representation is initialized using the arguments supplied.
        //
        // Testing:
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a1);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a2);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a3);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a4);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a5);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a6);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a7);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a8);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a9);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a10);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a11);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a12);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a13);
        //   bslstl::SharedPtrAllocateInplaceRep(const ALLOCATOR& a, ...a14);
        // --------------------------------------------------------------------

        using BloombergLP::bsltf::StdStatefulAllocator;

        typedef bsl::allocator<int> ALLOC_1;
        typedef StdStatefulAllocator<int, true, true, true, true> ALLOC_2;
        typedef StdStatefulAllocator<int, false, false, false, false> ALLOC_3;

        typedef TestHarness<ALLOC_1> T1;
        typedef TestHarness<ALLOC_2> T2;
        typedef TestHarness<ALLOC_3> T3;

        T1::testCase3(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T2::testCase3(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T3::testCase3(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING BASIC CONSTRUCTOR
        //
        // Concerns:
        //   Verify that upon construction the object is properly initialized,
        //
        // Plan:
        //   Construct object using basic constructor and verify that that
        //   accessors return the expected values.
        //
        // Testing:
        //   bslstl::SharedPtrAllocateInplaceRep(bslma::Allocator *basicAllocator);
        //   TYPE *ptr();
        //   void disposeRep();
        //   void *originalPtr() const;
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Constructor"
                            "\n-------------------\n");

#if 0
        numAllocations = ta.numAllocations();
        numDeallocations = ta.numDeallocations();
        {
            static const MyInplaceTestObject EXP;

            // Dynamically allocate object as the destructor is declared as
            // private.

            TCObj* xPtr = new(ta) TCObj(&ta);
            TCObj& x = *xPtr;
            TCObj const& X = *xPtr;

            ASSERT(++numAllocations == ta.numAllocations());
            ASSERT(EXP == *(x.ptr()));
            ASSERT(x.originalPtr() == static_cast<void*>(x.ptr()));

            // Manually deallocate the representation using 'disposeRep'.

            x.disposeRep();

            ASSERT(++numDeallocations == ta.numDeallocations());
        }
#else
        using BloombergLP::bsltf::StdStatefulAllocator;

        typedef bsl::allocator<int> ALLOC_1;
        typedef StdStatefulAllocator<int, true, true, true, true> ALLOC_2;
        typedef StdStatefulAllocator<int, false, false, false, false> ALLOC_3;

        typedef TestHarness<ALLOC_1> T1;
        typedef TestHarness<ALLOC_2> T2;
        typedef TestHarness<ALLOC_3> T3;

        T1::testCase2(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T2::testCase2(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
        T3::testCase2(verbose,
                      veryVerbose,
                      veryVeryVerbose,
                      veryVeryVeryVerbose);
#endif
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //
        // Testing:
        //   This test exercises basic functionality but tests nothing.
        // --------------------------------------------------------------------

        if (verbose) printf("\nBREATHING TEST\n"
                            "\n==============\n");

      } break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    // CONCERN: In no case does memory come from the global allocator.

    LOOP_ASSERT(globalAllocator.numBlocksTotal(),
                0 == globalAllocator.numBlocksTotal());

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
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
