// bslma_managedptr.cpp                                               -*-C++-*-
#include <bslma_managedptr.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#include <bslma_newdeleteallocator.h>
#include <stdio.h>    // 'sprintf'

// Design notes
// ------------
// These notes are recorded here as they are not part of the public interface,
// but reflect certain design decisions taken when implementing this component.
// A number of redundant constructors were removed from earlier designs in
// order to produce the leanest component with minimal template bloat.  One
// side effect of this is that some signatures, such as the constructor for
// converting from a managed-pointer-of-a-different-type, are implicit and so
// no longer clearly documented in a clear place of their own.
//
// A second design decision was to implement aliasing as member-function
// templates taking references to arbitrary managed pointer types.  The other
// approach considered was to take 'ManagedPtr_Ref' objects by value, like the
// move operations.  This was ruled out due to the static-assert in the
// conversion-to-_Ref operator, that enforces a type-compatibility relationship
// between the aliased type and the managed pointer target type.  This
// restriction is not present in the current contract, and the risks opened by
// removing the static-assert and allowing general conversions everywhere (as
// undefined behavior) were seen as too large.

namespace BloombergLP {
namespace bslma {
                       // ----------------------------
                       // struct ManagedPtrNoOpDeleter
                       // ----------------------------

void ManagedPtrUtil::noOpDeleter(void *, void *)
{
}

}  // close package namespace
}  // close enterprise namespace

// ----------------------------------------------------------------------------
// Copyright (C) 2013 Bloomberg Finance L.P.
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
