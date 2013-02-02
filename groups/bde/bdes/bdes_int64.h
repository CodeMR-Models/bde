// bdes_int64.h                 -*-C++-*-
#ifndef INCLUDED_BDES_INT64
#define INCLUDED_BDES_INT64

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide namespace for platform-dependent 64-bit integer types.
//
//@DEPRECATED: Use 'bsls_types' instead.
//
//@CLASSES:
//   bdes_Int64: namespace for platform-dependent 64-bit integer types
//
//@AUTHOR: Tom Marshall (tmarshal)
//
//@DESCRIPTION: This component provides platform-independent type names for
// both signed and unsigned 64 bit integers.  For healthy platforms, the
// component simply provides a BDE-standard 'typedef' for the non-standard
// native type name.  For defective platforms or compilers, a bde-relevant
// subset of the needed functionality can be implemented.
//
// The types (and platform-specific 'typedef's) exposed here are intended for
// use by 'bdes_platformutil' only (and not directly by any other client).
// This component exists to factor out various pathologies, present and future,
// with 64-bit integers.
//
// !WARNING!  If a given platform has a defective native 64-bit integer type,
// then there is no guarantee that the types supplied here will be suitable
// *except* as they are explicitly used by other BDE components.  If a user
// encounters problems with the types, please contact the author or a BDE
// library maintenance engineer.
//
///USAGE
///-----
// Do not use this component; use 'bsls_types' instead.

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>  // required by 'bdes'
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

namespace BloombergLP {

                          // =================
                          // struct bdes_Int64
                          // =================

struct bdes_Int64 {
    // Provide a namespace for the implementations of platform-neutral 64-bit
    // integers.  Namely, this 'struct' defines 'Int64' as a 64-bit signed
    // integer and 'Uint64' as a 64-bit unsigned integer for all supported
    // platforms.

    // TYPES
    typedef bsls::Types::Int64  Int64;
    typedef bsls::Types::Uint64 Uint64;
        // DEPRECATED: Use 'bsls::Types::Int64' and 'bsls::Types::Uint64',
        // respectively, instead.
};

}  // close namespace BloombergLP

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2002
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
