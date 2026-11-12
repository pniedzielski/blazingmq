// Copyright 2019-2023 Bloomberg Finance L.P.
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

#ifndef INCLUDED_BMQEX_EXECUTORTRAITS
#define INCLUDED_BMQEX_EXECUTORTRAITS

//@PURPOSE: Provides a traits class to access various properties of executors.
//
//@CLASSES:
//  ExecutorTraits: executor traits.
//
//@DESCRIPTION:
// This component provides a class template, 'bmqex::ExecutorTraits', to access
// various properties of Executors. Other components provided by the 'bmqex'
// package access executors through this template, which makes it possible to
// use any class type as an executor, as long as the user-provided
// specialization of 'ExecutorTraits' implements all required functionality.

// BDE
#include <bsl_utility.h>
#include <bslmf_enableif.h>
#include <bslmf_integralconstant.h>
#include <bslmf_util.h>
#include <bslmf_voidtype.h>
#include <bsl_type_traits.h>

namespace BloombergLP {
namespace bmqex {

// =================================
// struct ExecutorTraits_CanDispatch
// =================================

/// Provides a metafunction to detect if an executor has a proper `dispatch`
/// member function.
template <class EXECUTOR, class FUNCTION>
    struct ExecutorTraits_CanDispatch : bsl::integral_constant < bool,
    requires(const EXECUTOR& executor, FUNCTION&& f) {
    executor.dispatch(bsl::forward<FUNCTION>(f));
}>{};

// =========================
// struct ExecutorTraits_Imp
// =========================

/// Provides an implementation for `ExecutorTraits`.
struct ExecutorTraits_Imp {
    // CLASS METHODS

    /// Perform `executor.post(bsl::forward<FUNCTION>(f))`.
    template <class EXECUTOR, class FUNCTION>
    static void post(const EXECUTOR& executor, FUNCTION&& f);

    /// Perform `executor.dispatch(bsl::forward<FUNCTION>(f))` if that is a
    /// well-formed expression, and `executor.post(bsl::forward<FUNCTION>(f))`
    /// otherwise.
    template <class EXECUTOR, class FUNCTION>
    static void dispatch(const EXECUTOR& executor, FUNCTION&& f);
};

// =====================
// struct ExecutorTraits
// =====================

/// Provides the default, non-specialized, executor traits to access various
/// properties of Executors.
template <class EXECUTOR>
struct ExecutorTraits {
    // TYPES

    /// Defines the type of the executor.
    typedef EXECUTOR ExecutorType;

    // CLASS METHODS

    /// Perform `executor.post(bsl::forward<FUNCTION>(f))`.
    template <class FUNCTION>
    static void post(const EXECUTOR& executor, FUNCTION&& f);

    /// If `executor.dispatch(bsl::forward<FUNCTION>(f))` is a well-formed
    /// expression, perform `executor.dispatch(bsl::forward<FUNCTION>(f))`.
    /// Otherwise, perform `executor.post(bsl::forward<FUNCTION>(f))`.
    template <class FUNCTION>
    static void dispatch(const EXECUTOR& executor, FUNCTION&& f);
};

// ============================================================================
//                           INLINE DEFINITIONS
// ============================================================================

// -------------------------
// struct ExecutorTraits_Imp
// -------------------------

// CLASS METHODS
template <class EXECUTOR, class FUNCTION>
inline void ExecutorTraits_Imp::post(const EXECUTOR& executor, FUNCTION&& f)
{
    executor.post(bsl::forward<FUNCTION>(f));
}

template <class EXECUTOR, class FUNCTION>
inline void ExecutorTraits_Imp::dispatch(const EXECUTOR& executor,
                                         FUNCTION&&      f)
{
    if constexpr (ExecutorTraits_CanDispatch<EXECUTOR, FUNCTION>::value) {
        executor.dispatch(bsl::forward<FUNCTION>(f));
    }
    else {
        executor.post(bsl::forward<FUNCTION>(f));
    }
}

// ---------------------
// struct ExecutorTraits
// ---------------------

// CLASS METHODS
template <class EXECUTOR>
template <class FUNCTION>
inline void ExecutorTraits<EXECUTOR>::post(const EXECUTOR& executor,
                                           FUNCTION&&      f)
{
    ExecutorTraits_Imp::post(executor, bsl::forward<FUNCTION>(f));
}

template <class EXECUTOR>
template <class FUNCTION>
inline void ExecutorTraits<EXECUTOR>::dispatch(const EXECUTOR& executor,
                                               FUNCTION&&      f)
{
    ExecutorTraits_Imp::dispatch(executor, bsl::forward<FUNCTION>(f));
}

}  // close package namespace
}  // close enterprise namespace

#endif
