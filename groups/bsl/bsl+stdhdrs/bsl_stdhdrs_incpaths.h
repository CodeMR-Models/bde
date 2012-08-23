/* bsl_stdhdrs_incpaths.h                                          -*-C++-*- */
#ifndef INCLUDED_BSL_STDHDRS_INCPATHS
#define INCLUDED_BSL_STDHDRS_INCPATHS

#ifdef __cplusplus

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Define paths where native headers can be found
//
//@MACROS:
// BSL_NATIVE_CPP_LIB_HEADER(filename),
// BSL_NATIVE_CPP_RUNTIME_HEADER(filename)
// BSL_NATIVE_CPP_C_HEADER(filename),
// BSL_NATIVE_CPP_DEPRECATED_HEADER(filename),
// BSL_NATIVE_C_LIB_HEADER(filename)
//
//@AUTHOR: Pablo Halpern (phalpern), Arthur Chiu (achiu21)
//
//@DESCRIPTION: [!PRIVATE!] This component is intended for use only by other
// components in the bsl package group.  The stability of this interface is not
// guaranteed.
//
// The macros in this component allow a source file compiled in bde-stl mode to
// include the compiler's native C++ library headers, even when the compiler's
// search path would otherwise find the headers with the same name in the
// 'bsl+stdhdrs' package.  The actual paths are hard coded to be platform
// specific.
//
// Each macro takes a 'filename' argument and expands to the path for the file
// in the native compiler's directory.  The expanded macros are suitable for
// use in '#include' directives.  In order to find the native file, the correct
// macro must be used for each header file, as follows:
//..
//  Macro                               Used for
//  =================================   ======================================
//  BSL_NATIVE_CPP_LIB_HEADER           Most C++ standard header files
//
//  BSL_NATIVE_CPP_RUNTIME_HEADER       Headers closely related to the
//                                      compiler ('<exception>', '<typeinfo>',
//                                      and '<new>')
//
//  BSL_NATIVE_CPP_C_HEADER             C++ standard versions of the C-language
//                                      library (e.g., '<cstdlib>', '<cctype>')
//
//  BSL_NATIVE_CPP_DEPRECATED_HEADER    Headers to C++ standard header files
//                                      that have been deprecated (<strstream>)
//
//  BSL_NATIVE_C_LIB_HEADER             Headers that are part of the C
//                                      standard library (e.g., '<stdio.h>').
//                                      Note that this will *not* work for
//                                      'errno.h'
//..
// Note that 'BSL_NATIVE_C_LIB_HEADER' cannot be used to include the native
// version of 'errno.h' because 'errno' is '#define'd as a macro by several
// standard headers (errno.h, stdlib.h, stddef.h) - hence 'errno' cannot be
// correctly passed as an argument to the macros defined within this component.
//
///Usage
///-----
// The 'bsl+stdhdrs' package provides low-level system facilities, including a
// replacement for some of the features of the C++ standard library.  Under
// bde-stl mode, including headers '<new>', '<vector>', '<strsteam>', and
// '<cstdlib>' as follows:
//..
//  #include <new>
//  #include <vector>
//  #include <strstream>
//  #include <cstdlib>
//..
// will usually find Bloomberg's implementation of standard headers provided in
// the 'bsl+stdhdrs' package.  Sometimes, however, it is necessary to force
// inclusion of the standard headers natively supplied with the compiler.  In
// these cases, one can use the macros in this component, as follows:
//..
//  #include <bsl_bslhdrs_incpaths.h>
//
//  #include BSL_NATIVE_CPP_RUNTIME_HEADER(new)
//  #include BSL_NATIVE_CPP_LIB_HEADER(vector)
//  #include BSL_NATIVE_CPP_DEPRECATED_HEADER(strstream)
//  #include BSL_NATIVE_CPP_C_HEADER(cstring)
//  #include BSL_NATIVE_C_LIB_HEADER(stdio.h)
//..
// Note that the macros used for '<new>' is different from that used for
// '<vector>' because '<new>' is one of the headers that is closely related to
// the compiler.  Note also that neither the arguments to the macros nor their
// invocations are quoted or enclosed in angle brackets.

