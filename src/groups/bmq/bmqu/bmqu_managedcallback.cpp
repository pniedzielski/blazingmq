// Copyright 2025 Bloomberg Finance L.P.
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

#include <bmqu_managedcallback.h>

#include <bmqscm_version.h>
#include <bsl_utility.h>

namespace BloombergLP {
namespace bmqu {

namespace {

class VoidCallback : public bmqu::ManagedCallback::CallbackFunctor {
  private:
    // PRIVATE DATA
    bmqu::ManagedCallback::VoidFunctor d_callback;

  public:
    // CREATORS
    explicit VoidCallback(const bmqu::ManagedCallback::VoidFunctor& callback)
    : d_callback(callback)
    {
        // NOTHING
    }

    explicit VoidCallback(bmqu::ManagedCallback::VoidFunctor&& callback)
    : d_callback(bsl::move(callback))
    {
        // NOTHING
    }

    ~VoidCallback() override
    {
        // NOTHING
    }

    // ACCESSORS
    void operator()() const override
    {
        if (d_callback) {
            d_callback();
        }
    }
};

}  // close unnamed namespace

// ---------------------------------------
// struct ManagedCallback::CallbackFunctor
// ---------------------------------------

ManagedCallback::CallbackFunctor::~CallbackFunctor()
{
    // NOTHING
}

// ----------------------
// struct ManagedCallback
// ----------------------

void ManagedCallback::set(const VoidFunctor& callback)
{
    // Preconditions for placement are checked in `place`.
    // Destructor is called by `reset` of the holding DispatcherEvent.
    new (place<VoidCallback>()) VoidCallback(callback);
}

void ManagedCallback::set(VoidFunctor&& callback)
{
    // Preconditions for placement are checked in `place`.
    // Destructor is called by `reset` of the holding DispatcherEvent.
    new (place<VoidCallback>()) VoidCallback(bsl::move(callback));
}

}  // close package namespace
}  // close enterprise namespace
