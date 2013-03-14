// bslstp_exalgorithm.h                                               -*-C++-*-
#ifndef INCLUDED_BSLSTP_EXALGORITHM
#define INCLUDED_BSLSTP_EXALGORITHM

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a namespace for algorithm extensions
//
//@CLASSES:
//  bslstp::exalgorithm::copy_n: copy 'n' elements
//
//@SEE_ALSO: bsl+stdhdrs
//
//@AUTHOR: Arthur Chiu (achiu21)
//
//@DESCRIPTION: This component is for internal use only.  Please include
// '<bsl_algorithm>' instead.  This component provides a namespace for
// algorithm extensions that are not specified in the C++ standard.  This
// provides backward compatibility to code using extensions that STLPort
// provides.
//
// Note that the functions in this component are based on STLPort's
// implementation, with copyright notice as follows:
//..
//-----------------------------------------------------------------------------
// Copyright (c) 1994
// Hewlett-Packard Company
//
// Copyright (c) 1996,1997
// Silicon Graphics Computer Systems, Inc.
//
// Copyright (c) 1997
// Moscow Center for SPARC Technology
//
// Copyright (c) 1999
// Boris Fomitchev
//
// This material is provided "as is", with absolutely no warranty expressed
// or implied.  Any use is at your own risk.
//
// Permission to use or copy this software for any purpose is hereby granted
// without fee, provided the above notices are retained on all copies.
// Permission to modify the code and to distribute modified code is granted,
// provided the above notices are retained, and a notice that the code was
// modified is included with the above copyright notice.
//-----------------------------------------------------------------------------
//..
//
///Usage
///-----
// This component is for use by the 'bsl+stdhdrs' package.  Use 'algorithm'
// directly.

#ifdef BDE_OMIT_TRANSITIONAL // STP
#error "bslstp_exalgorithm is not for publication"
#endif

// Prevent 'bslstl' headers from being included directly in 'BSL_OVERRIDES_STD'
// mode.  Doing so is unsupported, and is likely to cause compilation errors.
#if defined(BSL_OVERRIDES_STD) && !defined(BSL_STDHDRS_PROLOGUE_IN_EFFECT)
#error "include <bsl_algorithm.h> instead of <bslstp_exalgorithm.h> in \
BSL_OVERRIDES_STD mode"
#endif

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLSTL_ITERATOR
#include <bslstl_iterator.h>  // iterator tags
#endif

#ifndef INCLUDED_BSLSTL_PAIR
#include <bslstl_pair.h>
#endif

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>          // 'copy'
#define INCLUDED_ALGORITHM
#endif

namespace bsl {

// PRIVATE FREE FUNCTIONS
template <class InputIter, class Size, class OutputIter>
pair<InputIter, OutputIter>
bslstp_ExAlgorithm_CopyNImp(InputIter  first,
                            Size       count,
                            OutputIter result,
                            const      input_iterator_tag&)
{
    for ( ; count > 0; --count) {
        *result = *first;
        ++first;
        ++result;
    }
    return pair<InputIter, OutputIter>(first, result);
}

template <class RAIter, class Size, class OutputIter>
inline
pair<RAIter, OutputIter>
bslstp_ExAlgorithm_CopyNImp(RAIter     first,
                            Size       count,
                            OutputIter result,
                            const      random_access_iterator_tag&)
{
    RAIter last = first + count;
    return pair<RAIter, OutputIter>(last,
                                    native_std::copy(first, last, result));
}

// FREE FUNCTIONS
template <class InputIter, class Size, class OutputIter>
inline
pair<InputIter, OutputIter> copy_n(InputIter  first,
                                   Size       count,
                                   OutputIter result)
    // Copy the specified 'count' elements from the specified 'first' address
    // to the specified 'result' address.
{
    typedef typename iterator_traits<InputIter>::iterator_category tag;
    return bslstp_ExAlgorithm_CopyNImp(first, count, result, tag());
}

#if defined(BSLS_PLATFORM_CMP_SUN) && !defined(BDE_BUILD_TARGET_STLPORT)

// Provide an override for 'count' since Sun only provides a 4 argument version
// while the C++ standard requires a 3 argument version.
template <class InputIter, class TYPE>
inline
typename iterator_traits<InputIter>::difference_type
count(InputIter first, InputIter last, const TYPE& value)
{
    typename iterator_traits<InputIter>::difference_type ret = 0;
    native_std::count(first, last, value, ret);
    return ret;
}

// Provide an override for 'count_if' since Sun only provides a 4 argument
// version while the C++ standard requires a 3 argument version.
template <class InputIter, class PREDICATE>
inline
typename iterator_traits<InputIter>::difference_type
count_if(InputIter first, InputIter last, PREDICATE pred)
{
    typename iterator_traits<InputIter>::difference_type ret = 0;
    native_std::count_if(first, last, pred, ret);
    return ret;
}

#else

// On all other platforms, use the compiler vendor supplied version of 'count'
// and 'count_if'.
using native_std::count;
using native_std::count_if;

#endif  // BSLS_PLATFORM_CMP_SUN && !BDE_BUILD_TARGET_STLPORT

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
