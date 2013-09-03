// bdeu_arrayutil.h                                                   -*-C++-*-
#ifndef INCLUDED_BDEU_ARRAYUTIL
#define INCLUDED_BDEU_ARRAYUTIL

#ifndef INCLUDED_BDES_IDENT
#  include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide utilities to determine properties of fixed-sized arrays.
//
//@CLASSES:
// bdeu_ArrayUtil: namespace for array access function templates.
//
//@MACROS:
// BDEU_ARRAYUTIL_SIZE: get a constant expression with an array's size. 
//
//@AUTHOR: Dietmar Kuhl (dkuhl)
//
//@DESCRIPTION: This component provides a utility 'struct',
// 'bdeu_ArrayUtil', that serves as a namespace for a collection of function
// templates providing access to the size and iterators of statically sized
// built-in arrays.
// 
// The basic idea is that the compiler knows the size of staticly sized arrays
// and the corresponding information can be exposed using simple function
// templates. The use of these function templates is easier and safer than
// the alternatives like use of 'sizeof()' (turning the array into a pointer
// doesn't cause the use of 'sizeof()' to fail at compile-time but it yields
// a wrong result) or manually specifying the size of an array.
//
///Usage Example 1
///---------------
// When creating a sequence of values it is often easy to write the sequence
// as an initialized array and use this array to initialize a container. Since
// the array's size may get adjusted in during when the program is maintained,
// the code using the array should automatically determine the array's size or
// automatically determine iterators to the beginning and the end of the
// array. For example, to initialize a 'bsl::vector<int>' with the first few
// prime numbers stored in an array the following code uses the 'begin()' and
// 'end()' methods of 'bdeu_ArrayUtil':
//..
//  const int        primes[] = { 2, 3, 5, 7, 11, 13, 17 };
//  bsl::vector<int> values(bdeu_ArrayUtil::begin(primes),
//                          bdeu_ArrayUtil::end(primes));
//
//  assert(values.size() == bdeu_ArrayUtil::size(primes));
//..
// After constructing 'values' with the content of the array 'primes' the
// assertion verifies that the correct number of values is stored in 'values'.
// When the size is needed as a constant expression, e.g., to use it for the
// size of another array, the macro 'BDEU_ARRAYUTIL_SIZE(array)' can be used:
//..
//  int reversePrimes[BDEU_ARRAYUTIL_SIZE(primes)];
//  bsl::copy(values.rbegin(), values.rend(),
//            bdeu_ArrayUtil::begin(reversePrimes));
//
//  assert(bsl::mismatch(bdeu_ArrayUtil::rbegin(primes),
//                       bdeu_ArrayUtil::rend(primes),
//                       bdeu_ArrayUtil::begin(reversePrimes)).second
//         == bdeu_ArrayUtil::end(reversePrimes));
//..
// After defining the array 'reversePrimes' with the same size as 'primes' the
// elements of 'values' are copied in reverse order into this array. The
// assertion verifies that 'reversePrimes' contains the values from 'primes'
// but in reverse order: 'bsl::mismatch()' is used with a reverse sequence of
// 'primes' by using the 'rbegin()' and 'rend()' methods for 'primes' and
// normal sequence using the 'begin()' and 'end()' methods for 'reversePrimes'.
//
///Usage Example 2
///---------------
// The functions 'begin()', 'end()', and 'size()' provided by this component
// are similar to functions provided by container. The main difference is that
// they reside in a utility component rather than being member functions.
//
// A typical use case for the 'size()' function is a function expecting a
// pointer to a sequence of keys (e.g., columns in a database) and the number
// of the keys in the sequence:
//..
//  void query(const bsl::string*  columns,
//             int                 numberOfColumns,
//             bsl::string        *result) {
//      for (int i = 0; i != numberOfColumns; ++i) {
//          result[i] = "queried " + columns[i];
//      }
//  }
//
//  void loadData(bsl::vector<bsl::string> *data) {
//      const bsl::string columns[] = { "column1", "column2", "column3" };
//      bsl::string       result[BDEU_ARRAYUTIL_SIZE(columns)];
//       query(columns, bdeu_ArrayUtil::size(columns), result);
//      data->assign(bdeu_ArrayUtil::begin(result),
//                   bdeu_ArrayUtil::end(result));
//  }
//..
// The 'loadData()' function shows how to use the different function templates.
// The array 'columns' doesn't have a size specified. It is determined from
// the number of elements it is initialized with. In this case it is easy to
// see that there are three elements but in real situations the number of
// elements can be non-trivial to get right. Also, changing the number of
// elements would make it necessary to apply the corresponding change in
// multiple places. Thus, the size is determined using 'bdeu_ArrayUtil':
//
// * The size of 'result' should match the size of 'columns'. When specifying
//   the size of an array a constant expression is necessary. In C++ 2011 the
//   function 'bdeu_ArrayUtil::size()' could return a constant expression but
//   for compilers not, yet, implementing the standard a trick needs to be
//   used (using 'sizeof' with a reference to suitably sized array of 'char').
//   This trick is packaged into the macro 'BDEU_ARRAYUTIL_SIZE()'.
// * When the size is needed in a context where a const expression isn't
//   required, e.g., when calling 'query()', the `bdeu_ArrayUtil::size()'
//   function can be used with the array.
// * The 'bdeu_ArrayUtil::begin()' and 'bdeu_ArrayUtil::end()' functions are
//   used to obtain begin and end iterators used with the vector's 'assign()'
//   function to put the 'result' obtained from the call to 'query()' into the
//   vector pointed to by 'data'.
//
// Similar needs for an array of a sequence of values frequently arise when
// using one of the database interfaces. Another common use case are test
// cases where the content of a somehow computed sequence needs to be compared
// with an expected result:
//..
//  void checkData(const bsl::vector<bsl::string>& data) {
//      const bsl::string expect[] = {
//          "queried column1", "queried column2", "queried column3"
//      };
//      assert(data.size() == bdeu_ArrayUtil::size(expect));
//      assert(bdeu_ArrayUtil::end(expect)
//             == bsl::mismatch(bdeu_ArrayUtil::begin(expect),
//                              bdeu_ArrayUtil::end(expect),
//                              data.begin()).first);
//  }
//..
// In this code the actual result in 'data' is compared to the values in the
// array 'expect':
//
// * First it is made sure that the sizes of 'data' and 'expect' are identical
//   using 'bdeu_ArrayUtil::size()'.
// * Next, the sequences are compared using the 'mismatch()' algorithm: To get
//   the begin and of the 'expect' array 'bdeu_ArrayUtil::begin()' and
//   'bdeu_ArrayUtil::end()', respectively, are used.
//..
//  void usageExample(int verbose) {
//      if (verbose) cout << "\nUsing Basic Functions"
//                        << "\n--------------" << endl;
//       bsl::vector<bsl::string> data;
//      loadData(&data);
//      checkData(data);
//  }
//..

