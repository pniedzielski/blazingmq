// Copyright 2026 Bloomberg Finance L.P.
// SPDX-License-Identifier: Apache-2.0
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

#ifndef INCLUDED_BMQU_UNIQUEPTR
#define INCLUDED_BMQU_UNIQUEPTR

//@PURPOSE: Provide an allocator-aware 'bsl::unique_ptr' and its factories.
//
//@CLASSES:
//  bmqu::AllocatorDeleter: deleter returning objects to a 'bslma::Allocator'
//  bmqu::UniquePtr:        'bsl::unique_ptr' using 'bmqu::AllocatorDeleter'
//  bmqu::UniquePtrUtil:    factory functions for 'bmqu::UniquePtr'
//
//@SEE_ALSO: bslma_managedptr
//
//@DESCRIPTION: This component provides 'bmqu::UniquePtr<T>', an alias for
// 'bsl::unique_ptr<T, bmqu::AllocatorDeleter>', which is a move-only smart
// pointer owning an object allocated from a 'bslma::Allocator', together with
// 'bmqu::UniquePtrUtil', a utility providing factory functions creating such
// pointers.  It is intended as a standard replacement for 'bslma::ManagedPtr'
// in code that allocates objects from 'bslma::Allocator's.
//
// 'bmqu::AllocatorDeleter' is a non-template class whose function-call
// operator is a template, so 'bmqu::UniquePtr<DERIVED>' converts implicitly
// to 'bmqu::UniquePtr<BASE>', exactly like 'bslma::ManagedPtr' does.
//
/// Differences with 'bslma::ManagedPtr'
///------------------------------------
// The following differences must be kept in mind when replacing
// 'bslma::ManagedPtr' with 'bmqu::UniquePtr':
//
//: o A 'bslma::ManagedPtr<BASE>' loaded with a 'DERIVED *' remembers the
//:   'DERIVED' type and destroys the object as a 'DERIVED', even if 'BASE'
//:   does not have a virtual destructor.  'bmqu::UniquePtr<BASE>' always
//:   destroys the object through a 'BASE *'.  Therefore a polymorphic 'BASE'
//:   must have a virtual destructor (this is checked at compile time), and a
//:   'bmqu::UniquePtr<BASE>' must never own a 'DERIVED' object if 'BASE' is
//:   not polymorphic.
//:
//: o 'bsl::unique_ptr::reset(p)' keeps the current deleter, and therefore the
//:   current allocator.  To replace the managed object with an object
//:   allocated from a (potentially) different allocator, assign the result
//:   of one of the 'bmqu::UniquePtrUtil' factory functions instead.  A
//:   default-constructed 'bmqu::AllocatorDeleter' (as used by a
//:   default-constructed 'bmqu::UniquePtr') uses the default allocator
//:   installed at the time it is constructed.
//:
//: o 'bmqu::UniquePtr<void>' is not supported, since the deleter would not
//:   know the type of the object to destroy.  Keep using 'bslma::ManagedPtr'
//:   for type-erased ownership.
//
/// Usage
///-----
// This section illustrates intended use of this component.
//
/// Example 1: Creating an Object Owned by a 'bmqu::UniquePtr'
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 'bmqu::UniquePtrUtil::allocateUnique' allocates an object from the
// specified allocator and constructs it from the specified arguments.  If
// the object's type uses 'bslma' allocators, the allocator is also passed to
// the object's constructor, exactly like
// 'bslma::ManagedPtrUtil::allocateManaged' does:
//..
//  bslma::TestAllocator ta;
//  {
//      bmqu::UniquePtr<bsl::string> string =
//          bmqu::UniquePtrUtil::allocateUnique<bsl::string>(&ta, "hello");
//      BSLS_ASSERT(*string == "hello");
//      BSLS_ASSERT(string->get_allocator().mechanism() == &ta);
//  }
//  BSLS_ASSERT(0 == ta.numBlocksInUse());
//..
//
/// Example 2: Taking Ownership of an Existing Object
///- - - - - - - - - - - - - - - - - - - - - - - - -
// 'bmqu::UniquePtrUtil::adopt' takes ownership of an object that was already
// allocated from an allocator, mirroring the 'bslma::ManagedPtr' constructor
// taking a pointer and an allocator:
//..
//  bmqu::UniquePtr<Base> object = bmqu::UniquePtrUtil::adopt(
//                                       new (ta) Derived(&ta),
//                                       &ta);
//..

// BDE
#include <bsl_memory.h>
#include <bsl_type_traits.h>
#include <bsl_utility.h>
#include <bslma_allocator.h>
#include <bslma_allocatorutil.h>
#include <bslma_default.h>

