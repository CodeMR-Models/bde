// baesu_objectfileformat.h                                           -*-C++-*-
#ifndef INCLUDED_BAESU_OBJECTFILEFORMAT
#define INCLUDED_BAESU_OBJECTFILEFORMAT

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide platform-dependent object file format trait definitions.
//
//@CLASSES:
//   baesu_ObjectFileFormat: namespace for object file format traits
//
//@SEE_ALSO:
//
//@AUTHOR: Bill Chapman
//
//@DESCRIPTION: This component defines a set of traits that identify and
// describe a platform's object file format properties.  For example, the
// 'baesu_ObjectFileFormat::ResolverPolicy' trait is ascribed a "value" (i.e.,
// 'Elf' or 'Xcoff') appropriate for each supported platform.  The various
// stack trace traits are actually types declared in the
// 'bdescu_ObjectFileFormat' 'struct'.  These types are intended to be used in
// specializing template implementations or to enable function overloading
// based on the prevalent system's characteristics.  #defines are also
// provided by this component to facilitate conditional compilation depending
// upon object file formats.
//
///Usage
///-----
// In this section we show the intended usage of this component.
//
///Example 1: Accessing 'baesu_ObjectFileFormat' Information at Run Time
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// The templated (specialized) 'typeTest' function returns a unique, non-zero
// value when passed an object of types
// 'baesu_ObjectFileFormat::{Elf,Xcoff,Windows}', and 0 otherwise.
//..
//  template <typename TYPE>
//  int typeTest(const TYPE &)
//  {
//      return 0;
//  }
//
//  int typeTest(const baesu_ObjectFileFormat::Elf &)
//  {
//      return 1;
//  }
//
//  int typeTest(const baesu_ObjectFileFormat::Xcoff &)
//  {
//      return 2;
//  }
//
//  int typeTest(const baesu_ObjectFileFormat::Windows &)
//  {
//      return 3;
//  }
//
//  int main() ...
//..
// We define an object 'policy' of type 'baesu_ObjectFileFormat::Policy', which
// will be of type '...::Elf', '...::Xcoff', or '...::Windows' appropriate for
// the platform.
//..
//      baesu_ObjectFileFormat::Policy policy;
//..
// We now test it using 'typeTest':
//..
//      assert(typeTest(policy) > 0);
//
//  #if defined(BAESU_OBJECTFILEFORMAT_RESOLVER_ELF)
//      assert(1 == typeTest(policy));
//  #endif
//
//  #if defined(BAESU_OBJECTFILEFORMAT_RESOLVER_XCOFF)
//      assert(2 == typeTest(policy));
//  #endif
//
//  #if defined(BAESU_OBJECTFILEFORMAT_RESOLVER_WINDOWS)
//      assert(3 == typeTest(policy));
//  #endif
//  }
//..

#ifndef INCLUDED_BAESCM_VERSION
#include <baescm_version.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

namespace BloombergLP {

                        // ============================
                        // class baesu_ObjectFileFormat
                        // ============================

struct baesu_ObjectFileFormat {
    // This 'struct' is named 'ObjectFileFormat' for historical reasons, what
    // it really determines is resolving strategy.  Linux, for example, can be
    // resolved using either the 'Elf' or 'Dladdr' policies.  We choose 'Elf'
    // for linux because that mode of resolving yields more information.

    struct Elf {};        // resolve as elf object

    struct Xcoff {};      // resolve as xcoff object

    struct Windows {};    // format used on Microsoft Windows platform

    struct Dladdr {};     // resulve using the 'dladdr' call

    struct Dummy {};

#if defined(BSLS_PLATFORM_OS_SOLARIS) || \
    defined(BSLS_PLATFORM_OS_LINUX)   || \
    defined(BSLS_PLATFORM_OS_HPUX)

    typedef Elf Policy;
#   define BAESU_OBJECTFILEFORMAT_RESOLVER_ELF 1

#elif defined(BSLS_PLATFORM_OS_AIX)

    typedef Xcoff Policy;
#   define BAESU_OBJECTFILEFORMAT_RESOLVER_XCOFF 1

#elif defined(BSLS_PLATFORM_OS_WINDOWS)

    typedef Windows Policy;
#   define BAESU_OBJECTFILEFORMAT_RESOLVER_WINDOWS 1

#elif defined(BSLS_PLATFORM_OS_DARWIN)

    typedef Dladdr Policy;
#   define BAESU_OBJECTFILEFORMAT_RESOLVER_DLADDR 1

#else

    typedef Dummy Policy;
#   error unrecognized platform
#   define BAESU_OBJECTFILEFORMAT_RESOLVER_UNIMPLEMENTED 1

#endif

};

}  // close namespace BloombergLP

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2011
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
