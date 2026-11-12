// Copyright 2021-2024 Bloomberg Finance L.P.
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

#include <mqbmock_storagemanager.h>

#include <mqbscm_version.h>

// BDE
#include <bsl_memory.h>
#include <bsl_ostream.h>
#include <bsla_annotations.h>

namespace BloombergLP {
namespace mqbmock {

// --------------------
// class StorageManager
// --------------------

// CREATORS
StorageManager::StorageManager()
{
    // NOTHING
}

StorageManager::~StorageManager()
{
    // NOTHING
}

// MANIPULATORS
int StorageManager::start([[maybe_unused]] bsl::ostream& errorDescription)
{
    return 0;
}

void StorageManager::stop()
{
    // NOTHING
}

void StorageManager::initializeQueueKeyInfoMap(
    [[maybe_unused]] const mqbc::ClusterState& clusterState)
{
}

void StorageManager::registerQueue(
    [[maybe_unused]] const bmqt::Uri&        uri,
    [[maybe_unused]] const mqbu::StorageKey& queueKey,
    [[maybe_unused]] int                     partitionId,
    [[maybe_unused]] const AppInfos&         appIdKeyPairs,
    [[maybe_unused]] mqbi::Domain*           domain)
{
    // NOTHING
}

void StorageManager::unregisterQueue([[maybe_unused]] const bmqt::Uri& uri,
                                     [[maybe_unused]] int partitionId)
{
    // NOTHING
}

int StorageManager::updateQueuePrimary(
    [[maybe_unused]] const bmqt::Uri& uri,
    [[maybe_unused]] int              partitionId,
    [[maybe_unused]] const AppInfos&  addedIdKeyPairs,
    [[maybe_unused]] const AppInfos&  removedIdKeyPairs)
{
    return 0;
}

void StorageManager::setPrimaryForPartition(
    [[maybe_unused]] int                  partitionId,
    [[maybe_unused]] mqbnet::ClusterNode* primaryNode,
    [[maybe_unused]] unsigned int         primaryLeaseId)
{
    // NOTHING
}

void StorageManager::clearPrimaryForPartition(
    [[maybe_unused]] int                  partitionId,
    [[maybe_unused]] mqbnet::ClusterNode* primary)
{
    // NOTHING
}

void StorageManager::setPrimaryStatusForPartition(
    [[maybe_unused]] int                                partitionId,
    [[maybe_unused]] bmqp_ctrlmsg::PrimaryStatus::Value value)
{
    // NOTHING
}

void StorageManager::stopPFSMs()
{
    // NOTHING
}

void StorageManager::detectPrimaryLossInPFSM([[maybe_unused]] int partitionId)
{
    // NOTHING
}

void StorageManager::detectSelfPrimaryInPFSM(
    [[maybe_unused]] int                  partitionId,
    [[maybe_unused]] mqbnet::ClusterNode* primaryNode,
    [[maybe_unused]] unsigned int         primaryLeaseId)
{
    // NOTHING
}

void StorageManager::detectSelfReplicaInPFSM(
    [[maybe_unused]] int                  partitionId,
    [[maybe_unused]] mqbnet::ClusterNode* primaryNode,
    [[maybe_unused]] unsigned int         primaryLeaseId)
{
    // NOTHING
}

void StorageManager::processPrimaryStateRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processReplicaStateRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processReplicaDataRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

int StorageManager::configureStorage(
    [[maybe_unused]] bsl::ostream&                      errorDescription,
    [[maybe_unused]] bsl::shared_ptr<mqbi::Storage>*    out,
    [[maybe_unused]] const bmqt::Uri&                   uri,
    [[maybe_unused]] const mqbu::StorageKey&            queueKey,
    [[maybe_unused]] int                                partitionId,
    [[maybe_unused]] const bsls::Types::Int64           messageTtl,
    [[maybe_unused]] const int                          maxDeliveryAttempts,
    [[maybe_unused]] const mqbconfm::StorageDefinition& storageDef)
{
    return 0;
}

void StorageManager::processStorageEvent(
    [[maybe_unused]] const mqbevt::StorageEvent& event)
{
    // NOTHING
}

void StorageManager::processStorageSyncRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processPartitionSyncStateRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processPartitionSyncDataRequest(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processPartitionSyncDataRequestStatus(
    [[maybe_unused]] const bmqp_ctrlmsg::ControlMessage& message,
    [[maybe_unused]] mqbnet::ClusterNode*                source)
{
    // NOTHING
}

void StorageManager::processRecoveryEvent(
    [[maybe_unused]] const mqbevt::RecoveryEvent& event)
{
    // NOTHING
}

void StorageManager::processReceiptEvent(
    [[maybe_unused]] const bmqp::Event&   event,
    [[maybe_unused]] mqbnet::ClusterNode* source)
{
    // NOTHING
}

void StorageManager::processPrimaryStatusAdvisory(
    [[maybe_unused]] const bmqp_ctrlmsg::PrimaryStatusAdvisory& advisory,
    [[maybe_unused]] mqbnet::ClusterNode*                       source)
{
    // NOTHING
}

void StorageManager::processReplicaStatusAdvisory(
    [[maybe_unused]] int                             partitionId,
    [[maybe_unused]] mqbnet::ClusterNode*            source,
    [[maybe_unused]] bmqp_ctrlmsg::NodeStatus::Value status)
{
    // NOTHING
}

void StorageManager::processShutdownEvent()
{
    // NOTHING
}

void StorageManager::applyForEachQueue(
    [[maybe_unused]] int                 partitionId,
    [[maybe_unused]] const QueueFunctor& functor) const
{
    // NOTHING
}

void StorageManager::processCommand(
    [[maybe_unused]] mqbcmd::StorageResult*        result,
    [[maybe_unused]] const mqbcmd::StorageCommand& command)
{
    return;
}

void StorageManager::gcUnrecognizedDomainQueues()
{
    // NOTHING
}

int StorageManager::purgeQueueOnDomain(
    [[maybe_unused]] mqbcmd::StorageResult* result,
    [[maybe_unused]] const bsl::string&     domainName)
{
    return 0;
}

// ACCESSORS
mqbi::Dispatcher::ProcessorHandle
StorageManager::processorForPartition([[maybe_unused]] int partitionId) const
{
    return mqbi::Dispatcher::k_INVALID_PROCESSOR_HANDLE;
}

bool StorageManager::isStorageEmpty([[maybe_unused]] const bmqt::Uri& uri,
                                    [[maybe_unused]] int partitionId) const
{
    return true;
}

mqbs::FileStore&
StorageManager::fileStore([[maybe_unused]] int partitionId) const
{
    BSLS_ASSERT_INVOKE_NORETURN("Unimplemented");
}

bmqu::UniquePtr<mqbi::StorageManagerIterator>
StorageManager::getIterator([[maybe_unused]] int partitionId) const
{
    return bmqu::UniquePtr<mqbi::StorageManagerIterator>();
}

}  // close package namespace
}  // close enterprise namespace
