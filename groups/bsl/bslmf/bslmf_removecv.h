// bslmf_removecv.h                                                   -*-C++-*-
#ifndef INCLUDED_BSLMF_REMOVECV
#define INCLUDED_BSLMF_REMOVECV

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a meta-function for removing top-level cv-qualifiers.
//
//@CLASSES:
//  bsl::remove_const: meta-function for removing top-level cv-qualifiers
//
//@SEE_ALSO: bslmf_addcv
//
//@AUTHOR:
//
//@DESCRIPTION: This component defines a meta-function, 'bsl::remove_cv', that
// may be used to remove any top-level cv-qualifiers ('const'-qualifier and
// 'volatile'-qualifier) from a type.
//
// 'bsl::remove_const' meets the requirements of the 'remove_cv' template
// defined in the C++11 standard [meta.trans.cv].
//
///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Removing the CV-Qualifiers of a Type
///- - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to remove the cv-qualifiers from a particular type.
//
// First, we create two 'typedef's -- a 'const'-qualified and
// 'volatile'-qualified type ('MyCvType') and the same type without the
// cv-qualifier ('MyType'):
//..
//  typedef int                MyType;
//  typedef const volatile int MyCvType;
//..
// Now, we remove the cv-qualifiers from 'MyCvType' using 'bsl::remove_cv' and
// verify that the resulting type is the same as 'MyType':
//..
//  assert(true == (bsl::is_same<bsl::remove_cv<MyCvType>::type,
//                                                            MyType>::value));
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_REMOVECONST
#include <bslmf_removeconst.h>
#endif

#ifndef INCLUDED_BSLMF_REMOVEVOLATILE
#include <bslmf_removevolatile.h>
#endif

namespace bsl {

                         // ================
                         // struct remove_cv
                         // ================

template <typename TYPE>
struct remove_cv {
    // This 'struct' template implements the 'remove_cv' meta-function defined
    // in the C++11 standard [meta.trans.cv], providing an alias, 'type', that
    // returns the result.  'type' has the same type as the (template
    // parameter) 'TYPE' except that any top-level cv-qualifiers has been
    // removed.

    // PUBLIC TYPES
    typedef typename remove_const<typename remove_volatile<TYPE>::type>::type
                                                                          type;
        // This 'typedef' is an alias to the same type as the (template
        // parameter) 'TYPE' except that any top-level cv-qualifier has been
        // removed.
};

}  // close namespace bsl

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