namespace BloombergLP {
namespace bmqu {

// ======================
// class AllocatorDeleter
// ======================

/// A deleter, usable with `bsl::unique_ptr`, destroying objects and
/// returning their memory to the `bslma::Allocator` it was created with.
class AllocatorDeleter {
  private:
    // DATA

    /// The allocator objects are returned to.  Never null.
    bslma::Allocator* d_allocator_p;

  public:
    // CREATORS

    /// Create a deleter returning objects to the currently installed
    /// default allocator.
    AllocatorDeleter() noexcept;

    /// Create a deleter returning objects to the specified `allocator`.
    /// If `allocator` is 0, the currently installed default allocator is
    /// used.
    explicit AllocatorDeleter(bslma::Allocator* allocator) noexcept;

    // ACCESSORS

    /// Destroy the object at the specified `object` address and return its
    /// memory to the allocator of this deleter.  The behavior is undefined
    /// unless `object` was allocated from that allocator and, if `object`
    /// does not point to the most derived object, `TYPE` has a virtual
    /// destructor.
    template <class TYPE>
    void operator()(TYPE* object) const;

    /// Return the allocator objects are returned to.
    bslma::Allocator* allocator() const noexcept;
};

// ===============
// using UniquePtr
// ===============

/// A move-only smart pointer owning an object of the specified `TYPE`
/// allocated from a `bslma::Allocator`.
template <class TYPE>
using UniquePtr = bsl::unique_ptr<TYPE, AllocatorDeleter>;

// ====================
// struct UniquePtrUtil
// ====================

/// This utility struct provides factory functions creating `UniquePtr`s.
struct UniquePtrUtil {
    // CLASS METHODS

    /// Return a `UniquePtr` owning an object of the specified `TYPE`,
    /// allocated from the specified `allocator` and constructed from the
    /// specified `args`.  If `TYPE` uses `bslma` allocators, `allocator` is
    /// passed to its constructor as well.  If `allocator` is 0, the
    /// currently installed default allocator is used.  This is the
    /// `UniquePtr` equivalent of `bslma::ManagedPtrUtil::allocateManaged`.
    template <class TYPE, class... ARGS>
    static UniquePtr<TYPE> allocateUnique(bslma::Allocator* allocator,
                                          ARGS&&... args);

    /// Return a `UniquePtr` owning the object at the specified `object`
    /// address, which will be returned to the specified `allocator` when
    /// the returned pointer is destroyed.  If `allocator` is 0, the
    /// currently installed default allocator is used.  The behavior is
    /// undefined unless `object` is null or was allocated from `allocator`.
    /// This is the `UniquePtr` equivalent of constructing (or loading) a
    /// `bslma::ManagedPtr` from a pointer and an allocator.
    template <class TYPE>
    static UniquePtr<TYPE> adopt(TYPE*             object,
                                 bslma::Allocator* allocator) noexcept;
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

// ----------------------
// class AllocatorDeleter
// ----------------------

// CREATORS
inline AllocatorDeleter::AllocatorDeleter() noexcept
: d_allocator_p(bslma::Default::defaultAllocator())
{
    // NOTHING
}

inline AllocatorDeleter::AllocatorDeleter(bslma::Allocator* allocator) noexcept
: d_allocator_p(bslma::Default::allocator(allocator))
{
    // NOTHING
}

// ACCESSORS
template <class TYPE>
inline void AllocatorDeleter::operator()(TYPE* object) const
{
    static_assert(!bsl::is_polymorphic<TYPE>::value ||
                      bsl::has_virtual_destructor<TYPE>::value,
                  "a polymorphic type must have a virtual destructor");

    d_allocator_p->deleteObject(object);
}

inline bslma::Allocator* AllocatorDeleter::allocator() const noexcept
{
    return d_allocator_p;
}

// --------------------
// struct UniquePtrUtil
// --------------------

// CLASS METHODS
template <class TYPE, class... ARGS>
inline UniquePtr<TYPE>
UniquePtrUtil::allocateUnique(bslma::Allocator* allocator, ARGS&&... args)
{
    typedef typename bsl::remove_cv<TYPE>::type UnqualifiedType;

    allocator = bslma::Default::allocator(allocator);

    TYPE* object = bslma::AllocatorUtil::newObject<UnqualifiedType>(
        allocator,
        bsl::forward<ARGS>(args)...);

    return UniquePtr<TYPE>(object, AllocatorDeleter(allocator));
}

template <class TYPE>
inline UniquePtr<TYPE>
UniquePtrUtil::adopt(TYPE* object, bslma::Allocator* allocator) noexcept
{
    return UniquePtr<TYPE>(object, AllocatorDeleter(allocator));
}

}  // close package namespace
}  // close enterprise namespace

#endif
