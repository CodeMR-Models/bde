// bslmf_addreference.h                                               -*-C++-*-
#ifndef INCLUDED_BSLMF_ADDREFERENCE
#define INCLUDED_BSLMF_ADDREFERENCE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a meta-function for adding "reference-ness" to a type.
//
//@CLASSES:
//  bslmf::AddReference: meta-function to form a reference to a type
//
//@AUTHOR: Alisdair Meredith (ameredit)
//
//@DESCRIPTION: This component defines a simple template 'struct',
// 'bslmf::AddReference', that is used to define a reference type from the type
// supplied as its single template type parameter.  Types that are 'void' or
// already reference types are unmodified.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: A Simple Wrapper Class
///- - - - - - - - - - - - - - - - -
// First, let us write a simple class that can wrap any other type:
//..
//  template <class TYPE>
//  class Wrapper {
//    private:
//      // DATA
//      TYPE d_data;
//
//    public:
//      // TYPES
//      typedef typename bslmf::AddReference<TYPE>::Type WrappedType;
//
//      // CREATORS
//      Wrapper(TYPE value) : d_data(value) {}
//          // Create a 'Wrapper' object having the specified 'value'.
//
//      //! ~Wrapper() = default;
//          // Destroy this object.
//..
// Then, we would like to expose access to the wrapped element through a method
// that returns a reference to the data member 'd_data'.  However, there would
// be a problem if the user supplied a parameterized type 'TYPE' that is a
// reference type, as references-to-references were not permitted by the
// language (prior the C++11 standard).  We can resolve such problems using the
// meta-function 'bslmf::AddReference'.
//..
//  // MANIPULATORS
//  typename bslmf::AddReference<TYPE>::Type value()
//  {
//      return d_data;
//  }
//..
// Next, we supply an accessor function, 'value', that similarly wraps the
// parameterized type 'TYPE' with the 'bslmf::AddReference' meta-function.  In
// this case we must remember to const-quality 'TYPE' before passing it on to
// the meta-function.
//..
//      // ACCESSORS
//      typename bslmf::AddReference<const TYPE>::Type value() const
//      {
//          return d_data;
//      }
//  };
//..
// Now, we write a test function, 'runTest', to verify our simple wrapper type.
// We start by wrapping a simple 'int' value:
//..
//  void runTests()
//  {
//      int i = 42;
//
//      Wrapper<int> ti(i);  const Wrapper<int>& TI = ti;
//      assert(42 == i);
//      assert(42 == TI.value());
//
//      ti.value() = 13;
//      assert(42 == i);
//      assert(13 == TI.value());
//..
// Finally, we test 'Wrapper' with a reference type:
//..
//      Wrapper<int&> tr(i);  const Wrapper<int&>& TR = tr;
//      assert(42 == i);
//      assert(42 == TR.value());
//
//      tr.value() = 13;
//      assert(13 == i);
//      assert(13 == TR.value());
//
//      i = 42;
//      assert(42 == i);
//      assert(42 == TR.value());
//  }
//..
///Example 2: Expected Results
///- - - - - - - - - - - - - -
// For this example, the associated comments below indicate the expected type
// of 'bslmf::AddReference::Type' for a broad range of parameterized types:
//..
//  struct MyType {};
//  typedef MyType& MyTypeRef;
//
//  bslmf::AddReference<int             >::Type x1; // int&
//  bslmf::AddReference<int&            >::Type x2; // int&
//  bslmf::AddReference<int volatile    >::Type x3; // volatile int&
//  bslmf::AddReference<int volatile&   >::Type x4; // volatile int&
//
//  bslmf::AddReference<MyType          >::Type     // MyType&
//  bslmf::AddReference<MyType&         >::Type     // MyType&
//  bslmf::AddReference<MyTypeRef       >::Type     // MyType&
//  bslmf::AddReference<MyType const    >::Type     // const MyType&
//  bslmf::AddReference<MyType const&   >::Type     // const MyType&
//  bslmf::AddReference<const MyTypeRef >::Type     // MyType&
//  bslmf::AddReference<const MyTypeRef&>::Type     // MyType& (REQUIRES C++11)
//
//  bslmf::AddReference<void            >::Type     // void
//  bslmf::AddReference<void *          >::Type     // void *&
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_ADDLVALUEREFERENCE
#include <bslmf_addlvaluereference.h>
#endif

namespace BloombergLP {

namespace bslmf {

                        // ===================
                        // struct AddReference
                        // ===================


template <class TYPE>
struct AddReference {
    // This meta-function class defines a typedef, 'Type', that is an alias for
    // a reference to the parameterized 'TYPE'.  References to cv-qualified
    // 'void' will produce the original 'void' type and not a reference (see
    // specializations below).  References-to-references "collapse" to produce
    // an alias to the original reference type, which is the revised rule
    // according to the C++11 standard.  Note that there is no requirement that
    // the parameterized 'TYPE' be a complete type.

    typedef typename bsl::add_lvalue_reference<TYPE>::type Type;
};

}  // close package namespace

#ifndef BDE_OMIT_TRANSITIONAL  // BACKWARD_COMPATIBILITY
// ===========================================================================
//                           BACKWARD COMPATIBILITY
// ===========================================================================

#ifdef bslmf_AddReference
#undef bslmf_AddReference
#endif
#define bslmf_AddReference bslmf::AddReference
    // This alias is defined for backward compatibility.
#endif  // BDE_OMIT_TRANSITIONAL -- BACKWARD_COMPATIBILITY

}  // close enterprise namespace

#endif

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
