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

#include <bmqu_uniqueptr.h>

// TEST DRIVER
#include <bmqtst_testhelper.h>

// BDE
#include <bsl_iostream.h>
#include <bsl_string.h>
#include <bsl_utility.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_managedptr.h>
#include <bslma_testallocator.h>

// CONVENIENCE
using namespace BloombergLP;

// ============================================================================
//                            TEST HELPERS UTILITY
// ----------------------------------------------------------------------------
namespace {

/// A polymorphic base class with a virtual destructor.
class Base {
  public:
    // CREATORS
    virtual ~Base();

    // ACCESSORS
    virtual int value() const = 0;
};

Base::~Base()
{
    // NOTHING
}

/// A class derived from `Base`, recording its destruction in a counter and
/// using an allocator, so that a test allocator can detect whether the
/// object was properly destroyed and deallocated.
class Derived : public Base {
  private:
    // DATA
    bsl::string d_string;

    int d_value;

    int* d_destructionCount_p;

  public:
    // CREATORS
    Derived(int value, int* destructionCount, bslma::Allocator* allocator)
    : d_string("a string long enough to require an allocation", allocator)
    , d_value(value)
    , d_destructionCount_p(destructionCount)
    {
        // NOTHING
    }

    ~Derived() override { ++*d_destructionCount_p; }

    // ACCESSORS
    int value() const override { return d_value; }
};

/// A class that does not use allocators, taking several constructor
/// arguments.
struct Pair {
    // DATA
    int d_first;

    int d_second;

    // CREATORS
    Pair(int first, int second)
    : d_first(first)
    , d_second(second)
    {
        // NOTHING
    }
};

}  // close unnamed namespace

// ============================================================================
//                                    TESTS
// ----------------------------------------------------------------------------

static void test1_breathingTest()
// ------------------------------------------------------------------------
// BREATHING TEST
//
// Concerns:
//   Exercise basic functionality before beginning testing in detail.
//
// Testing:
//   BREATHING TEST
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("BREATHING TEST");

    bslma::TestAllocator ta("test", false);
    {
        bmqu::UniquePtr<Pair> pair =
            bmqu::UniquePtrUtil::allocateUnique<Pair>(&ta, 1, 2);

        BMQTST_ASSERT(pair);
        BMQTST_ASSERT_EQ(pair->d_first, 1);
        BMQTST_ASSERT_EQ(pair->d_second, 2);
        BMQTST_ASSERT_EQ(pair.get_deleter().allocator(), &ta);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 1);
    }
    BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);
}

static void test2_allocateUniquePassesAllocator()
// ------------------------------------------------------------------------
// ALLOCATE UNIQUE PASSES ALLOCATOR
//
// Concerns:
//   1. 'allocateUnique' passes the allocator to the constructor of types
//      using 'bslma' allocators.
//   2. Memory allocated by the object is returned when the pointer is
//      destroyed.
//   3. A null allocator means the default allocator.
//
// Testing:
//   allocateUnique
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("ALLOCATE UNIQUE PASSES ALLOCATOR");

    bslma::TestAllocator ta("test", false);
    {
        bmqu::UniquePtr<bsl::string> string =
            bmqu::UniquePtrUtil::allocateUnique<bsl::string>(
                &ta,
                "a string long enough to require an allocation");

        BMQTST_ASSERT_EQ(*string,
                         "a string long enough to require an allocation");
        BMQTST_ASSERT_EQ(string->get_allocator().mechanism(), &ta);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 2);
    }
    BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);

    bslma::TestAllocator         da("default", false);
    bslma::DefaultAllocatorGuard guard(&da);
    {
        bmqu::UniquePtr<Pair> pair =
            bmqu::UniquePtrUtil::allocateUnique<Pair>(0, 3, 4);

        BMQTST_ASSERT_EQ(pair.get_deleter().allocator(), &da);
        BMQTST_ASSERT_EQ(da.numBlocksInUse(), 1);
    }
    BMQTST_ASSERT_EQ(da.numBlocksInUse(), 0);
}

static void test3_derivedToBaseConversion()
// ------------------------------------------------------------------------
// DERIVED TO BASE CONVERSION
//
// Concerns:
//   1. 'UniquePtr<DERIVED>' converts to 'UniquePtr<BASE>'.
//   2. The object is destroyed as a 'DERIVED' and its memory (and the
//      memory it allocated) is returned to the allocator.
//
// Testing:
//   UniquePtr<DERIVED> -> UniquePtr<BASE>
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("DERIVED TO BASE CONVERSION");

    bslma::TestAllocator ta("test", false);
    int                  destructionCount = 0;
    {
        bmqu::UniquePtr<Derived> derived =
            bmqu::UniquePtrUtil::allocateUnique<Derived>(&ta,
                                                         42,
                                                         &destructionCount,
                                                         &ta);

        bmqu::UniquePtr<Base> base(bsl::move(derived));
        BMQTST_ASSERT(!derived);
        BMQTST_ASSERT_EQ(base->value(), 42);
        BMQTST_ASSERT_EQ(base.get_deleter().allocator(), &ta);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 2);
    }
    BMQTST_ASSERT_EQ(destructionCount, 1);
    BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);
}