// Note that 'bslscm_version.h' is not included here because it pulls in c++
// symbols.  When someone includes a C header file (such as stdio.h) inside an
// 'extern "C"' block, the symbol inside bslscm_version (which is hidden inside
// an unnamed namespace) will be viewed as a C symbol (with the file scope
// ignored), which causes link errors when multiple files defining the version
// symbol are linked together.

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#if defined(BSLS_PLATFORM__CMP_SUN)
  // Sun CC 5.5 or above
  //
#   define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_CISO646_HEADER(filename) <../CC/std/filename>

#   if !defined(BDE_BUILD_TARGET_STLPORT)
#       define BSL_NATIVE_CPP_LIB_HEADER(filename) <../CC/Cstd/filename>
#       define BSL_NATIVE_CPP_RUNTIME_HEADER(filename) <../CC/filename>
#       define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename) <../CC/Cstd/filename>
#       define BSL_NATIVE_CPP_C_HEADER(filename) <../CC/std/filename>
#   else
#       define BSL_NATIVE_SUN_STLPORT_HEADER(filename)                        \
                                                      <../CC/stlport4/filename>
#       define BSL_NATIVE_CPP_LIB_HEADER(filename)                            \
                                        BSL_NATIVE_SUN_STLPORT_HEADER(filename)
#       define BSL_NATIVE_CPP_RUNTIME_HEADER(filename)                        \
                                        BSL_NATIVE_SUN_STLPORT_HEADER(filename)
#       define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename)                     \
                                        BSL_NATIVE_SUN_STLPORT_HEADER(filename)
#       define BSL_NATIVE_CPP_C_HEADER(filename)                              \
                                        BSL_NATIVE_SUN_STLPORT_HEADER(filename)
#   endif

#elif defined(BSLS_PLATFORM__CMP_CLANG)
  // Clang (should go before CMP_GNU)
#   define BSL_NATIVE_CPP_LIB_HEADER(filename) \
            <../__CLANG_GNUC__.__CLANG_GNUC_MINOR__.__CLANG_GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CPP_RUNTIME_HEADER(filename) \
            <../__CLANG_GNUC__.__CLANG_GNUC_MINOR__.__CLANG_GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename) \
            <../__CLANG_GNUC__.__CLANG_GNUC_MINOR__.__CLANG_GNUC_PATCHLEVEL__/backward/filename>
#   define BSL_NATIVE_CPP_C_HEADER(filename) \
            <../__CLANG_GNUC__.__CLANG_GNUC_MINOR__.__CLANG_GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CISO646_HEADER(filename) \
            <../__CLANG_GNUC__.__CLANG_GNUC_MINOR__.__CLANG_GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>

#elif defined(BSLS_PLATFORM__CMP_GNU)
  // gcc 4.1 or above
#   define BSL_NATIVE_CPP_LIB_HEADER(filename) \
            <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CPP_RUNTIME_HEADER(filename) \
            <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename) \
            <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/backward/filename>
#   define BSL_NATIVE_CPP_C_HEADER(filename) \
            <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_CISO646_HEADER(filename) \
            <../__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__/filename>
#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include/filename>

#   if defined(BSLS_PLATFORM__OS_HPUX)
#       define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include-fixed/filename>
#   else
#       define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>
#   endif

#elif defined(BSLS_PLATFORM__CMP_HP)
  // HP C/aC++
#   define BSL_NATIVE_CPP_LIB_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_CPP_RUNTIME_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_CPP_C_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_CISO646_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include_std/filename>
#   define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>

#else
  // Most other compilers
#   define BSL_NATIVE_CPP_LIB_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_CPP_RUNTIME_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_CPP_DEPRECATED_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_CPP_C_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_CISO646_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>
#endif

#else /* ! __cplusplus */

#   define BSL_NATIVE_C_LIB_HEADER(filename) <../include/filename>
#   define BSL_NATIVE_SYS_TIME_HEADER(filename) <../include/filename>

#endif /* __cplusplus */

#endif // ! defined(INCLUDED_BSL_STDHDRS_INCPATHS)

/*
// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2011
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
*/
