// Copyright 2016-2023 Bloomberg Finance L.P.
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

#include <bmqa_abstractsession.h>

#include <bmqscm_version.h>
// BDE
#include <bsla_annotations.h>
#include <bslma_allocator.h>
#include <bsls_assert.h>

namespace BloombergLP {
namespace bmqa {

// ---------------------
// class AbstractSession
// ---------------------

AbstractSession::~AbstractSession()
{
    // NOTHING
}

// MANIPULATORS

/// Session management
///------------------
int AbstractSession::start([[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::startAsync(
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

void AbstractSession::stop()
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

void AbstractSession::stopAsync()
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

void AbstractSession::finalizeStop()
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

void AbstractSession::loadMessageEventBuilder(
    [[maybe_unused]] MessageEventBuilder* builder)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

void AbstractSession::loadConfirmEventBuilder(
    [[maybe_unused]] ConfirmEventBuilder* builder)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

void AbstractSession::loadMessageProperties(
    [[maybe_unused]] MessageProperties* buffer)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

/// Queue management
///----------------
int AbstractSession::getQueueId([[maybe_unused]] QueueId*         queueId,
                                [[maybe_unused]] const bmqt::Uri& uri)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::getQueueId(
    [[maybe_unused]] QueueId*                   queueId,
    [[maybe_unused]] const bmqt::CorrelationId& correlationId)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::openQueue(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::Uri&          uri,
    [[maybe_unused]] bsls::Types::Uint64       flags,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

OpenQueueStatus AbstractSession::openQueueSync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::Uri&          uri,
    [[maybe_unused]] bsls::Types::Uint64       flags,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return bmqa::OpenQueueStatus(bmqa::QueueId(),
                                 bmqt::OpenQueueResult::e_NOT_SUPPORTED,
                                 "Method is undefined in base protocol");
}

int AbstractSession::openQueueAsync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::Uri&          uri,
    [[maybe_unused]] bsls::Types::Uint64       flags,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

void AbstractSession::openQueueAsync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::Uri&          uri,
    [[maybe_unused]] bsls::Types::Uint64       flags,
    [[maybe_unused]] const OpenQueueCallback&  callback,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

int AbstractSession::configureQueue(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

ConfigureQueueStatus AbstractSession::configureQueueSync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)

{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return bmqa::ConfigureQueueStatus(
        bmqa::QueueId(),
        bmqt::ConfigureQueueResult::e_NOT_SUPPORTED,
        "Method is undefined in base protocol");
}

int AbstractSession::configureQueueAsync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bmqt::QueueOptions& options,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

void AbstractSession::configureQueueAsync(
    [[maybe_unused]] QueueId*                      queueId,
    [[maybe_unused]] const bmqt::QueueOptions&     options,
    [[maybe_unused]] const ConfigureQueueCallback& callback,
    [[maybe_unused]] const bsls::TimeInterval&     timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

int AbstractSession::closeQueue(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

CloseQueueStatus AbstractSession::closeQueueSync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return bmqa::CloseQueueStatus(bmqa::QueueId(),
                                  bmqt::CloseQueueResult::e_NOT_SUPPORTED,
                                  "Method is undefined in base protocol");
}

int AbstractSession::closeQueueAsync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

void AbstractSession::closeQueueAsync(
    [[maybe_unused]] QueueId*                  queueId,
    [[maybe_unused]] const CloseQueueCallback& callback,
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");
}

/// Queue manipulation
///------------------
Event AbstractSession::nextEvent(
    [[maybe_unused]] const bsls::TimeInterval& timeout)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return Event();
}

int AbstractSession::post([[maybe_unused]] const MessageEvent& event)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::confirmMessage([[maybe_unused]] const Message& message)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::confirmMessage(
    [[maybe_unused]] const MessageConfirmationCookie& cookie)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

int AbstractSession::confirmMessages(
    [[maybe_unused]] ConfirmEventBuilder* builder)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

/// Debugging related
///-----------------
int AbstractSession::configureMessageDumping(
    [[maybe_unused]] const bslstl::StringRef& command)
{
    // PRECONDITIONS
    BSLS_ASSERT_OPT(false && "Method is undefined in base protocol");

    return -1;
}

}  // close package namespace
}  // close enterprise namespace
