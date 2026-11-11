// Copyright 2018-2023 Bloomberg Finance L.P.
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

#include <bmqex_executor.h>

#include <bmqscm_version.h>
#include <bsl_utility.h>
namespace BloombergLP {
namespace bmqex {

// -------------------------
// class Executor_TargetBase
// -------------------------

Executor_TargetBase::~Executor_TargetBase()
{
    // NOTHING
}

// -------------------------
// class Executor_Box_DefImp
// -------------------------

// MANIPULATORS
void Executor_Box_DefImp::swap(Executor_Box_DefImp& other) noexcept
{
    d_target.swap(other.d_target);
}

Executor_TargetBase* Executor_Box_DefImp::target() noexcept
{
    return d_target.get();
}

const Executor_TargetBase* Executor_Box_DefImp::target() const noexcept
{
    return d_target.get();
}

// -------------------------
// class Executor_Box_SboImp
// -------------------------

// CREATORS
Executor_Box_SboImp::Executor_Box_SboImp(
    const Executor_Box_SboImp& original) noexcept
{
    original.target()->copy(this);
}

Executor_Box_SboImp::Executor_Box_SboImp(
    bslmf::MovableRef<Executor_Box_SboImp> original) noexcept
{
    bslmf::MovableRefUtil::access(original).target()->move(this);
}

Executor_Box_SboImp::~Executor_Box_SboImp()
{
    target()->~Executor_TargetBase();
}

// MANIPULATORS
Executor_Box_SboImp&
Executor_Box_SboImp::operator=(const Executor_Box_SboImp& rhs) noexcept
{
    target()->~Executor_TargetBase();
    rhs.target()->copy(this);

    return *this;
}

Executor_Box_SboImp& Executor_Box_SboImp::operator=(
    bslmf::MovableRef<Executor_Box_SboImp> rhs) noexcept
{
    target()->~Executor_TargetBase();
    bslmf::MovableRefUtil::access(rhs).target()->move(this);

    return *this;
}

void Executor_Box_SboImp::swap(Executor_Box_SboImp& other) noexcept
{
    Executor_Box_SboImp tmp(bsl::move(other));

    other = bsl::move(*this);
    *this = bsl::move(tmp);
}

// ACCESSORS
Executor_TargetBase* Executor_Box_SboImp::target() noexcept
{
    return reinterpret_cast<Executor_TargetBase*>(this);
}

const Executor_TargetBase* Executor_Box_SboImp::target() const noexcept
{
    return reinterpret_cast<const Executor_TargetBase*>(this);
}

// ------------------
// class Executor_Box
// ------------------

// CREATORS
Executor_Box::Executor_Box() noexcept : d_imp()
{
    // NOTHING
}

// MANIPULATORS
void Executor_Box::swap(Executor_Box& other) noexcept
{
    d_imp.swap(other.d_imp);
}

// ACCESSORS
Executor_TargetBase* Executor_Box::target() noexcept
{
    if (d_imp.isUnset()) {
        return 0;  // RETURN
    }

    return d_imp.is<Executor_Box_DefImp>()
               ? d_imp.the<Executor_Box_DefImp>().target()
               : d_imp.the<Executor_Box_SboImp>().target();
}

const Executor_TargetBase* Executor_Box::target() const noexcept
{
    return const_cast<Executor_Box*>(this)->target();
}

// --------------
// class Executor
// --------------

// CREATORS
Executor::Executor() noexcept : d_box()
{
    // NOTHING
}

Executor::Executor(const Executor& original, bslma::Allocator*) noexcept
: d_box(original.d_box)
{
    // NOTHING
}

Executor::Executor(bslmf::MovableRef<Executor> original,
                   bslma::Allocator*) noexcept
: d_box(bsl::move(bslmf::MovableRefUtil::access(original).d_box))
{
    // NOTHING
}

// MANIPULATORS
Executor& Executor::operator=(const Executor& rhs) noexcept
{
    d_box = rhs.d_box;
    return *this;
}

Executor& Executor::operator=(bslmf::MovableRef<Executor> rhs) noexcept
{
    d_box = bsl::move(bslmf::MovableRefUtil::access(rhs).d_box);
    return *this;
}

void Executor::swap(Executor& other) noexcept
{
    d_box.swap(other.d_box);
}

// ACCESSORS
Executor::operator bool() const noexcept
{
    return static_cast<bool>(d_box.target());
}

const bsl::type_info& Executor::targetType() const noexcept
{
    return d_box.target() ? d_box.target()->targetType() : typeid(void);
}

}  // close package namespace
}  // close enterprise namespace
