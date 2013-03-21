// bdef_vfunc6.t.cpp                                                  -*-C++-*-

#include <bdef_vfunc6.h>

#include <bslma_default.h>
#include <bslma_testallocator.h>                // for testing only

#include <bsl_iostream.h>
#include <bsl_c_stdlib.h>     // atoi()

using namespace BloombergLP;
using namespace bsl;  // automatically added by script

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// The 'bdef_Vfunc6' class holds a pointer to an instance of a 'bdefr_Vfunc6'
// (an abstract class), and manipulates it appropriately.  Our main objective
// is to make sure that the 'bdefr_Vfunc6' object is initialized correctly and
// that its integer data (the reference count) is modified by the methods of
// 'bdef_Vfunc6' as expected.
//
// Note that places where test drivers in this family are likely to require
// adjustment are indicated by the tag: "ADJ".
//
// We use the following abbreviations:
//  'A1-N'    for 'A1, A2, ... , AN' 'c_A1'    for 'const A1' 'c_A1-N   for
//  'const A1, const A2, ... ,  const AN' 'c_A1-N&' for 'const A1&, const A2&,
//  ... , const AN&'
//
//-----------------------------------------------------------------------------
// [ 1] bdef_Vfunc6<A1-6>();
// [ 1] bdef_Vfunc6<A1-6>(bdefr_Vfunc6<A1-6> *);
// [ 1] bdef_Vfunc6<A1-6>(const bdef_Vfunc6<A1-6>&);
// [ 1] ~bdef_Vfunc6<A1-6>();
// [ 2] bdef_Vfunc6<A1-6>& operator=(const bdef_Vfunc6<A1-6>&);
// [ 1] void operator()(c_A1-6&) const;
// [ 1] operator const void *() const;
//-----------------------------------------------------------------------------
// [ 3] USAGE EXAMPLE

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

# define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number

//=============================================================================
//                  CONCRETE DERIVED CLASS FOR TESTING
//-----------------------------------------------------------------------------

template <class F, class A1, class A2, class A3, class A4, class A5, class A6,
          class D1, class D2, class D3>
class FunctorRep : public bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> {
    // This class defines the representation for a function object (functor),
    // characterized by a function-call operator taking six arguments and
    // returning 'void', that holds a pure procedure (i.e., free function,
    // static member function, or functor) taking three additional trailing
    // arguments, and the arguments' corresponding values.
    //
    // It mimics the behavior of classes in 'bdefi' with one exception: it
    // allows to count the number of time the 'execute' function has been
    // called, which is used to verify that different functors, that are
    // supposed to share the implementation (as a result of copy construction
    // or assignment) indeed share the implementation.

    F  d_f;  // function pointer or function object (functor)
    D1 d_d1; // first embedded argument
    D2 d_d2; // second embedded argument
    D3 d_d3; // third embedded argument

    int d_invocationCounter;    // invocation counter
    int *d_invocationCounter_p; // pointer to the invocation counter.  Since
                                // the counter must be incremented in the
                                // 'const' method 'execute', we increment it
                                // via this pointer.

  private:
    // not implemented
    FunctorRep(const FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>&);
    FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>&
        operator=(const FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>&);
  private:
    inline ~FunctorRep();
        // Destroy this functor.  This destructor can be called only through
        // the static 'deleteObject' method of the base class.

  public:
    // CREATORS
    inline FunctorRep(F                 procedure,
                      const D1&         embeddedArg1,
                      const D2&         embeddedArg2,
                      const D3&         embeddedArg3,
                      bslma::Allocator *basicAllocator);
        // Create a representation for a function object (functor) taking six
        // arguments and returning 'void', using the specified 'procedure'
        // (i.e., free function, static member function, or functor) taking
        // three additional trailing arguments, and the corresponding
        // arguments' specified 'embeddedArg1', 'embeddedArg2', etc., values.
        // Use the specified 'basicAllocator' to supply memory.  Note that any
        // value returned from 'procedure' will be ignored.

    // ACCESSORS
    inline void execute(const A1& argument1, const A2& argument2,
                        const A3& argument3, const A4& argument4,
                        const A5& argument5, const A6& argument6) const;
        // Invoke the underlying procedure (free function, static member
        // function, or functor) with the specified 'argument1-6' along with
        // the sequence of argument values specified at construction.
        // Increase the invocation counter.

