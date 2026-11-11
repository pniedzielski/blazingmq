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

#include <mqbmock_queueengine.h>

#include <mqbscm_version.h>
// BDE
#include <bsl_iostream.h>
#include <bsla_annotations.h>

namespace BloombergLP {
namespace mqbmock {

// -----------------
// class QueueEngine
// -----------------

// CREATORS
QueueEngine::QueueEngine([[maybe_unused]] bslma::Allocator* allocator)
{
    // NOTHING
}

QueueEngine::~QueueEngine()
{
    // NOTHING
}

// MANIPULATORS
int QueueEngine::configure([[maybe_unused]] bsl::ostream& errorDescription,
                           [[maybe_unused]] bool          isReconfigure)
{
    return 0;
}

void QueueEngine::close()
{
    // NOTHING
}

void QueueEngine::resetState([[maybe_unused]] bool keepConfirming)
{
    // NOTHING
}

int QueueEngine::rebuildInternalState(
    [[maybe_unused]] bsl::ostream& errorDescription)
{
    return 0;
}

mqbi::QueueHandle* QueueEngine::getHandle(
    [[maybe_unused]] const mqbi::OpenQueueConfirmationCookieSp& context,
    [[maybe_unused]] const bsl::shared_ptr<mqbi::QueueHandleRequesterContext>&
        clientContext,
    [[maybe_unused]] const bmqp_ctrlmsg::QueueHandleParameters&
                                  handleParameters,
    [[maybe_unused]] unsigned int upstreamSubQueueId,
    [[maybe_unused]] const mqbi::QueueHandle::GetHandleCallback& callback)
{
    return 0;
}

void QueueEngine::configureHandle(
    [[maybe_unused]] mqbi::QueueHandle*                    handle,
    [[maybe_unused]] const bmqp_ctrlmsg::StreamParameters& streamParameters,
    [[maybe_unused]] const mqbi::QueueHandle::HandleConfiguredCallback&
        configuredCb)
{
    // NOTHING
}

void QueueEngine::releaseHandle(
    [[maybe_unused]] mqbi::QueueHandle* handle,
    [[maybe_unused]] const bmqp_ctrlmsg::QueueHandleParameters&
                          handleParameters,
    [[maybe_unused]] bool isFinal,
    [[maybe_unused]] const mqbi::QueueHandle::HandleReleasedCallback&
        releasedCb)
{
    // NOTHING
}

void QueueEngine::onHandleUsable(
    [[maybe_unused]] mqbi::QueueHandle* handle,
    [[maybe_unused]] unsigned int       upstreamSubQueueId)
{
    // NOTHING
}

void QueueEngine::afterNewMessage()
{
    // NOTHING
}

int QueueEngine::onConfirmMessage(
    [[maybe_unused]] mqbi::QueueHandle*       handle,
    [[maybe_unused]] const bmqt::MessageGUID& msgGUID,
    [[maybe_unused]] unsigned int             subQueueId)
{
    return 0;
}

int QueueEngine::onRejectMessage(
    [[maybe_unused]] mqbi::QueueHandle*       handle,
    [[maybe_unused]] const bmqt::MessageGUID& msgGUID,
    [[maybe_unused]] unsigned int             subQueueId)
{
    // TODO: Implement
    return 0;
}

void QueueEngine::beforeMessageRemoved(
    [[maybe_unused]] const bmqt::MessageGUID& msgGUID)
{
    // NOTHING
}

void QueueEngine::afterQueuePurged(
    [[maybe_unused]] const bsl::string&      appId,
    [[maybe_unused]] const mqbu::StorageKey& appKey)
{
    // NOTHING
}

void QueueEngine::afterPostMessage()
{
    // executed by the *QUEUE DISPATCHER* thread

    // NOTHING
}

void QueueEngine::evaluateAppSubscriptions(
    [[maybe_unused]] const bmqp::PutHeader&              putHeader,
    [[maybe_unused]] const bsl::shared_ptr<bdlbb::Blob>& appData,
    [[maybe_unused]] const bmqp::MessagePropertiesInfo&  mpi)
{
    // executed by the *QUEUE DISPATCHER* thread

    // NOTHING
}

// ACCESSORS
unsigned int QueueEngine::messageReferenceCount() const
{
    return 0;
}

void QueueEngine::loadInternals(
    [[maybe_unused]] mqbcmd::QueueEngine* out) const
{
    // NOTHING
}

}  // close package namespace
}  // close enterprise namespace
