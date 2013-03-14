// bslmf_issame.h                                                     -*-C++-*-
#ifndef INCLUDED_BSLMF_ISSAME
#define INCLUDED_BSLMF_ISSAME

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a meta-function for testing if two types are the same.
//
//@CLASSES:
//  bsl::is_same: standard meta-function for testing if two types are the same
//  bslmf::IsSame: meta-function for testing if two types are the same
//
//@AUTHOR: Shawn Edwards (sedwards)
//
//@DESCRIPTION: This component defines two meta-functions, 'bsl::is_same' and
// 'BloombergLP::bslmf::IsSame', both of which may be used to query whether two
// types are the same.  Two types are the same if they name the same type
// having the same cv-qualifications.
//
// 'bsl::is_same' meets the requirements of the 'is_same' template defined in
// the C++11 standard [meta.rel], while 'bslmf::IsSame' was devised before
// 'is_same' was standardized.
//
// The two meta-functions are functionally equivalent.  The major difference
// between them is that the result for 'bsl::is_same' is indicated by the class
// member 'value', while the result for 'bslmf::IsSame' is indicated by the
// class member 'VALUE'.
//
// Note that 'bsl::is_same' should be preferred over 'bslmf::IsSame', and in
// general, should be used by new components.
//
///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Determine Same Types
///- - - - - - - - - - - - - - - -
// Suppose that we have several pairs of types and want to assert whether
// the types in each pair are the same.
//
// First, we define several 'typedef's:
//..
//  typedef       int    INT1;
//  typedef       double DOUBLE;
//  typedef       short  SHORT;
//  typedef const short  CONST_SHORT;
//  typedef       int    INT2;
//  typedef       int&   INT_REF;
//..
// Now, we instantiate the 'bsl::is_same' template for certain pairs of the
// 'typedef's and assert the 'value' static data member of each instantiation:
//..
//  assert(true  == (bsl::is_same<INT1, INT2>::value));
//  assert(false == (bsl::is_same<INT, DOUBLE>::value));
//..
// Note that a 'const'-qualified type is considered distinct from the
// non-'const' (but otherwise identical) type:
//..
//  assert(false == (bsl::is_same<SHORT, CONST_SHORT>::value));
//..
// Similarly, a 'TYPE' and a reference to 'TYPE' ('TYPE&') are distinct:
//..
//  assert(false == (bsl::is_same<INT, INT_REF>::value));
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_METAINT
#include <bslmf_metaint.h>
#endif

namespace bsl {

                            // ==============
                            // struct is_same
                            // ==============

template <class TYPE1, class TYPE2>
struct is_same : false_type {
    // This 'struct' template provides a meta-function to determine whether the
    // (template parameter) 'TYPE1' and the (template parameter) 'TYPE2' are
    // the same.  This generic default template derives from 'bsl::false_type'.
    // A template specialization is provided (below) that derives from
    // 'bsl::true_type'.
};

template <class TYPE>
struct is_same<TYPE, TYPE> : true_type {
     // This partial specialization of 'is_same' derives from 'bsl::true_type'
     // for when the (template parameter) types are the same.
};

}  // close namespace bsl

namespace BloombergLP {

namespace bslmf {

                            // =============
                            // struct IsSame
                            // =============

template <class TYPE1, class TYPE2>
struct IsSame : bsl::is_same<TYPE1, TYPE2>::type {
    // This 'struct' template implements a meta-function to determine if the
    // (template parameter) 'TYPE1' and the (template parameter) 'TYPE2' are
    // the same.  This 'struct' derives from 'bsl::true_type' if 'TYPE1' and
    // 'TYPE2' are the same, and 'bsl::false_type' otherwise.
    //
    // Note that although this 'struct' is functionally equivalent to
    // 'bsl::is_same', the use of 'bsl::is_same' should be preferred.
};

}  // close package namespace
}  // close enterprise namespace

#ifndef BDE_OMIT_TRANSITIONAL  // BACKWARD_COMPATIBILITY
// ===========================================================================
//                           BACKWARD COMPATIBILITY
// ===========================================================================

#ifdef bslmf_IsSame
#undef bslmf_IsSame
#endif
#define bslmf_IsSame bslmf::IsSame
    // This alias is defined for backward compatibility.
#endif  // BDE_OMIT_TRANSITIONAL -- BACKWARD_COMPATIBILITY

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
