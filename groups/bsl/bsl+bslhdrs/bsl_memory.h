// bsl_memory.h                                                       -*-C++-*-
#ifndef INCLUDED_BSL_MEMORY
#define INCLUDED_BSL_MEMORY

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide functionality of the corresponding C++ Standard header.
//
//@SEE_ALSO: package bsl+stdhdrs
//
//@AUTHOR: Arthur Chiu (achiu21)
//
//@DESCRIPTION: Provide types, in the 'bsl' namespace, equivalent to those
// defined in the corresponding C++ standard header.  Include the native
// compiler-provided standard header, and also directly include Bloomberg's
// implementation of the C++ standard type (if one exists).  Finally, place the
// included symbols from the 'std' namespace (if any) into the 'bsl' namespace.

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#include <memory>

namespace bsl
{
    // Import selected symbols into bsl namespace
    using native_std::new_handler;
    using native_std::advance;
    using native_std::auto_ptr;
    using native_std::bad_alloc;
    using native_std::bidirectional_iterator_tag;
    using native_std::forward_iterator_tag;
    using native_std::get_temporary_buffer;
    using native_std::input_iterator_tag;
    using native_std::iterator;
    using native_std::nothrow;
    using native_std::nothrow_t;
    using native_std::output_iterator_tag;
    using native_std::random_access_iterator_tag;
    using native_std::raw_storage_iterator;
    using native_std::return_temporary_buffer;
    using native_std::set_new_handler;
    using native_std::uninitialized_copy;
    using native_std::uninitialized_fill;
    using native_std::uninitialized_fill_n;
}

// Include Bloomberg's implementation, unless compilation is configured to
// override native types in the 'std' namespace with Bloomberg's
// implementation, in which case the implementation file will be included
// by the Bloomberg supplied standard header file.

#ifndef BSL_OVERRIDES_STD
#include <bslstl_allocator.h>
#endif

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2009
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