    int invocationCounter() const;
        // Return the number of times the 'execute' method was invoked.
};

template <class F, class A1, class A2, class A3, class A4, class A5, class A6,
          class D1, class D2, class D3>
inline FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>::FunctorRep(
                                             F                 procedure,
                                             const D1&         embeddedArg1,
                                             const D2&         embeddedArg2,
                                             const D3&         embeddedArg3,
                                             bslma::Allocator *basicAllocator)
: bdefr_Vfunc6<A1, A2, A3, A4, A5, A6>(basicAllocator)
, d_f(procedure)
, d_d1(embeddedArg1)
, d_d2(embeddedArg2)
, d_d3(embeddedArg3)
, d_invocationCounter(0)
{
  d_invocationCounter_p = &d_invocationCounter;
}

template <class F, class A1, class A2, class A3, class A4, class A5, class A6,
          class D1, class D2, class D3>
inline
FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>::~FunctorRep()
{
}

template <class F, class A1, class A2, class A3, class A4, class A5, class A6,
          class D1, class D2, class D3> inline
void FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>::execute(
                                                   const A1& argument1,
                                                   const A2& argument2,
                                                   const A3& argument3,
                                                   const A4& argument4,
                                                   const A5& argument5,
                                                   const A6& argument6) const
{
    d_f(argument1, argument2, argument3, argument4, argument5, argument6,
        d_d1, d_d2, d_d3);
    ++*d_invocationCounter_p;
}

template <class F, class A1, class A2, class A3, class A4, class A5, class A6,
          class D1, class D2, class D3>
int
FunctorRep<F, A1, A2, A3, A4, A5, A6, D1, D2, D3>::invocationCounter() const
{
    return d_invocationCounter;
}

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

    typedef void (*parmMatchingFunc)(int *, int *, int * , int *,
                                     int *, int *, int * , int *, int *);
    typedef FunctorRep<parmMatchingFunc, int *, int *, int * , int *, int *,
                       int *, int * , int *, int *> ParmMatchingFunctorImp;
    typedef bdefr_Vfunc6<int *, int *, int *, int *, int *, int *> ObjRep;
    typedef bdef_Vfunc6<int *, int *, int *, int *, int *, int *> Obj;

    const int ARG1       = 1;
    const int ARG2       = 2;
    const int ARG3       = 3;
    const int ARG4       = 4;
    const int ARG5       = 5;
    const int ARG6       = 6;
    const int ARG7       = 7;
    const int ARG8       = 8;
    const int ARG9       = 9;

    const int ARG1_PLUS1 = ARG1 + 1;

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

static void checkParameterMatching(int *a1, int *a2, int *a3, int *a4,
                                   int *a5, int *a6, int *a7, int *a8,
                                   int *a9)
    // Invoke the global 'ASSERT' macro to verify that the specified arguments
    // 'a1-9' are equal the global values 'ARG1-9' respectively.  Then
    // increment the 'a1' by one.
    //
    // This function is used to test the correctness of parameter passing.  We
    // always call 'checkParameterMatching' as 'checkParameterMatching(ARG1,
    // ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);' If the parameters are
    // passed in the wrong order, the ASSERT statement will catch the bug.
    // The last statement incrementing '*a1' allows the caller to verify that
    // the function was actually called and that the 'ASSERT' statements were
    // in fact executed.

    {
        ASSERT(*a1 == ARG1);
        ASSERT(*a2 == ARG2);
        ASSERT(*a3 == ARG3);
        ASSERT(*a4 == ARG4);
        ASSERT(*a5 == ARG5);
        ASSERT(*a6 == ARG6);
        ASSERT(*a7 == ARG7);
        ASSERT(*a8 == ARG8);
        ASSERT(*a9 == ARG9);
        ++*a1;
    }

static int getCount(ObjRep *rep)
    // Return the current count of the specified 'object'.
{
    // Since we can retrieve the value of the count only by calling the
    // 'decrement' method, we will use a composition of the 'increment' and
    // 'decrement' functions to retrieve the value of 'd_count' yet still
    // preserve its current value.

    rep->increment();
    return rep->decrement();
}


