// btescm_version.h                                                   -*-C++-*-
#ifndef INCLUDED_BTESCM_VERSION
#define INCLUDED_BTESCM_VERSION

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide source control management (versioning) information.
//
//@CLASSES:
//  baescm_Version: namespace for RCS and SCCS versioning information for 'bte'
//
//@AUTHOR: Jeffrey Mendelsohn (jmendels)
//
//@DESCRIPTION: This component provides source control management (versioning)
// information for the 'bte' package group.  In particular, this component
// embeds RCS-style and SCCS-style version strings in binary executable files
// that use one or more components from the 'bte' package group.  This version
// information may be extracted from binary files using common UNIX utilities
// (e.g., 'ident' and 'what').  In addition, the 'version' 'static' member
// function in the 'btescm_Version' struct can be used to query version
// information for the 'bte' package group at runtime.  The following USAGE
// examples illustrate these two basic capabilities.
//
// Note that unless the 'version' method will be called, it is not necessary to
// "#include" this component header file to get 'bte' version information
// embedded in an executable.  It is only necessary to use one or more 'bte'
// components (and, hence, link in the 'bte' library).

#ifndef INCLUDED_BCESCM_VERSION
#include <bcescm_version.h>
#endif

#ifndef INCLUDED_BDESCM_VERSION
#include <bdescm_version.h>
#endif

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

namespace BloombergLP {

struct btescm_Version {
    static const char *s_ident;
    static const char *s_what;

    static const char *s_version;
    static const char *s_dependencies;
    static const char *s_buildInfo;
    static const char *s_timestamp;
    static const char *s_sourceControlInfo;

    static const char *version();
};

inline
const char *btescm_Version::version()
{
    return s_version;
}

// Force linker to pull in this component's object file.

#if defined(BSLS_PLATFORM_CMP_IBM)
static const char **btescm_version_assertion = &btescm_Version::s_version;
#else
namespace {
    extern const char **const btescm_version_assertion =
                                                    &btescm_Version::s_version;
}
#endif

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
