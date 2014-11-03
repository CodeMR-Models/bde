// bslmf_removevolatile.h                                             -*-C++-*-
#ifndef INCLUDED_BSLMF_REMOVEVOLATILE
#define INCLUDED_BSLMF_REMOVEVOLATILE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a meta-function for removing 'volatile'-qualifier.
//
//@CLASSES:
//  bsl::remove_volatile: meta-function for removing 'volatile'-qualifier
//
//@SEE_ALSO: bslmf_addvolatile
//
//@AUTHOR:
//
//@DESCRIPTION: This component defines a meta-function, 'bsl::remove_volatile',
// that may be used to remove any top-level 'volatile'-qualifier from a type.
//
// 'bsl::remove_volatile' meets the requirements of the 'remove_volatile'
// template defined in the C++11 standard [meta.trans.cv].
//
///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Removing the 'volatile'-Qualifier of a Type
/// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to remove any 'volatile'-qualifier from a particular
// type.
//
// First, we create two 'typedef's -- a 'volatile'-qualified type
// ('MyVolatileType') and the same type without the 'volatile'-qualifier
// ('MyType'):
//..
//  typedef int          MyType;
//  typedef volatile int MyVolatileType;
//..
// Now, we remove the 'volatile'-qualifier from 'MyVolatileType' using
// 'bsl::remove_volatile' and verify that the resulting type is the same as
// 'MyType':
//..
//  assert(true == (bsl::is_same<bsl::remove_volatile<MyVolatileType>::type,
//                                                            MyType>::value));
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

namespace bsl {

                         // ======================
                         // struct remove_volatile
                         // ======================

template <class TYPE>
struct remove_volatile {
    // This 'struct' template implements the 'remove_volatile' meta-function
    // defined in the C++11 standard [meta.trans.cv], providing an alias,
    // 'type', that returns the result.  'type' has the same type as the
    // (template parameter) 'TYPE' except that any top-level
    // 'volatile'-qualifier has been removed.  Note that this generic default
    // template provides a 'type' that is an alias to 'TYPE' for when 'TYPE' is
    // not 'volatile'-qualified.  A template specialization is provided (below)
    // that removes the 'volatile'-qualifier for when 'TYPE' is
    // 'volatile'-qualified.

    // PUBLIC TYPES
    typedef TYPE type;
        // This 'typedef' is an alias to the (template parameter) 'TYPE'.
};

template <class TYPE>
struct remove_volatile<TYPE volatile> {
     // This partial specialization of 'bsl::remove_volatile', for when the
     // (template parameter) 'TYPE' is 'volatile'-qualified, provides a
     // 'typedef', 'type', that has the 'volatile'-qualifier removed.

    // PUBLIC TYPES
    typedef TYPE type;
        // This 'typedef' is an alias to the same type as the (template
        // parameter) 'TYPE' except with the 'volatile'-qualifier removed.
};

}  // close namespace bsl

#endif

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
