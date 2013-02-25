// bsltf_uniontesttype.h                                              -*-C++-*-
#ifndef INCLUDED_BSLTF_UNIONTESTTYPE
#define INCLUDED_BSLTF_UNIONTESTTYPE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an union test type.
//
//@CLASSES:
//   bsltf::UnionTestType: union test type
//
//@SEE_ALSO: bsltf_templatetestfacility
//
//@AUTHOR: Chen He (che2)
//
//@DESCRIPTION: This component provides a union type, 'UnionTestType', that
// supports the set of operations for a single unconstrained (value-semantic)
// attribute type.  'UnionTestType' can be used during testing as the
// parameterized type of a class templates to ensure 'union' types will
// function correctly.  Note that this is particular valuable when testing a
// container template that supports different types of contained elements.
//
///Attributes
///----------
//..
//  Name                Type         Default
//  ------------------  -----------  -------
//  data                int          0
//..
//: o 'data': value of this type
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Constructing And Using The Union
///- - - - - - - - - - - - - - - - - - - - - -
// Suppose we wanted to show how objects of 'UnionTestType' can be created and
// used.
//..
// UnionTestType X;
//
// X.setData(-1);
// assert(X.data() == -1);
//
// UnionTestType Y = X;
// assert(Y == X);
//
// UnionTestType Z(X);
// assert(Z == X);
//..

#include <string.h>

namespace BloombergLP {
namespace bsltf {

                        // ===================
                        // union UnionTestType
                        // ===================

union UnionTestType {
    // This union provides a union test type to facilitate testing of
    // templates.

    // DATA
  private:
    int    d_data;    // integer data value
    char   d_char;    // the char in this union
    float  d_float;   // the float in this union
    double d_double;  // the double in this union

  public:
    // CREATORS
    UnionTestType();
        // Construct a 'UnionTestType' union having all members set to 0.

    explicit UnionTestType(int value);
        // Construct a 'UnionTestType' having the 'd_data' member set to the
        // specified 'value'.

    // MANIPULATORS
    void setData(int value);
        // Set the 'data' attribute of this object to the specified 'value'.

    // ACCESSORS
    int data() const;
        // Return the value of the 'data' attribute of this object.
};

// FREE OPERATORS
bool operator==(const UnionTestType& lhs, const UnionTestType& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' unions have the same
    // value, and 'false' otherwise.  Two 'UnionTestType' objects have the same
    // if their 'data' attributes are the same.

bool operator!=(const UnionTestType& lhs, const UnionTestType& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' objects do not have the
    // same value, and 'false' otherwise.  Two 'UnionTestType' objects do not
    // have the same value if their 'data' attributes are not the same.

// ===========================================================================
//                  INLINE AND TEMPLATE FUNCTION IMPLEMENTATIONS
// ===========================================================================

                        // -------------------
                        // class UnionTestType
                        // -------------------

// CREATORS
inline
UnionTestType::UnionTestType()
: d_data(0)
{
}

inline
UnionTestType::UnionTestType(int value)
: d_data(value)
{
}

// MANIPULATORS
inline
void UnionTestType::setData(int value)
{
    d_data = value;
}

// ACCESSORS
inline
int UnionTestType::data() const
{
    return d_data;
}

// FREE OPERATORS
inline
bool operator==(const bsltf::UnionTestType& lhs,
                const bsltf::UnionTestType& rhs)
{
    return lhs.data() == rhs.data();
}

inline
bool operator!=(const bsltf::UnionTestType& lhs,
                const bsltf::UnionTestType& rhs)
{
    return lhs.data() != rhs.data();
}

}  // close package namespace
}  // close enterprise namespace

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
