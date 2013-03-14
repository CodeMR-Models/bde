// bslmf_tag.h                                                        -*-C++-*-
#ifndef INCLUDED_BSLMF_TAG
#define INCLUDED_BSLMF_TAG

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an integral-constant-to-type conversion.
//
//@CLASSES:
//  bslmf::Tag: map integral constants to C++ types
//
//@MACROS:
//: BSLMF_TAG_TO_INT(EXPR): map tag to integral value
//: BSLMF_TAG_TO_BOOL(EXPR): map tag to boolean value
//
//@AUTHOR: Pablo Halpern (phalpern)
//
//@DESCRIPTION: This component defines a simple template structure used to map
// an integral constant to a C++ type.  'bslmf::Tag<unsigned>' defines a
// different type for each distinct compile-time constant integral parameter.
// That is, instantiations with different integer values form distinct types,
// so that 'bslmf::Tag<0>' is a distinct type from 'bslmf::Tag<1>', which, in
// turn, is also distinct from 'bslmf::Tag<2>', and so on.
//
// This component also provides two macros for mapping a 'bslmf::Tag<N>'
// instance to the integral value 'N' ('BSLMF_TAG_TO_INT'), and to the boolean
// value 'N != 0' ('BSLMF_TAG_TO_BOOL').
//
///Macro Summary
///-------------
// This section provides a brief description of the macros defined in this
// component.
//
//: 'BSLMF_TAG_TO_INT(EXPR)'
//:     Given an integral value, 'V', and an expression, 'EXPR', of type
//:     'bslmf::Tag<V>', this macro returns a compile-time constant with the
//:     value 'V'.  'EXPR' is not evaluated at run-time.
//:
//: 'BSLMF_TAG_TO_BOOL(EXPR)'
//:     Given an integral value, 'V', and an expression, 'EXPR', of type
//:     'bslmf::Tag<V>', this macro returns a compile-time constant with the
//:     value 'true' or 'false', depending on the boolean value of 'V'.  'EXPR'
//:     is not evaluated at run-time.
//
///Usage
///-----
// The most common use of this structure is to perform static function
// dispatching based on a compile-time calculation.  Often the calculation is
// nothing more than a simple predicate, allowing us to select one of two
// functions.  The following function, 'doSomething', uses a fast
// implementation (e.g., 'memcpy') if the parameterized type allows for such
// operations; otherwise it will use a more generic and slower implementation
// (e.g., copy constructor).
//..
//  template <class T>
//  void doSomethingImp(T *t, bslmf::Tag<0> *)
//  {
//      // slow but generic implementation
//  }
//
//  template <class T>
//  void doSomethingImp(T *t, bslmf::Tag<1> *)
//  {
//      // fast implementation (appropriate for bitwise-movable types)
//  }
//
//  template <class T, bool IsFast>
//  void doSomething(T *t)
//  {
//      doSomethingImp(t, (bslmf::Tag<IsFast> *)0);
//  }
//..
// For some parameter types, the fast version of 'doSomethingImp' is not legal.
// The power of this approach is that the compiler will compile just the
// implementation selected by the tag argument.
//..
//  void f()
//  {
//      int i;
//      doSomething<int, true>(&i);      // fast version selected for 'int'
//
//      double m;
//      doSomething<double, false>(&m);  // slow version selected for 'double'
//  }
//..
// Note that an alternative design would be to use template partial
// specialization instead of standard function overloading to avoid the cost of
// passing a 'bslmf::Tag<N>' pointer.
//
// The value of the integral parameter supplied to an instantiation of
// 'bslmf::Tag<N>' is "recoverable" by using the 'BSLMF_TAG_TO_INT' macro.  For
// example:
//..
//  bslmf::Tag<7> tag;
//  assert( 7 == BSLMF_TAG_TO_INT(tag));
//  assert(53 == BSLMF_TAG_TO_INT(bslmf::Tag<50 + 3>()));
//..
// The 'BSLMF_TAG_TO_BOOL' macro can be used to determine if the parameter is
// non-zero:
//..
//  assert( 1 == BSLMF_TAG_TO_BOOL(tag));
//  assert( 0 == BSLMF_TAG_TO_BOOL(bslmf::Tag<0>()));
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

namespace BloombergLP {

namespace bslmf {

                           // =============
                           // struct Tag<N>
                           // =============

template <unsigned N>
struct Tag {
    // This template class is never intended to produce a run-time instance.
    // The only useful attribute of a tag is its size (which is, of course,
    // computable at compile time, even if an instance is never created).  Note
    // that in case of overflow on Linux 64-bit machines, we split the size
    // into 2 data members.

    // DATA
    char d_upperSizeArray[(N >> 16)        + 1];
    char d_lowerSizeArray[(N & 0x0000FFFF) + 1];
};

}  // close package namespace

#define BSLMF_TAG_TO_UINT(BSLMF_EXPR)                                        \
                         (((sizeof(BSLMF_EXPR.d_upperSizeArray) - 1) << 16)  \
                         | (sizeof(BSLMF_EXPR.d_lowerSizeArray) - 1))

#define BSLMF_TAG_TO_INT(BSLMF_EXPR) ((int)BSLMF_TAG_TO_UINT(BSLMF_EXPR))

#define BSLMF_TAG_TO_BOOL(BSLMF_EXPR) (BSLMF_TAG_TO_INT(BSLMF_EXPR) != 0)

#ifndef BDE_OMIT_TRANSITIONAL  // BACKWARD_COMPATIBILITY
// ===========================================================================
//                           BACKWARD COMPATIBILITY
// ===========================================================================

#ifdef bslmf_Tag
#undef bslmf_Tag
#endif
#define bslmf_Tag bslmf::Tag
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