//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    bslma::TestAllocator testAllocator(veryVeryVerbose);

    switch (test) { case 0:  // Zero is always the leading case.
      case 3: {
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

        if (verbose) cout << endl
                          << "Testing Usage Example" << endl
                          << "=====================" << endl;
        {
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING ASSIGNMENT OPERATOR
        //
        //   We need to test the following six cases:
        //    a. Assignment to uninitialized object from uninitialized object.
        //    b. Assignment to initialized object from uninitialized object.
        //    c. Assignment of uninitialized object to self.
        //    d. Assignment to uninitialized object from initialized object.
        //    e. Assignment to initialized object from initialized object.
        //    f. Assignment of initialized object to self.
        //
        //   We have the following specific concerns for each case:
        //    1. The assignment operator fully initializes the object
        //       whose function can be detected with 'operator const void *()
        //       const', and executed with 'void operator()'.
        //    2. A new object shares a 'bdefr_Vfunc6' object with its "right
        //       hand side (or both are null)."
        //    3. The reference counting works as expected.
        //
        // Plan:
        //   Create an object 'Obj1' using the default constructor.
        //    1a. Create an object 'Obj2' using the default constructor.
        //        Execute the assignment operator using 'Obj1' as the
        //        right-hand-side.  Execute 'operator const void *() const'.
        //    2a. N/A.
        //    3a. N/A.
        //
        //   Create an object 'Obj1' using the default constructor.  Create a
        //   'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' object 'R'.  Call the
        //   constructor to create 'bdef_Vfunc6' objects 'Obj2' and 'Obj3'
        //   using 'R'.
        //    1b. Assign 'Obj1' to 'Obj2'.  Execute
        //        'operator const void *() const'.
        //    2b. N/A.
        //    3b. Check the reference counter using the 'getCount' function.
        //        Assign 'Obj1' to 'Obj3'.  Using 'bslma::TestAllocator' verify
        //        that the memory allocated for 'R' was deallocated.
        //
        //   Create an object 'Obj1' using the default constructor.
        //    1c. Assign 'Obj1' to 'Obj1'.
        //        Execute 'operator const void *() const'.
        //    2c. N/A.
        //    3c. N/A.
        //
        //   Create the 'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' object.  Call
        //   the constructor to create the 'bdef_Vfunc6' object 'Obj1'.
        //    1d. Create an object 'Obj2' using the default constructor.
        //        Execute the assignment operator using 'Obj1' as the
        //        right-hand-side.  Execute 'operator const void *() const',
        //        and 'void operator()' of the newly assigned object.
        //    2d. Use 'invocationCounter' method of the 'FunctorRep' class
        //        to verify that 'Obj1' and 'Obj2' use the same 'bdefr_Vfunc6'
        //        object.
        //    3d. Check the reference counter using the 'getCount' function.
        //        Create another object and check it again.
        //
        //   Create two 'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' objects 'R1'
        //   and 'R2'.  Create two 'bdef_Vfunc6' objects 'Obj1' and 'Obj2'
        //   using 'R1' and 'R2', respectively.
        //    1e. Create an object 'Obj3' using a copy constructor with 'Obj1'
        //        as an argument.  Assign 'Obj2' to 'Obj3'.  Execute 'operator
        //        const void *() const', and 'void operator()' of 'Obj3'.
        //    2e. Use the 'invocationCounter' method of the 'FunctorRep' class
        //        to verify that 'Obj2' and 'Obj3' use the same 'bdefr_Vfunc6'
        //        object.
        //    3e. Check the reference counter using the 'getCount' function
        //        before and after the assignment for both 'Obj1' and 'Obj2'
        //        representations.
        //
        //   Create a 'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' object.  Call
        //   the constructor to create a 'bdef_Vfunc6' object 'Obj1'.
        //    1f. Assign 'Obj1' to 'Obj1'.
        //        Execute 'operator const void *() const', and 'void
        //        operator()'
        //    2f. N/A.
        //    3f. Check the reference counter using the 'getCount' function.
        //        Using 'bslma::TestAllocator', test that memory was not freed
        //        and then allocated again.
        //
        // Testing:
        //   bdef_Vfunc6<A1-6>& operator=(const bdef_Vfunc6<A1-6>&);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing Assignment Operator" << endl
                          << "===========================" << endl;

        if (verbose)
            cout << "\nTesting an assignment from an empty object." << endl;

        if (verbose)
            cout << "\tTesting an assignment to an empty object." << endl;
        {
            Obj x1;
            Obj x2;
            x2 = x1;

            if (veryVerbose)
                cout << "\t\tTesting that functor initialization is "
                        "detectable." << endl;
            ASSERT(!x2);
        }

        if (verbose)
            cout << "\tTesting an assignment to an initialized object."
                 << endl;
        {
            Obj x1;

            // int a1 = ARG1;
            // int a2 = ARG2;
            // int a3 = ARG3;
            // int a4 = ARG4;
            // int a5 = ARG5;
            // int a6 = ARG6;
            int a7 = ARG7;
            int a8 = ARG8;
            int a9 = ARG9;

            ObjRep *rep1 = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &a7, &a8, &a9, &testAllocator);
            int numDeallocations = testAllocator.numDeallocations();

            Obj x2(rep1);
            Obj x3(rep1);

            ASSERT(2 == getCount(rep1));

            x2 = x1;

            if (veryVerbose)
                cout << "\t\tTesting that functor initialization is "
                        "detectable." << endl;
            ASSERT(!x2);

            if (veryVerbose)
                cout << "\t\tTesting reference counter." << endl;
            if (veryVerbose)
                cout << "\t\t\tDecreased to positive number." << endl;
            ASSERT(1 == getCount(rep1));

            if (veryVerbose)
                cout << "\t\t\tDecreased to '0'." << endl;
            ASSERT(numDeallocations == testAllocator.numDeallocations());
            x3 = x1;
            ASSERT(numDeallocations + 1 == testAllocator.numDeallocations());
            ASSERT(sizeof(ParmMatchingFunctorImp) ==
                   testAllocator.lastAllocatedNumBytes());
        }

        if (verbose) cout << "\tTesting an assignment to self." << endl;
        {
            Obj x1;
            x1 = x1;
            if (veryVerbose)
                cout << "\t\tTesting that functor initialization is "
                        "detectable." << endl;
            ASSERT(!x1);
        }

        if (verbose)
            cout << "\nTesting an assignment from non-empty object." << endl;

        if (verbose)
            cout << "\tTesting an assignment to an empty object." << endl;
        {
            int a1 = ARG1;
            int a2 = ARG2;
            int a3 = ARG3;
            int a4 = ARG4;
            int a5 = ARG5;
            int a6 = ARG6;
            int a7 = ARG7;
            int a8 = ARG8;
            int a9 = ARG9;

            ObjRep *rep1 = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &a7, &a8, &a9, &testAllocator);
            Obj x1(rep1);
            Obj x2;
            x2 = x1;

            if (veryVerbose)
                cout << "\t\tTesting that functor initialization is "
                        "detectable." << endl;
            ASSERT(x2);

            if (veryVerbose)
                cout << "\t\tTesting that the functor executes correctly."
                     << endl;
            x2(&a1, &a2, &a3, &a4, &a5, &a6);
            ASSERT(ARG1_PLUS1 == a1);
            a1 = ARG1;

            if (veryVerbose)
                cout << "\t\tTesting that objects share representation."
                     << endl;
            ParmMatchingFunctorImp* rep1_2 = (ParmMatchingFunctorImp *)rep1;


            ASSERT(1 == rep1_2->invocationCounter());
            x1(&a1, &a2, &a3, &a4, &a5, &a6);
            a1 = ARG1;
            ASSERT(2 == rep1_2->invocationCounter());
            x2(&a1, &a2, &a3, &a4, &a5, &a6);
            a1 = ARG1;
            ASSERT(3 == rep1_2->invocationCounter());

            if (veryVerbose)
                cout << "\t\tTesting the reference counter." << endl;
            ASSERT(2 == getCount(rep1));
        }

        if (verbose)
            cout << "\tTesting assignment to an initialized object." << endl;
        {
            // int a1 = ARG1;
            // int a2 = ARG2;
            // int a3 = ARG3;
            // int a4 = ARG4;
            // int a5 = ARG5;
            // int a6 = ARG6;
            int a7 = ARG7;
            int a8 = ARG8;
            int a9 = ARG9;

            ObjRep *rep1 = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &a7, &a8, &a9, &testAllocator);
            Obj x1(rep1);

            int b1 = ARG1;
            int b2 = ARG2;
            int b3 = ARG3;
            int b4 = ARG4;
            int b5 = ARG5;
            int b6 = ARG6;
            int b7 = ARG7;
            int b8 = ARG8;
            int b9 = ARG9;

            ObjRep *rep2 = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &b7, &b8, &b9, &testAllocator);
            Obj x2(rep2);

            ASSERT(1 == getCount(rep1));
            ASSERT(1 == getCount(rep2));

            Obj x3(x1);
            x3 = x2;

            if (veryVerbose)
                cout << "\t\tTesting that functors share representation."
                     << endl;
            ParmMatchingFunctorImp* rep2_2 = (ParmMatchingFunctorImp *)rep2;

            ASSERT(0 == rep2_2->invocationCounter());
            x2(&b1, &b2, &b3, &b4, &b5, &b6);
            b1 = ARG1;
            ASSERT(1 == rep2_2->invocationCounter());
            x3(&b1, &b2, &b3, &b4, &b5, &b6);
            b1 = ARG1;
            ASSERT(2 == rep2_2->invocationCounter());

            if (veryVerbose)
                cout << "\t\tTesting that the functor executes correctly."
                     << endl;
            x3(&b1, &b2, &b3, &b4, &b5, &b6);
            ASSERT(ARG1_PLUS1 == b1);
            b1 = ARG1;

            if (veryVerbose)
                cout << "\t\tTesting reference counting." << endl;
            ASSERT(1 == getCount(rep1));
            ASSERT(2 == getCount(rep2));

            if (veryVerbose)
                cout << "\t\t\tWhen counter decreases to '0'." << endl;

            int numDeallocations = testAllocator.numDeallocations();
            x1 = x3;
            ASSERT(3 == getCount(rep2));
            ASSERT(numDeallocations + 1 == testAllocator.numDeallocations());
            ASSERT(sizeof(ParmMatchingFunctorImp) ==
                   testAllocator.lastAllocatedNumBytes());
        }

        if (verbose) cout << "\tTesting assignment to self." << endl;
        {
            int a1 = ARG1;
            int a2 = ARG2;
            int a3 = ARG3;
            int a4 = ARG4;
            int a5 = ARG5;
            int a6 = ARG6;
            int a7 = ARG7;
            int a8 = ARG8;
            int a9 = ARG9;

            ObjRep *rep1 = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &a7, &a8, &a9, &testAllocator);
            Obj x1(rep1);
            int numAlloc = testAllocator.numAllocations();
            int numDealloc = testAllocator.numDeallocations();
            x1 = x1;
            if (veryVerbose)
                cout << "\t\tTesting that ref. counter did not change."
                     << endl;
            ASSERT(1 == getCount(rep1));
            if (veryVerbose)
                cout << "\t\tTesting that memory was not newed/deleted."
                     << endl;
            ASSERT(numAlloc == testAllocator.numAllocations());
            ASSERT(numDealloc == testAllocator.numDeallocations());

            if (veryVerbose)
                cout << "\t\tTesting that functor initialization is visible."
                     << endl;
            ASSERT(x1);

            if (veryVerbose)
                 cout << "\t\tTesting that functor is executable." << endl;
            x1(&a1, &a2, &a3, &a4, &a5, &a6);
            ASSERT(ARG1_PLUS1 == a1);
            a1 = ARG1;
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // TESTING CREATORS and ACCESSORS
        //   For each of the following creators:
        //    a. default constructor
        //    b. constructor taking a pointer to a 'bdefr_Vfunc6' object
        //    c. copy constructor
        //
        //   we want to verify that:
        //    1. Method creates a fully initialized instance.
        //    2. For each of the created objects, the function is detectable
        //       with 'operator const void *() const' and can be executed with
        //       'void operator()'.
        //    3. The reference count of the 'bdefr_Vfunc6' object used is
        //       incremented.
        //
        //   Specific concerns for the copy constructor:
        //    4. A new object created by the copy constructor shares a
        //       'bdefr_Vfunc6' object with its copy.
        //
        //   Specific concerns for the destructor:
        //    5. The value of the reference counter is decreased when the
        //       envelope object is destroyed.
        //    6. The 'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6>' object
        //       is destroyed when the reference counter reaches '0'.
        //
        // Plan:
        //   1a. Create an object using the default constructor.
        //   2a. Verify that 'd_rep_p' is initialized to '0' using
        //       'operator const void *() const'.
        //
        //   1b. Create the 'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' object.
        //       Call the constructor taking it as an argument.
        //   2b. Check if the function is detectable with
        //       'operator const void *() const'.  Execute the function.
        //       Verify that the function was executed.
        //   3b. Verify that the reference counter was incremented using
        //       'getCount' method.  Create another object using the same
        //       'bdefr_Vfunc6<A1, A2, A3, A4, A5, A6> *' object.  Verify that
        //       the reference counter is incremented.
        //
        //   1c. Create an object using copy constructor.
        //   2c. Execute 'operator const void *() const', and
        //       'void operator()' of the newly created object.
        //   3c. Verify that the reference counter was incremented using
        //       'getCount' function.
        //   4.  Use 'invocationCounter' method of the 'FunctorRep' class
        //       to verify that both 'bdef_Vfunc6' objects use the same
        //       'bdefr_Vfunc6' object.
        //   5.  Destroy all objects.  Verify that the reference counter is
        //   6.  decreased.  When the last object is destroyed check the
        //       memory allocator to verify that the memory was freed.
        //
        // Testing:
        //   bdef_Vfunc6(const bdef_Vfunc6 &rhs);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing Creators and Accessors" << endl
                          << "==============================" << endl;
        {
            if (verbose) cout << "\nTesting Default Constructor." << endl;
            Obj x1;

            if (veryVerbose)
                cout << "\tTesting that the functor rep. is initialized to '0'"
                     << endl;
            ASSERT(!x1);

            if (verbose)
                cout << "\nTesting Constructor Taking '*bdefr_Vfunc6'."
                     << endl;

            int a1 = ARG1;
            int a2 = ARG2;
            int a3 = ARG3;
            int a4 = ARG4;
            int a5 = ARG5;
            int a6 = ARG6;
            int a7 = ARG7;
            int a8 = ARG8;
            int a9 = ARG9;

            ObjRep *rep = new(testAllocator)
                ParmMatchingFunctorImp(checkParameterMatching,
                                       &a7, &a8, &a9, &testAllocator);
            ASSERT(sizeof(ParmMatchingFunctorImp) ==
                       testAllocator.numBytesInUse());
            {
                Obj x1(rep);

                if (veryVerbose)
                    cout << "\tTesting that the functor rep. is detectable."
                         << endl;
                ASSERT(x1);

                if (veryVerbose)
                    cout << "\tTesting the 'execute' method." << endl;
                x1(&a1, &a2, &a3, &a4, &a5, &a6);
                ASSERT(ARG1_PLUS1 == a1);
                a1 = ARG1;
                if (veryVerbose)
                    cout << "\tTesting reference count." << endl;
                ASSERT(1 == getCount(rep));

                if (veryVerbose)
                    cout << "\tTesting that two objects share representation."
                         << endl;
                {
                    Obj x2(rep);
                    ParmMatchingFunctorImp *rep_2 =
                        (ParmMatchingFunctorImp *)rep;

                    ASSERT(1 == rep_2->invocationCounter());
                    x1(&a1, &a2, &a3, &a4, &a5, &a6);
                    a1 = ARG1;
                    ASSERT(2 == rep_2->invocationCounter());
                    x2(&a1, &a2, &a3, &a4, &a5, &a6);
                    a1 = ARG1;
                    ASSERT(3 == rep_2->invocationCounter());

                    if (veryVerbose)
                        cout << "\tTesting reference count." << endl;
                    ASSERT(2 == getCount(rep));

                    if (verbose) cout << "\nTest Copy Constructor." << endl;
                    {
                        Obj x3(x1);

                        if (veryVerbose)
                            cout << "\tTesting that the function is "
                                    "detectable." << endl;
                        ASSERT(x3);

                        if (veryVerbose)
                            cout << "\tTesting 'execute' method." << endl;
                        x3(&a1, &a2, &a3, &a4, &a5, &a6);
                        ASSERT(ARG1_PLUS1 == a1);
                        a1 = ARG1;

                        if (veryVerbose)
                            cout << "\tTesting that two objects share "
                                    "representation." << endl;
                        ASSERT(4 == rep_2->invocationCounter());
                        if (veryVerbose)
                            cout << "\tTesting reference counting." << endl;
                        ASSERT(3 == getCount(rep));
                    }
                    ASSERT(2 == getCount(rep));
                }
                ASSERT(1 == getCount(rep));
                ASSERT(sizeof(ParmMatchingFunctorImp) ==
                       testAllocator.numBytesInUse());
            }
            if (veryVerbose)
                cout << "\tTesting reference counting when the counter is '0'."
                     << endl;
            ASSERT(0 == testAllocator.numBytesInUse());
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