#ifndef INCLUDED_BDESCM_VERSION
#  include <bdescm_version.h>
#endif
#ifndef INCLUDED_BSL_ITERATOR
#  include <bsl_iterator.h>
#endif
#ifndef INCLUDED_BSL_CSTDDEF
#  include <bsl_cstddef.h>
#endif

#define BDEU_ARRAYUTIL_SIZE(a) sizeof(BloombergLP::bdeu_ArrayUtil::sizer(a))
    // Return the number of elements in the passed array as a constant
    // expression.

namespace BloombergLP
{
                            // ====================
                            // struct bdeuArrayUtil
                            // ====================

struct bdeu_ArrayUtil {
    // This 'struct' provides a namespace for a collection of function
    // templates providing access to the size and iterator of statically
    // sized built-in arrays

    // CLASS METHODS

    template <class TYPE, bsl::size_t SIZE>
    static TYPE* begin(TYPE (&array)[SIZE]);
        // Return an iterator pointing to the first element of the specified
        // 'array' of template parameter 'SIZE' elements of template parameter
        // 'TYPE'.

    template <class TYPE, bsl::size_t SIZE>
    static TYPE* end(TYPE (&array)[SIZE]);
    	// Return the past-the-end iterator for the specified 'array' of
        // template parameter 'SIZE' elements of template parameter 'TYPE'.

    template <class TYPE, bsl::size_t SIZE>
    static bsl::reverse_iterator<TYPE*> rbegin(TYPE (&array)[SIZE]);
        // Return a reverse iterator pointing to the last element of the
        // specified 'array' of template parameter 'SIZE' elements of template
        // parameter 'TYPE'.

    template <class TYPE, bsl::size_t SIZE>
    static bsl::reverse_iterator<TYPE*> rend(TYPE (&array)[SIZE]);
        // Return the past-the-end reverse iterator for the specified 'array'
        // of template parameter 'SIZE' elements of template parameter 'TYPE'.

    template <class TYPE, bsl::size_t SIZE>
    static bsl::size_t size(TYPE (&array)[SIZE]);
        // Return the number of elements in the specified 'array' of 'SIZE'
        // elements of template parameter 'TYPE'. 

    template <class TYPE, bsl::size_t SIZE>
    static char (&sizer(TYPE (&array)[SIZE]))[SIZE];
        // Return a reference to an array of char with the same number of
        // elements as the specified 'array' of template parameter 'SIZE'
        // elements of template parameter 'TYPE'. This function is *not*
        // implemented and can only be used in a non-evaluated context. The
        // function is used together with 'sizeof()' to get a constant
        // expression with the 'SIZE' of the 'array'.
        //@SEE_ALSO: BDEU_ARRAYUTIL_SIZE
};

}

// ---------------------------------------------------------------------------

template <class TYPE, bsl::size_t SIZE>
TYPE* BloombergLP::bdeu_ArrayUtil::begin(TYPE (&array)[SIZE])
{
    return array;
}

template <class TYPE, bsl::size_t SIZE>
TYPE* BloombergLP::bdeu_ArrayUtil::end(TYPE (&array)[SIZE])
{
    return array + SIZE;
}

template <class TYPE, bsl::size_t SIZE>
bsl::reverse_iterator<TYPE*>
BloombergLP::bdeu_ArrayUtil::rbegin(TYPE (&array)[SIZE])
{
    return bsl::reverse_iterator<TYPE*>(end(array));
}

template <class TYPE, bsl::size_t SIZE>
bsl::reverse_iterator<TYPE*>
BloombergLP::bdeu_ArrayUtil::rend(TYPE (&array)[SIZE])
{
    return bsl::reverse_iterator<TYPE*>(begin(array));
}

template <class TYPE, bsl::size_t SIZE>
bsl::size_t BloombergLP::bdeu_ArrayUtil::size(TYPE (&)[SIZE])
{
    return SIZE;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
#endif
