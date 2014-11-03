// bslmf_ispair.h                                                     -*-C++-*-
#ifndef INCLUDED_BSLMF_ISPAIR
#define INCLUDED_BSLMF_ISPAIR

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a compile-time check for the bsl::pair type.
//
//@CLASSES:
//  bslmf::IsPair: meta-function for determining the bsl::pair type.
//
//@SEE_ALSO:
//
//@DESCRIPTION: This component defines a simple template structure used to
// evaluate whether it's parameter is a 'bsl::pair' type.  It's needed to
// enable 'bslalg::TypeTraitPair' type trait.

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

namespace BloombergLP {

namespace bslmf {

template <class TYPE>
struct IsPair : bsl::false_type {};

}  // close package namespace
}  // close enterprise namespace

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
