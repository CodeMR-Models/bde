// bslalg_scalarprimitives.h                                          -*-C++-*-
#ifndef INCLUDED_BSLALG_SCALARPRIMITIVES
#define INCLUDED_BSLALG_SCALARPRIMITIVES

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Primitive algorithms that operate on single elements.
//
//@CLASSES:
//  bslalg::ScalarPrimitives: namespace for algorithms
//
//@SEE_ALSO: bslalg_constructorproxy
//
//@AUTHOR: Pablo Halpern (phalpern), Herve Bronnimann (hbronnim)
//
//@DESCRIPTION: This component provides primitive algorithms that operate on
// single elements with a uniform interface but select a different
// implementation according to the various 'bslalg' type traits possessed by
// the underlying type.  Such primitives are exceptionally useful for
// implementing generic components such as containers.  There are six families
// of algorithms, each with a collection of overloads:
//..
//  Algorithm           Forwards to (depending on traits)
//  ----------------    ------------------------------------------------
//  defaultConstruct    Default constructor, with or without allocator
//
//  copyConstruct       Copy constructor, with or without allocator,
//                        or bitwise copy if appropriate
//
//  destructiveMove     Copy construction followed by destruction of the
//                        original, with or without allocator,
//                        or bitwise copy if appropriate
//
//  construct           In-place construction (using variadic template
//                        arguments, simulated by overloads), with
//                        or without allocator
//
//  swap                Three way assignment, or bitwise swap
//..
// The traits under consideration by this component are:
//..
//  Trait                                         Description
//  --------------------------------------------  -----------------------------
//  bsl::is_trivially_default_constructible       "TYPE has the trivial default
//                                                constructor trait", or
//                                                "TYPE has a trivial default
//                                                constructor"
//
//  bslma::UsesBslmaAllocator                     "the 'TYPE' constructor takes
//                                                an allocator argument", or
//                                                "'TYPE' supports 'bslma'
//                                                allocators"
//
//  bsl::is_trivially_copyable                    "TYPE has the bit-wise
//                                                copyable trait", or
//                                                "TYPE is bit-wise copyable"
//                                                (implies that it has a
//                                                trivial destructor too)
//
//  bslmf::IsBitwiseMoveable                      "TYPE has the bit-wise
//                                                moveable trait", or
//                                                "TYPE is bit-wise moveable"
//
//  bslmf::IsPair                                 "TYPE has the pair trait"
//                                                or "TYPE is a pair"
//..
///Usage
///-----
// This component is for use primarily by the 'bslstl' package.  Other clients
// should use the STL algorithms provided in '<algorithm>' and '<memory>'.

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLALG_AUTOSCALARDESTRUCTOR
#include <bslalg_autoscalardestructor.h>
#endif

#ifndef INCLUDED_BSLALG_SCALARDESTRUCTIONPRIMITIVES
#include <bslalg_scalardestructionprimitives.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSLMF_ISSAME
#include <bslmf_issame.h>
#endif

#ifndef INCLUDED_BSLMF_REMOVECVQ
#include <bslmf_removecvq.h>
#endif

#include <bslmf_istriviallydefaultconstructible.h>
#include <bslma_usesbslmaallocator.h>
#include <bslmf_istriviallycopyable.h>
#include <bslmf_isbitwisemoveable.h>
#include <bslmf_ispair.h>

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_UTIL
#include <bsls_util.h>
#endif

#ifndef INCLUDED_NEW
#include <new>          // placement 'new'
#define INCLUDED_NEW
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif

#ifndef INCLUDED_CSTRING
#include <cstring>
#define INCLUDED_CSTRING
#endif

namespace BloombergLP {

namespace bslalg {

struct ScalarPrimitives_Imp;

                       // =======================
                       // struct ScalarPrimitives
                       // =======================

struct ScalarPrimitives {
    // This 'struct' provides a namespace for a suite of utility functions that
    // operate on elements of a parameterized type 'TARGET_TYPE'.  If any of
    // the '...Construct' methods throws, then its target 'address' is left
    // uninitialized and there are no effects, unless otherwise mentioned in
    // the documentation.

  private:
    // PRIVATE TYPES
    typedef ScalarPrimitives_Imp Imp;

  public:
    // CLASS METHODS
    template <typename TARGET_TYPE>
    static void defaultConstruct(TARGET_TYPE      *address,
                                 bslma::Allocator *allocator);
    template <typename TARGET_TYPE>
    static void defaultConstruct(TARGET_TYPE *address,
                                 void        *allocator);
        // Build a default-initialized object of the parameterized
        // 'TARGET_TYPE' in the uninitialized memory at the specified
        // 'address', as if by using the 'TARGET_TYPE' default constructor.  If
        // the specified 'allocator' is based on 'bslma::Allocator' and
        // 'TARGET_TYPE' takes an allocator constructor argument, then
        // 'allocator' is passed to the default constructor.  If the
        // constructor throws, the 'address' is left in an uninitialized state.
        // Note that this operation may bypass the constructor and simply fill
        // memory with 0 if 'TARGET_TYPE' has the trivial default constructor
        // trait and does not use 'bslma::Allocator'.

    template <typename TARGET_TYPE>
    static void copyConstruct(TARGET_TYPE        *address,
                              const TARGET_TYPE&  original,
                              bslma::Allocator   *allocator);
    template <typename TARGET_TYPE>
    static void copyConstruct(TARGET_TYPE        *address,
                              const TARGET_TYPE&  original,
                              void               *allocator);
        // Build an object of the parameterized 'TARGET_TYPE' from the
        // specified 'original' object of the same 'TARGET_TYPE' in the
        // uninitialized memory at the specified 'address', as if by using the
        // copy constructor of 'TARGET_TYPE'.  If the specified 'allocator' is
        // based on 'bslma::Allocator' and 'TARGET_TYPE' takes an allocator
        // constructor argument, then 'allocator' is passed to the copy
        // constructor.  If the constructor throws, the 'address' is left in an
        // uninitialized state.  Note that bit-wise copy will be used if
        // 'TARGET_TYPE' has the bit-wise copyable trait.