static void test4_adopt()
// ------------------------------------------------------------------------
// ADOPT
//
// Concerns:
//   1. 'adopt' takes ownership of an object allocated from an allocator,
//      and returns it to that allocator.
//   2. 'adopt' accepts a null object.
//
// Testing:
//   adopt
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("ADOPT");

    bslma::TestAllocator ta("test", false);
    int                  destructionCount = 0;
    {
        bmqu::UniquePtr<Base> base = bmqu::UniquePtrUtil::adopt<Base>(
            new (ta) Derived(7, &destructionCount, &ta),
            &ta);

        BMQTST_ASSERT_EQ(base->value(), 7);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 2);
    }
    BMQTST_ASSERT_EQ(destructionCount, 1);
    BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);

    bmqu::UniquePtr<Pair> null = bmqu::UniquePtrUtil::adopt<Pair>(0, &ta);
    BMQTST_ASSERT(!null);
}

static void test5_defaultConstruction()
// ------------------------------------------------------------------------
// DEFAULT CONSTRUCTION
//
// Concerns:
//   1. A default-constructed 'UniquePtr' is null.
//   2. A default-constructed 'AllocatorDeleter' uses the default allocator
//      installed at the time it is constructed.
//   3. Assigning the result of a factory function replaces the deleter, so
//      the object is returned to the right allocator.
//
// Testing:
//   AllocatorDeleter()
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("DEFAULT CONSTRUCTION");

    bslma::TestAllocator         da("default", false);
    bslma::DefaultAllocatorGuard guard(&da);

    bslma::TestAllocator ta("test", false);
    {
        bmqu::UniquePtr<Pair> pair;
        BMQTST_ASSERT(!pair);
        BMQTST_ASSERT_EQ(pair.get_deleter().allocator(), &da);

        pair = bmqu::UniquePtrUtil::allocateUnique<Pair>(&ta, 5, 6);
        BMQTST_ASSERT_EQ(pair.get_deleter().allocator(), &ta);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 1);

        pair.reset();
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);
    }
    BMQTST_ASSERT_EQ(da.numBlocksTotal(), 0);
}

static void test6_toManaged()
// ------------------------------------------------------------------------
// TO MANAGED
//
// Concerns:
//   1. 'toManaged' transfers ownership to a 'bslma::ManagedPtr', leaving
//      the 'UniquePtr' empty.
//   2. The object is destroyed and returned to the original allocator when
//      the 'bslma::ManagedPtr' is destroyed.
//
// Testing:
//   toManaged
// ------------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("TO MANAGED");

    bslma::TestAllocator ta("test", false);
    int                  destructionCount = 0;
    {
        bmqu::UniquePtr<Base> base =
            bmqu::UniquePtrUtil::allocateUnique<Derived>(&ta,
                                                         9,
                                                         &destructionCount,
                                                         &ta);

        bslma::ManagedPtr<Base> managed = bmqu::UniquePtrUtil::toManaged(
            bsl::move(base));

        BMQTST_ASSERT(!base);
        BMQTST_ASSERT_EQ(managed->value(), 9);
        BMQTST_ASSERT_EQ(destructionCount, 0);
        BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 2);
    }
    BMQTST_ASSERT_EQ(destructionCount, 1);
    BMQTST_ASSERT_EQ(ta.numBlocksInUse(), 0);
}

// ============================================================================
//                                 MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    TEST_PROLOG(bmqtst::TestHelper::e_DEFAULT);

    switch (_testCase) {
    case 0:
    case 6: test6_toManaged(); break;
    case 5: test5_defaultConstruction(); break;
    case 4: test4_adopt(); break;
    case 3: test3_derivedToBaseConversion(); break;
    case 2: test2_allocateUniquePassesAllocator(); break;
    case 1: test1_breathingTest(); break;
    default: {
        bsl::cerr << "WARNING: CASE '" << _testCase << "' NOT FOUND."
                  << bsl::endl;
        bmqtst::TestHelperUtil::testStatus() = -1;
    } break;
    }

    TEST_EPILOG(bmqtst::TestHelper::e_CHECK_DEF_GBL_ALLOC);
}