    template <typename TARGET_TYPE, typename ALLOCATOR>
    static void destructiveMove(TARGET_TYPE *address,
                                TARGET_TYPE *original,
                                ALLOCATOR   *allocator);
        // Move the state of the object of the parameterized 'TARGET_TYPE' from
        // the object at the specified 'original' address to the uninitialized
        // memory at the specified 'address', as if by constructing a copy and
        // then destroying the original.  If the parameterized 'ALLOCATOR' is
        // based on 'bslma::Allocator' and 'TARGET_TYPE' takes an allocator
        // constructor argument, then the moved object uses the specified
        // 'allocator' to supply memory.  If the move constructor throws, the
        // 'address' is left in an uninitialized state.  The behavior is
        // undefined unless the 'original' object also uses the 'allocator'.
        // Note that bit-wise copy will be used if TARGET_TYPE' has the
        // bit-wise moveable trait.

    template <typename TARGET_TYPE, typename ARG1>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE, typename ARG1>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          void        *allocator);
    template <typename TARGET_TYPE, typename ARG1, typename ARG2>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE, typename ARG1, typename ARG2>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          const ARG5&  a5,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          const ARG5&  a5,
                          const ARG6&  a6,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          const ARG5&  a5,
                          const ARG6&  a6,
                          const ARG7&  a7,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7, typename ARG8>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          const ARG5&  a5,
                          const ARG6&  a6,
                          const ARG7&  a7,
                          const ARG8&  a8,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9>
    static void construct(TARGET_TYPE *address,
                          const ARG1&  a1,
                          const ARG2&  a2,
                          const ARG3&  a3,
                          const ARG4&  a4,
                          const ARG5&  a5,
                          const ARG6&  a6,
                          const ARG7&  a7,
                          const ARG8&  a8,
                          const ARG9&  a9,
                          void        *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7, typename ARG8,
              typename ARG9, typename ARG10>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          const ARG10&      a10,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7, typename ARG8,
              typename ARG9, typename ARG10>
    static void construct(TARGET_TYPE  *address,
                          const ARG1&   a1,
                          const ARG2&   a2,
                          const ARG3&   a3,
                          const ARG4&   a4,
                          const ARG5&   a5,
                          const ARG6&   a6,
                          const ARG7&   a7,
                          const ARG8&   a8,
                          const ARG9&   a9,
                          const ARG10&  a10,
                          void         *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10, typename ARG11>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          const ARG10&      a10,
                          const ARG11&      a11,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10, typename ARG11>
    static void construct(TARGET_TYPE  *address,
                          const ARG1&   a1,
                          const ARG2&   a2,
                          const ARG3&   a3,
                          const ARG4&   a4,
                          const ARG5&   a5,
                          const ARG6&   a6,
                          const ARG7&   a7,
                          const ARG8&   a8,
                          const ARG9&   a9,
                          const ARG10&  a10,
                          const ARG11&  a11,
                          void         *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          const ARG10&      a10,
                          const ARG11&      a11,
                          const ARG12&      a12,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12>
    static void construct(TARGET_TYPE  *address,
                          const ARG1&   a1,
                          const ARG2&   a2,
                          const ARG3&   a3,
                          const ARG4&   a4,
                          const ARG5&   a5,
                          const ARG6&   a6,
                          const ARG7&   a7,
                          const ARG8&   a8,
                          const ARG9&   a9,
                          const ARG10&  a10,
                          const ARG11&  a11,
                          const ARG12&  a12,
                          void         *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12,
              typename ARG13>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          const ARG10&      a10,
                          const ARG11&      a11,
                          const ARG12&      a12,
                          const ARG13&      a13,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12,
              typename ARG13>
    static void construct(TARGET_TYPE  *address,
                          const ARG1&   a1,
                          const ARG2&   a2,
                          const ARG3&   a3,
                          const ARG4&   a4,
                          const ARG5&   a5,
                          const ARG6&   a6,
                          const ARG7&   a7,
                          const ARG8&   a8,
                          const ARG9&   a9,
                          const ARG10&  a10,
                          const ARG11&  a11,
                          const ARG12&  a12,
                          const ARG13&  a13,
                          void         *allocator);
    template <typename TARGET_TYPE,
              typename ARG1,  typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5,  typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9,  typename ARG10, typename ARG11, typename ARG12,
              typename ARG13, typename ARG14>
    static void construct(TARGET_TYPE      *address,
                          const ARG1&       a1,
                          const ARG2&       a2,
                          const ARG3&       a3,
                          const ARG4&       a4,
                          const ARG5&       a5,
                          const ARG6&       a6,
                          const ARG7&       a7,
                          const ARG8&       a8,
                          const ARG9&       a9,
                          const ARG10&      a10,
                          const ARG11&      a11,
                          const ARG12&      a12,
                          const ARG13&      a13,
                          const ARG14&      a14,
                          bslma::Allocator *allocator);
    template <typename TARGET_TYPE,
              typename ARG1,  typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5,  typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9,  typename ARG10, typename ARG11, typename ARG12,
              typename ARG13, typename ARG14>
    static void construct(TARGET_TYPE  *address,
                          const ARG1&   a1,
                          const ARG2&   a2,
                          const ARG3&   a3,
                          const ARG4&   a4,
                          const ARG5&   a5,
                          const ARG6&   a6,
                          const ARG7&   a7,
                          const ARG8&   a8,
                          const ARG9&   a9,
                          const ARG10&  a10,
                          const ARG11&  a11,
                          const ARG12&  a12,
                          const ARG13&  a13,
                          const ARG14&  a14,
                          void         *allocator);
        // Build an object of the parameterized 'TARGET_TYPE' in the
        // uninitialized memory at the specified 'address'.  Use the
        // 'TARGET_TYPE' constructor 'TARGET_TYPE(ARG1 const&, ...)' taking the
        // specified 'a1' up to 'a14' arguments of the respective parameterized
        // 'ARG1' up to 'ARG14' types.  If 'allocator' is based on
        // 'bslma::Allocator' and 'TARGET_TYPE' takes an allocator constructor
        // argument, then 'allocator' is passed to the 'TARGET_TYPE'
        // constructor in the last position.

#if !defined(BSL_LEGACY) || 1 == BSL_LEGACY
    template <typename TARGET_TYPE>
    static void destruct(TARGET_TYPE *object,
                         void        *allocator);
        // DEPRECATED.
        // Use 'ScalarDestructionPrimitives::destroy' without an
        // allocator argument.

    template <typename TARGET_TYPE>
    static void destruct(TARGET_TYPE *object);
        // DEPRECATED.
        // Use 'ScalarDestructionPrimitives::destroy' instead.
#endif

    template <typename LHS_TYPE, typename RHS_TYPE>
    static void swap(LHS_TYPE& lhs, RHS_TYPE& rhs);
        // Swap the contents of the specified 'lhs' modifiable reference of the
        // parameterized 'LHS_TYPE' with those of the specified 'rhs'
        // modifiable reference of the parameterized 'RHS_TYPE'.  Note that, if
        // 'LHS_TYPE' and 'RHS_TYPE' are the same type and that type has the
        // bit-wise moveable trait but does not use 'bslma' allocators, the
        // swap can be performed using a three-way bit-wise move.
};

                     // ===========================
                     // struct ScalarPrimitives_Imp
                     // ===========================

struct ScalarPrimitives_Imp {
    // This 'struct' provides a namespace for a suite of utility functions that
    // operate on arrays of elements of a parameterized type 'TARGET_TYPE'.
    // These utility functions are only for the purpose of implementing those
    // in the 'ScalarPrimitives' utility, and should not be used outside
    // this component.

    enum {
        // These constants are used in the overloads below, when the last
        // argument is of type 'bslmf::MetaInt<N> *', indicating that
        // 'TARGET_TYPE' has the traits for which the enumerator equal to 'N'
        // is named.

        USES_BSLMA_ALLOCATOR_TRAITS     = 5,
        PAIR_TRAITS                     = 4,
        HAS_TRIVIAL_DEFAULT_CTOR_TRAITS = 3,
        BITWISE_COPYABLE_TRAITS         = 2,
        BITWISE_MOVEABLE_TRAITS         = 1,
        NIL_TRAITS                      = 0
    };

    // CLASS METHODS
    template <typename TARGET_TYPE>
    static TARGET_TYPE *unconst(const TARGET_TYPE *pointer);
        // Return the 'const'-unqualified value of the specified 'pointer'.
        // This function resolves into a 'const_cast' and therefore has no
        // runtime cost, it exists only for template argument deduction.

    template <typename TARGET_TYPE>
    static void defaultConstruct(
                        TARGET_TYPE                                 *address,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void defaultConstruct(
                    TARGET_TYPE                                     *address,
                    bslma::Allocator                                *allocator,
                    bslmf::MetaInt<HAS_TRIVIAL_DEFAULT_CTOR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void defaultConstruct(TARGET_TYPE                 *address,
                                 bslma::Allocator            *allocator,
                                 bslmf::MetaInt<PAIR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void defaultConstruct(TARGET_TYPE                *address,
                                 bslma::Allocator           *allocator,
                                 bslmf::MetaInt<NIL_TRAITS> *);
        // Build a 'TARGET_TYPE' object in a default state in the uninitialized
        // memory at the specified 'address', using the specified 'allocator'
        // to supply memory if 'TARGET_TYPE' uses 'bslma' allocators.  Use the
        // default constructor of the parameterized 'TARGET_TYPE', or 'memset'
        // to 0 if 'TARGET_TYPE' has a trivial default constructor.  The last
        // argument is for traits overloading resolution only and its value is
        // ignored.

    template <typename TARGET_TYPE>
    static void defaultConstruct(
                      TARGET_TYPE                                     *address,
                      bslmf::MetaInt<HAS_TRIVIAL_DEFAULT_CTOR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void defaultConstruct(TARGET_TYPE                *address,
                                 bslmf::MetaInt<NIL_TRAITS> *);
        // Build a 'TARGET_TYPE' object in a default state in the uninitialized
        // memory at the specified 'address'.  Use the default constructor of
        // the parameterized 'TARGET_TYPE', or 'memset' to 0 if 'TARGET_TYPE'
        // has a trivial default constructor.  The last argument is for traits
        // overloading resolution only and its value is ignored.

    template <typename TARGET_TYPE>
    static void copyConstruct(
                        TARGET_TYPE                                 *address,
                        const TARGET_TYPE&                           original,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void copyConstruct(TARGET_TYPE                 *address,
                              const TARGET_TYPE&           original,
                              bslma::Allocator            *allocator,
                              bslmf::MetaInt<PAIR_TRAITS> *);
    template <typename TARGET_TYPE>
    static void copyConstruct(
                            TARGET_TYPE                             *address,
                            const TARGET_TYPE&                       original,
                            bslma::Allocator                        *allocator,
                            bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *);
    template <typename TARGET_TYPE>
    static void copyConstruct(TARGET_TYPE                *address,
                              const TARGET_TYPE&          original,
                              bslma::Allocator           *allocator,
                              bslmf::MetaInt<NIL_TRAITS> *);
        // Build in the uninitialized memory at the specified 'address' an
        // object of the parameterized 'TARGET_TYPE' that is a copy of the
        // specified 'original' object of the same 'TARGET_TYPE', using the
        // specified 'allocator' to supply memory.  Use the copy constructor of
        // the 'TARGET_TYPE', or a bit-wise copy if 'TARGET_TYPE' is a bit-wise
        // copyable type.  The last argument is for traits overloading
        // resolution only and its value is ignored.  Note that a bit-wise copy
        // is only appropriate if 'TARGET_TYPE' does not take allocators.

    template <typename TARGET_TYPE>
    static void copyConstruct(
                             TARGET_TYPE                             *address,
                             const TARGET_TYPE&                       original,
                             bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *);
    template <typename TARGET_TYPE>
    static void copyConstruct(TARGET_TYPE                *address,
                              const TARGET_TYPE&          original,
                              bslmf::MetaInt<NIL_TRAITS> *);
        // Build in the uninitialized memory at the specified 'address' an
        // object of the parameterized 'TARGET_TYPE' that is a copy of the
        // specified 'original' object of the same 'TARGET_TYPE'.  Use the copy
        // constructor of the 'TARGET_TYPE', or a bit-wise copy if
        // 'TARGET_TYPE' is a bit-wise copyable type.  The last argument is for
        // traits overloading resolution only and its value is ignored.  Note
        // that a bit-wise copy is only appropriate if 'TARGET_TYPE' does not
        // take allocators.

    template <typename TARGET_TYPE, typename ALLOCATOR>
    static void destructiveMove(
                            TARGET_TYPE                             *address,
                            TARGET_TYPE                             *original,
                            ALLOCATOR                               *allocator,
                            bslmf::MetaInt<BITWISE_MOVEABLE_TRAITS> *);
    template <typename TARGET_TYPE, typename ALLOCATOR>
    static void destructiveMove(TARGET_TYPE                *address,
                                TARGET_TYPE                *original,
                                ALLOCATOR                  *allocator,
                                bslmf::MetaInt<NIL_TRAITS> *);
        // Build a copy of the specified 'original' in the uninitialized memory
        // at the specified 'address'.  Use the copy constructor of the
        // parameterized 'TARGET_TYPE' (or a bit-wise copy if 'TARGET_TYPE' is
        // bit-wise moveable).  If 'TARGET_TYPE' is not bit-wise moveable, also
        // destroy the 'original'.  The last argument is for traits overloading
        // resolution only and its value is ignored.

    template <typename TARGET_TYPE, typename ARG1>
    static void construct(TARGET_TYPE                             *address,
                          const ARG1&                              a1,
                          bslma::Allocator                        *allocator,
                          bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *);
        // Build an object from the specified 'a1' in the uninitialized memory
        // at the specified 'address'.  The specified 'allocator' is ignored.
        // Use the parameterized 'TARGET_TYPE' constructor with the signature
        // 'TARGET_TYPE(ARG1 const&)' or bitwise copy for non-fundamental
        // types.  The traits argument is for overloading resolution only and
        // is ignored.

    template <typename TARGET_TYPE, typename ARG1>
    static void construct(TARGET_TYPE                 *address,
                          const ARG1&                  a1,
                          bslma::Allocator            *allocator,
                          bslmf::MetaInt<PAIR_TRAITS> *);
    template <typename TARGET_TYPE, typename ARG1, typename ARG2>
    static void construct(TARGET_TYPE                 *address,
                          const ARG1&                  a1,
                          const ARG2&                  a2,
                          bslma::Allocator            *allocator,
                          bslmf::MetaInt<PAIR_TRAITS> *);
        // Build an object from the specified 'a1' (and optionally 'a2') in the
        // uninitialized memory at the specified 'address'.  The specified
        // 'allocator' is passed through to the 'first_type' and 'second_type'
        // members of 'TARGET_TYPE'.  Use the parameterized 'TARGET_TYPE'
        // constructor with the signature 'TARGET_TYPE(ARG1 const&, ...)'.  The
        // traits argument is for overloading resolution only and is ignored.
        // Note that because pair types have at most two constructor arguments,
        // only two versions of this function are needed.

    template <typename TARGET_TYPE, typename ARG1>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE, typename ARG1, typename ARG2>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10, typename ARG11>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12,
              typename ARG13>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        const ARG13&                                 a13,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1,  typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5,  typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9,  typename ARG10, typename ARG11, typename ARG12,
              typename ARG13, typename ARG14>
    static void construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        const ARG13&                                 a13,
                        const ARG14&                                 a14,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *);
        // Build an object of the parameterized 'TARGET_TYPE' in the
        // uninitialized memory at the specified 'address', passing the
        // specified 'a1' up to 'a14' arguments of the corresponding
        // parameterized 'ARG1' up to 'ARG14' types to the 'TARGET_TYPE'
        // constructor with the signature
        // 'TARGET_TYPE(ARG1 const&, ..., bslma::Allocator *)', and pass the
        // specified 'allocator' in the last position.  The last argument is
        // for overloading resolution only and its value is ignored.

    template <typename TARGET_TYPE, typename ARG1>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE, typename ARG1, typename ARG2>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2, typename ARG3, typename ARG4,
              typename ARG5, typename ARG6, typename ARG7>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          const ARG10&                a10,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3, typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7, typename ARG8,
              typename ARG9, typename ARG10, typename ARG11>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          const ARG10&                a10,
                          const ARG11&                a11,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          const ARG10&                a10,
                          const ARG11&                a11,
                          const ARG12&                a12,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1, typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5, typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9, typename ARG10, typename ARG11, typename ARG12,
              typename ARG13>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          const ARG10&                a10,
                          const ARG11&                a11,
                          const ARG12&                a12,
                          const ARG13&                a13,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
    template <typename TARGET_TYPE,
              typename ARG1,  typename ARG2,  typename ARG3,  typename ARG4,
              typename ARG5,  typename ARG6,  typename ARG7,  typename ARG8,
              typename ARG9,  typename ARG10, typename ARG11, typename ARG12,
              typename ARG13, typename ARG14>
    static void construct(TARGET_TYPE                *address,
                          const ARG1&                 a1,
                          const ARG2&                 a2,
                          const ARG3&                 a3,
                          const ARG4&                 a4,
                          const ARG5&                 a5,
                          const ARG6&                 a6,
                          const ARG7&                 a7,
                          const ARG8&                 a8,
                          const ARG9&                 a9,
                          const ARG10&                a10,
                          const ARG11&                a11,
                          const ARG12&                a12,
                          const ARG13&                a13,
                          const ARG14&                a14,
                          bslma::Allocator           *allocator,
                          bslmf::MetaInt<NIL_TRAITS> *);
        // Build an object of the parameterized 'TARGET_TYPE' in the
        // uninitialized memory at the specified 'address', passing the
        // specified 'a1' up to 'a14' arguments of the corresponding
        // parameterized 'ARG1' up to 'ARG14' types to the 'TARGET_TYPE'
        // constructor with the signature 'TARGET_TYPE(ARG1 const&, ...)'.  The
        // specified 'allocator' is *not* passed through to the 'TARGET_TYPE'
        // constructor.  The last argument is for overloading resolution only
        // and is ignored.

    template <typename LHS_TYPE, typename RHS_TYPE>
    static void swap(LHS_TYPE&                                lhs,
                     RHS_TYPE&                                rhs,
                     bslmf::MetaInt<BITWISE_MOVEABLE_TRAITS> *);
    template <typename LHS_TYPE, typename RHS_TYPE>
    static void swap(LHS_TYPE&                   lhs,
                     RHS_TYPE&                   rhs,
                     bslmf::MetaInt<NIL_TRAITS> *);
        // Swap the contents of the specified 'lhs' object of the parameterized
        // 'LHS_TYPE' with the specified 'rhs' object of the parameterized
        // 'RHS_TYPE'.  Use a three-way bit-wise copy (with a temporary
        // uninitialized buffer), if 'LHS_TYPE' and 'RHS_TYPE' are the same
        // bit-wise moveable type, and a three-way assignment with a temporary
        // if not.  The last argument is for overloading resolution only and is
        // ignored.
};

// ===========================================================================
//                      TEMPLATE FUNCTION DEFINITIONS
// ===========================================================================

                       // -----------------------
                       // struct ScalarPrimitives
                       // -----------------------

                    // *** defaultConstruct overloads: ***

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives::defaultConstruct(TARGET_TYPE      *address,
                                   bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : bsl::is_trivially_default_constructible<TARGET_TYPE>::value
                  ? Imp::HAS_TRIVIAL_DEFAULT_CTOR_TRAITS
                  : bslmf::IsPair<TARGET_TYPE>::value
                      ? Imp::PAIR_TRAITS
                      : Imp::NIL_TRAITS
    };
    Imp::defaultConstruct(address, allocator, (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives::defaultConstruct(TARGET_TYPE *address,
                                   void        *)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bsl::is_trivially_default_constructible<TARGET_TYPE>::value
              ? Imp::HAS_TRIVIAL_DEFAULT_CTOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::defaultConstruct(address, (bslmf::MetaInt<VALUE>*)0);
}

                      // *** copyConstruct overloads: ***

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives::copyConstruct(TARGET_TYPE        *address,
                                const TARGET_TYPE&  original,
                                bslma::Allocator   *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : bsl::is_trivially_copyable<TARGET_TYPE>::value
                  ? Imp::BITWISE_COPYABLE_TRAITS
                  : bslmf::IsPair<TARGET_TYPE>::value
                      ? Imp::PAIR_TRAITS
                      : Imp::NIL_TRAITS
    };
    Imp::copyConstruct(address, original, allocator,
                       (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives::copyConstruct(TARGET_TYPE        *address,
                                const TARGET_TYPE&  original,
                                void               *)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bsl::is_trivially_copyable<TARGET_TYPE>::value
              ? Imp::BITWISE_COPYABLE_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::copyConstruct(address, original, (bslmf::MetaInt<VALUE>*)0);
}

                     // *** destructiveMove overloads: ***

template <typename TARGET_TYPE, typename ALLOCATOR>
inline
void
ScalarPrimitives::destructiveMove(TARGET_TYPE *address,
                                  TARGET_TYPE *original,
                                  ALLOCATOR   *allocator)
{
    BSLS_ASSERT_SAFE(address);
    BSLS_ASSERT_SAFE(original);

    enum {
        VALUE = bslmf::IsBitwiseMoveable<TARGET_TYPE>::value
              ? Imp::BITWISE_MOVEABLE_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::destructiveMove(address,
                         original,
                         allocator,
                         (bslmf::MetaInt<VALUE>*)0);
}

                       // *** construct overloads: ****

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : bslmf::IsSame<ARG1, TARGET_TYPE>::value
                && bsl::is_trivially_copyable<TARGET_TYPE>::value
                  ? Imp::BITWISE_COPYABLE_TRAITS
                  : bslmf::IsPair<TARGET_TYPE>::value
                      ? Imp::PAIR_TRAITS
                      : Imp::NIL_TRAITS
    };
    Imp::construct(address, a1, allocator, (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : bslmf::IsPair<TARGET_TYPE>::value
                  ? Imp::PAIR_TRAITS
                  : Imp::NIL_TRAITS
    };
    Imp::construct(address, a1, a2, allocator, (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address, a1, a2, a3, allocator, (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            const ARG5&  a5,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            const ARG5&  a5,
                            const ARG6&  a6,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            const ARG5&  a5,
                            const ARG6&  a6,
                            const ARG7&  a7,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            const ARG5&  a5,
                            const ARG6&  a6,
                            const ARG7&  a7,
                            const ARG8&  a8,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9>
inline
void
ScalarPrimitives::construct(TARGET_TYPE *address,
                            const ARG1&  a1,
                            const ARG2&  a2,
                            const ARG3&  a3,
                            const ARG4&  a4,
                            const ARG5&  a5,
                            const ARG6&  a6,
                            const ARG7&  a7,
                            const ARG8&  a8,
                            const ARG9&  a9,
                            void        *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            const ARG10&      a10,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10>
inline
void
ScalarPrimitives::construct(TARGET_TYPE  *address,
                            const ARG1&   a1,
                            const ARG2&   a2,
                            const ARG3&   a3,
                            const ARG4&   a4,
                            const ARG5&   a5,
                            const ARG6&   a6,
                            const ARG7&   a7,
                            const ARG8&   a8,
                            const ARG9&   a9,
                            const ARG10&  a10,
                            void         *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            const ARG10&      a10,
                            const ARG11&      a11,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11>
inline
void
ScalarPrimitives::construct(TARGET_TYPE  *address,
                            const ARG1&   a1,
                            const ARG2&   a2,
                            const ARG3&   a3,
                            const ARG4&   a4,
                            const ARG5&   a5,
                            const ARG6&   a6,
                            const ARG7&   a7,
                            const ARG8&   a8,
                            const ARG9&   a9,
                            const ARG10&  a10,
                            const ARG11&  a11,
                            void         *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11,
          typename ARG12>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            const ARG10&      a10,
                            const ARG11&      a11,
                            const ARG12&      a12,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11,
          typename ARG12>
inline
void
ScalarPrimitives::construct(TARGET_TYPE  *address,
                            const ARG1&   a1,
                            const ARG2&   a2,
                            const ARG3&   a3,
                            const ARG4&   a4,
                            const ARG5&   a5,
                            const ARG6&   a6,
                            const ARG7&   a7,
                            const ARG8&   a8,
                            const ARG9&   a9,
                            const ARG10&  a10,
                            const ARG11&  a11,
                            const ARG12&  a12,
                            void         *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(a1,  a2,  a3, a4, a5, a6, a7, a8, a9,
                                a10, a11, a12);
}

template <typename TARGET_TYPE,  typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9, typename ARG10, typename ARG11,
          typename ARG12, typename ARG13>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            const ARG10&      a10,
                            const ARG11&      a11,
                            const ARG12&      a12,
                            const ARG13&      a13,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE,  typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9, typename ARG10, typename ARG11,
          typename ARG12, typename ARG13>
inline
void
ScalarPrimitives::construct(TARGET_TYPE  *address,
                            const ARG1&   a1,
                            const ARG2&   a2,
                            const ARG3&   a3,
                            const ARG4&   a4,
                            const ARG5&   a5,
                            const ARG6&   a6,
                            const ARG7&   a7,
                            const ARG8&   a8,
                            const ARG9&   a9,
                            const ARG10&  a10,
                            const ARG11&  a11,
                            const ARG12&  a12,
                            const ARG13&  a13,
                            void         *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(
                       a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <typename TARGET_TYPE,  typename ARG1,  typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5,  typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9,  typename ARG10, typename ARG11,
          typename ARG12, typename ARG13, typename ARG14>
inline
void
ScalarPrimitives::construct(TARGET_TYPE      *address,
                            const ARG1&       a1,
                            const ARG2&       a2,
                            const ARG3&       a3,
                            const ARG4&       a4,
                            const ARG5&       a5,
                            const ARG6&       a6,
                            const ARG7&       a7,
                            const ARG8&       a8,
                            const ARG9&       a9,
                            const ARG10&      a10,
                            const ARG11&      a11,
                            const ARG12&      a12,
                            const ARG13&      a13,
                            const ARG14&      a14,
                            bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(address);

    enum {
        VALUE = bslma::UsesBslmaAllocator<TARGET_TYPE>::value
              ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::construct(address,
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14,
                   allocator,
                   (bslmf::MetaInt<VALUE>*)0);
}

template <typename TARGET_TYPE,  typename ARG1,  typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5,  typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9,  typename ARG10, typename ARG11,
          typename ARG12, typename ARG13, typename ARG14>
inline
void
ScalarPrimitives::construct(TARGET_TYPE  *address,
                            const ARG1&   a1,
                            const ARG2&   a2,
                            const ARG3&   a3,
                            const ARG4&   a4,
                            const ARG5&   a5,
                            const ARG6&   a6,
                            const ARG7&   a7,
                            const ARG8&   a8,
                            const ARG9&   a9,
                            const ARG10&  a10,
                            const ARG11&  a11,
                            const ARG12&  a12,
                            const ARG13&  a13,
                            const ARG14&  a14,
                            void         *)
{
    BSLS_ASSERT_SAFE(address);

    ::new (address) TARGET_TYPE(
                  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

}  // close package namespace

                          // *** destruct overloads: ***

#if !defined(BSL_LEGACY) || 1 == BSL_LEGACY

namespace bslalg {

template <typename TARGET_TYPE>
inline
void ScalarPrimitives::destruct(TARGET_TYPE *address, void *)
{
    ScalarDestructionPrimitives::destroy(address);
}

template <typename TARGET_TYPE>
inline
void ScalarPrimitives::destruct(TARGET_TYPE *address)
{
    ScalarDestructionPrimitives::destroy(address);
}

}  // close package namespace

#endif

namespace bslalg {

                          // *** swap overloads: ***

template <typename LHS_TYPE, typename RHS_TYPE>
void ScalarPrimitives::swap(LHS_TYPE& lhs, RHS_TYPE& rhs)
{
    enum {
        VALUE = bslmf::IsSame<LHS_TYPE, RHS_TYPE>::value
                && bslmf::IsBitwiseMoveable<LHS_TYPE>::value
              ? Imp::BITWISE_MOVEABLE_TRAITS
              : Imp::NIL_TRAITS
    };
    Imp::swap(lhs, rhs, (bslmf::MetaInt<VALUE>*)0);
}

                     // ---------------------------
                     // struct ScalarPrimitives_Imp
                     // ---------------------------

// CLASS METHODS
template <typename TARGET_TYPE>
inline
TARGET_TYPE *ScalarPrimitives_Imp::unconst(const TARGET_TYPE *pointer)
{
    return const_cast<TARGET_TYPE *>(pointer);
}

                    // *** defaultConstruct overloads: ***

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(
                        TARGET_TYPE                                 *address,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(allocator);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(
                      TARGET_TYPE                                     *address,
                      bslma::Allocator                                *,
                      bslmf::MetaInt<HAS_TRIVIAL_DEFAULT_CTOR_TRAITS> *)
{
    defaultConstruct(address,
                     (bslmf::MetaInt<HAS_TRIVIAL_DEFAULT_CTOR_TRAITS>*)0);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(TARGET_TYPE                 *address,
                                       bslma::Allocator            *allocator,
                                       bslmf::MetaInt<PAIR_TRAITS> *)
{
    ScalarPrimitives::defaultConstruct(
                                  unconst(BSLS_UTIL_ADDRESSOF(address->first)),
                                  allocator);
    AutoScalarDestructor<typename bslmf::RemoveCvq<
                                typename TARGET_TYPE::first_type>::Type>
                           guard(unconst(BSLS_UTIL_ADDRESSOF(address->first)));
    ScalarPrimitives::defaultConstruct(
                                 unconst(BSLS_UTIL_ADDRESSOF(address->second)),
                                 allocator);
    guard.release();
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(TARGET_TYPE                *address,
                                       bslma::Allocator           *,
                                       bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE();
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(
                      TARGET_TYPE                                     *address,
                      bslmf::MetaInt<HAS_TRIVIAL_DEFAULT_CTOR_TRAITS> *)
{
    if (bslmf::IsFundamental<TARGET_TYPE>::value
     || bslmf::IsPointer<TARGET_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental or pointer types.  Note that
        // assignment can't throw.

        ::new (address) TARGET_TYPE();
    } else {
        std::memset((char *)address, 0, sizeof *address);
    }
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::defaultConstruct(TARGET_TYPE                *address,
                                       bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE();
}

                      // *** copyConstruct overloads: ***

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(
                        TARGET_TYPE                                 *address,
                        const TARGET_TYPE&                           original,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(original, allocator);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(TARGET_TYPE                 *address,
                                    const TARGET_TYPE&           original,
                                    bslma::Allocator            *allocator,
                                    bslmf::MetaInt<PAIR_TRAITS> *)
{
    ScalarPrimitives::copyConstruct(
                                  unconst(BSLS_UTIL_ADDRESSOF(address->first)),
                                  original.first,
                                  allocator);
    AutoScalarDestructor<typename bslmf::RemoveCvq<
                                typename TARGET_TYPE::first_type>::Type>
                           guard(unconst(BSLS_UTIL_ADDRESSOF(address->first)));
    ScalarPrimitives::copyConstruct(
                                 unconst(BSLS_UTIL_ADDRESSOF(address->second)),
                                 original.second,
                                 allocator);
    guard.release();
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(
                             TARGET_TYPE                             *address,
                             const TARGET_TYPE&                       original,
                             bslma::Allocator                        *,
                             bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *)
{
    if (bslmf::IsFundamental<TARGET_TYPE>::value
     || bslmf::IsPointer<TARGET_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental or pointer types.  Note that
        // copy-constructor can't throw, and that assignment (although would
        // likely produce equivalent code) can't be used, in case 'TARGET_TYPE'
        // is 'const'-qualified.

        ::new (address) TARGET_TYPE(original);
    } else {
        std::memcpy(address, BSLS_UTIL_ADDRESSOF(original), sizeof original);
    }
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(TARGET_TYPE                *address,
                                    const TARGET_TYPE&          original,
                                    bslma::Allocator           *,
                                    bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(original);
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(
                             TARGET_TYPE                             *address,
                             const TARGET_TYPE&                       original,
                             bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *)
{
    if (bslmf::IsFundamental<TARGET_TYPE>::value
     || bslmf::IsPointer<TARGET_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental or pointer types.  Note that
        // copy-constructor can't throw, and that assignment (although would
        // likely produce equivalent code) can't be used, in case 'TARGET_TYPE'
        // is 'const'-qualified.

        ::new (address) TARGET_TYPE(original);
    } else {
        std::memcpy(address, BSLS_UTIL_ADDRESSOF(original), sizeof original);
    }
}

template <typename TARGET_TYPE>
inline
void
ScalarPrimitives_Imp::copyConstruct(TARGET_TYPE                *address,
                                    const TARGET_TYPE&          original,
                                    bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(original);
}

                     // *** destructiveMove overloads: ***

template <typename TARGET_TYPE, typename ALLOCATOR>
inline
void
ScalarPrimitives_Imp::destructiveMove(
                             TARGET_TYPE                             *address,
                             TARGET_TYPE                             *original,
                             ALLOCATOR                               *,
                             bslmf::MetaInt<BITWISE_MOVEABLE_TRAITS> *)
{
    if (bslmf::IsFundamental<TARGET_TYPE>::value
     || bslmf::IsPointer<TARGET_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental or pointer types.  Note that
        // copy-constructor can't throw, and that assignment (although would
        // likely produce equivalent code) can't be used, in case 'TARGET_TYPE'
        // is 'const'-qualified.

        ::new (address) TARGET_TYPE(*original);
    } else {
        std::memcpy(address, original, sizeof *original);   // no overlap
    }
}

template <typename TARGET_TYPE, typename ALLOCATOR>
inline
void
ScalarPrimitives_Imp::destructiveMove(TARGET_TYPE                *address,
                                      TARGET_TYPE                *original,
                                      ALLOCATOR                  *allocator,
                                      bslmf::MetaInt<NIL_TRAITS> *)
{
    ScalarPrimitives::copyConstruct(address, *original, allocator);
    ScalarDestructionPrimitives::destroy(original);
}

                        // *** construct overloads: ***

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives_Imp::construct(
                              TARGET_TYPE                             *address,
                              const ARG1&                              a1,
                              bslma::Allocator                        *,
                              bslmf::MetaInt<BITWISE_COPYABLE_TRAITS> *)
{
    if (bslmf::IsFundamental<TARGET_TYPE>::value
     || bslmf::IsPointer<TARGET_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental or pointer types.  Note that
        // copy-constructor can't throw, and that assignment (although would
        // likely produce equivalent code) can't be used, in case 'TARGET_TYPE'
        // is 'const'-qualified.

        ::new (address) TARGET_TYPE(a1);
    } else {
        std::memcpy(address, BSLS_UTIL_ADDRESSOF(a1), sizeof a1); // no overlap
    }
}

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                 *address,
                                const ARG1&                  a1,
                                bslma::Allocator            *allocator,
                                bslmf::MetaInt<PAIR_TRAITS> *)
{
    ScalarPrimitives::construct(unconst(BSLS_UTIL_ADDRESSOF(address->first)),
                                a1.first,
                                allocator);
    AutoScalarDestructor<typename bslmf::RemoveCvq<
                                typename TARGET_TYPE::first_type>::Type>
                           guard(unconst(BSLS_UTIL_ADDRESSOF(address->first)));
    ScalarPrimitives::construct(unconst(BSLS_UTIL_ADDRESSOF(address->second)),
                                a1.second,
                                allocator);
    guard.release();
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                 *address,
                                const ARG1&                  a1,
                                const ARG2&                  a2,
                                bslma::Allocator            *allocator,
                                bslmf::MetaInt<PAIR_TRAITS> *)
{
    ScalarPrimitives::construct(unconst(BSLS_UTIL_ADDRESSOF(address->first)),
                                a1,
                                allocator);
    AutoScalarDestructor<typename bslmf::RemoveCvq<
                                typename TARGET_TYPE::first_type>::Type>
                           guard(unconst(BSLS_UTIL_ADDRESSOF(address->first)));
    ScalarPrimitives::construct(unconst(BSLS_UTIL_ADDRESSOF(address->second)),
                                a2,
                                allocator);
    guard.release();
}

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, allocator);
}

template <typename TARGET_TYPE, typename ARG1>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                const ARG10&                a10,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                                allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                const ARG10&                a10,
                                const ARG11&                a11,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11,
          typename ARG12>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                             a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
                             allocator);
}

template <typename TARGET_TYPE, typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4, typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8, typename ARG9, typename ARG10, typename ARG11,
          typename ARG12>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                const ARG10&                a10,
                                const ARG11&                a11,
                                const ARG12&                a12,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                            a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

template <typename TARGET_TYPE,  typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9, typename ARG10, typename ARG11,
          typename ARG12, typename ARG13>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        const ARG13&                                 a13,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                        a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13,
                        allocator);
}

template <typename TARGET_TYPE,  typename ARG1, typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5, typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9, typename ARG10, typename ARG11,
          typename ARG12, typename ARG13>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                const ARG10&                a10,
                                const ARG11&                a11,
                                const ARG12&                a12,
                                const ARG13&                a13,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                       a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <typename TARGET_TYPE,  typename ARG1,  typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5,  typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9,  typename ARG10, typename ARG11,
          typename ARG12, typename ARG13, typename ARG14>
inline
void
ScalarPrimitives_Imp::construct(
                        TARGET_TYPE                                 *address,
                        const ARG1&                                  a1,
                        const ARG2&                                  a2,
                        const ARG3&                                  a3,
                        const ARG4&                                  a4,
                        const ARG5&                                  a5,
                        const ARG6&                                  a6,
                        const ARG7&                                  a7,
                        const ARG8&                                  a8,
                        const ARG9&                                  a9,
                        const ARG10&                                 a10,
                        const ARG11&                                 a11,
                        const ARG12&                                 a12,
                        const ARG13&                                 a13,
                        const ARG14&                                 a14,
                        bslma::Allocator                            *allocator,
                        bslmf::MetaInt<USES_BSLMA_ALLOCATOR_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14,
                   allocator);
}

template <typename TARGET_TYPE,  typename ARG1,  typename ARG2,  typename ARG3,
          typename ARG4,  typename ARG5,  typename ARG6,  typename ARG7,
          typename ARG8,  typename ARG9,  typename ARG10, typename ARG11,
          typename ARG12, typename ARG13, typename ARG14>
inline
void
ScalarPrimitives_Imp::construct(TARGET_TYPE                *address,
                                const ARG1&                 a1,
                                const ARG2&                 a2,
                                const ARG3&                 a3,
                                const ARG4&                 a4,
                                const ARG5&                 a5,
                                const ARG6&                 a6,
                                const ARG7&                 a7,
                                const ARG8&                 a8,
                                const ARG9&                 a9,
                                const ARG10&                a10,
                                const ARG11&                a11,
                                const ARG12&                a12,
                                const ARG13&                a13,
                                const ARG14&                a14,
                                bslma::Allocator           *,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    ::new (address) TARGET_TYPE(
                  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

                          // *** swap overloads: ***

template <typename LHS_TYPE, typename RHS_TYPE>
void ScalarPrimitives_Imp::swap(LHS_TYPE&                                lhs,
                                RHS_TYPE&                                rhs,
                                bslmf::MetaInt<BITWISE_MOVEABLE_TRAITS> *)
{
    if (bslmf::IsSame<LHS_TYPE, RHS_TYPE>::value
     && !bslmf::IsFundamental<LHS_TYPE>::value
     && !bslmf::IsPointer<LHS_TYPE>::value
     && !bslma::UsesBslmaAllocator<LHS_TYPE>::value) {
        // Detectable at compile-time, this condition ensures that we don't
        // call library functions for fundamental types.  It also ensures we
        // don't bitwise swap types that use allocators.  Note that assignment
        // can throw only for types that use allocators.

        char arena[sizeof lhs];
        std::memcpy(arena, BSLS_UTIL_ADDRESSOF(lhs),  sizeof lhs);
        std::memcpy(BSLS_UTIL_ADDRESSOF(lhs),
                    BSLS_UTIL_ADDRESSOF(rhs),
                    sizeof lhs);
                                                    // no overlap, or identical
        std::memcpy(BSLS_UTIL_ADDRESSOF(rhs),  arena, sizeof lhs);
    } else {
        LHS_TYPE temp(lhs);
        lhs = rhs;
        rhs = temp;
    }
}

template <typename LHS_TYPE, typename RHS_TYPE>
void ScalarPrimitives_Imp::swap(LHS_TYPE&                   lhs,
                                RHS_TYPE&                   rhs,
                                bslmf::MetaInt<NIL_TRAITS> *)
{
    LHS_TYPE temp(lhs);
    lhs = rhs;
    rhs = temp;
}

}  // close package namespace

// ===========================================================================
//                           BACKWARD COMPATIBILITY
// ===========================================================================

typedef bslalg::ScalarPrimitives bslalg_ScalarPrimitives;
    // This alias is defined for backward compatibility.

}  // close enterprise namespace

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2008
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
