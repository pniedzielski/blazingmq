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

#ifndef INCLUDED_MQBMOCK_STORAGEMANAGER
#define INCLUDED_MQBMOCK_STORAGEMANAGER

//@PURPOSE: Provide a mock implementation of the 'mqbi::StorageManager'
// interface.
//
//@CLASSES:
//  mqbmock::StorageManager: Mock impl of 'mqbi::StorageManager'
//
//@DESCRIPTION: This component provides a mock implementation,
// 'mqbmock::StorageManager', of the 'mqbi::StorageManager' protocol.
//
/// Thread Safety
///-------------
// The 'mqbmock::StorageManager' object is not thread safe.

// MQB
#include <mqbi_storagemanager.h>

// BDE
#include <bsl_memory.h>
#include <bsl_ostream.h>
#include <bsls_keyword.h>

namespace BloombergLP {
namespace mqbmock {

// ====================
// class StorageManager
// ====================

/// Mock implementation of `mqbi::StorageManager` interface.
class StorageManager final : public mqbi::StorageManager {
  public:
    // CREATORS

    /// Create a new `StorageManager` instance
    StorageManager();

    /// Destructor
    ~StorageManager() override;

    // MANIPULATORS

    /// Start this storage manager.  Return 0 on success, or a non-zero rc
    /// otherwise, populating the specified `errorDescription` with a
    /// description of the error.
    ///
    /// THREAD: Executed by the cluster's dispatcher thread.
    int start(bsl::ostream& errorDescription) override;

    /// Stop this storage manager.
    void stop() override;

    /// Initialize the queue key info map based on information in the specified
    /// `clusterState`.  Note that this method should only be called once;
    /// subsequent calls will be ignored.
    void
    initializeQueueKeyInfoMap(const mqbc::ClusterState& clusterState) override;

    /// Register a queue with the specified `uri`, `queueKey` and
    /// `partitionId`, having the specified `appIdKeyPairs`, and belonging
    /// to the specified `domain`.  Load into the specified `storage` the
    /// associated queue storage created.
    ///
    /// THREAD: Executed by the Client's dispatcher thread.
    void registerQueue(const bmqt::Uri&        uri,
                       const mqbu::StorageKey& queueKey,
                       int                     partitionId,
                       const AppInfos&         appIdKeyPairs,
                       mqbi::Domain*           domain) override;

    /// Synchronously unregister the queue with the specified `uri` from the
    /// specified `partitionId`.  Behavior is undefined unless this routine
    /// is invoked from the cluster dispatcher thread.
    ///
    /// THREAD: Executed by the Client's dispatcher thread.
    void unregisterQueue(const bmqt::Uri& uri, int partitionId) override;

    /// Configure the fanout queue having specified `uri` and `queueKey`,
    /// assigned to the specified `partitionId` to have the specified
    /// `addedIdKeyPairs` appId/appKey pairs added and `removedIdKeyPairs`
    /// appId/appKey pairs removed.  Return zero on success, and non-zero
    /// value otherwise.  Behavior is undefined unless this function is
    /// invoked at the primary node.  Behavior is also undefined unless the
    /// queue is configured in fanout mode.
    ///
    /// THREAD: Executed by the Queue's dispatcher thread.
    int updateQueuePrimary(const bmqt::Uri& uri,
                           int              partitionId,
                           const AppInfos&  addedIdKeyPairs,
                           const AppInfos&  removedIdKeyPairs) override;

    /// Behavior is undefined unless the specified 'partitionId' is in range
    /// and the specified 'primaryNode' is not null.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void setPrimaryForPartition(int                  partitionId,
                                mqbnet::ClusterNode* primaryNode,
                                unsigned int         primaryLeaseId) override;

    /// Behavior is undefined unless the specified 'partitionId' is in range
    /// and the specified 'primaryNode' is not null.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void clearPrimaryForPartition(int                  partitionId,
                                  mqbnet::ClusterNode* primary) override;

    /// Set the primary status of the specified 'partitionId' to the specified
    /// 'value'.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void setPrimaryStatusForPartition(
        int                                partitionId,
        bmqp_ctrlmsg::PrimaryStatus::Value value) override;

    /// Stop all Partition FSMs.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void stopPFSMs() override;

    /// Apply `RST_UNKNOWN` event to the Partition FSM for the specified
    /// `partitionId`.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void detectPrimaryLossInPFSM(int partitionId) override;

    /// Apply DETECT_SelfPrimary event to Partition FSM using the specified
    /// `partitionId`, `primaryNode`, `primaryLeaseId`.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void detectSelfPrimaryInPFSM(int                  partitionId,
                                 mqbnet::ClusterNode* primaryNode,
                                 unsigned int         primaryLeaseId) override;

    /// Apply DETECT_SelfReplica event to Partition FSM using the specified
    /// `partitionId`, `primaryNode` and `primaryLeaseId`.
    ///
    /// THREAD: Executed in cluster dispatcher thread.
    void detectSelfReplicaInPFSM(int                  partitionId,
                                 mqbnet::ClusterNode* primaryNode,
                                 unsigned int         primaryLeaseId) override;

    /// Process primary state request received from the specified `source`
    /// with the specified `message`.
    void
    processPrimaryStateRequest(const bmqp_ctrlmsg::ControlMessage& message,
                               mqbnet::ClusterNode* source) override;

    /// Process replica state request received from the specified `source`
    /// with the specified `message`.
    void
    processReplicaStateRequest(const bmqp_ctrlmsg::ControlMessage& message,
                               mqbnet::ClusterNode* source) override;

    /// Process replica data request received from the specified `source`
    /// with the specified `message`.
    void processReplicaDataRequest(const bmqp_ctrlmsg::ControlMessage& message,
                                   mqbnet::ClusterNode* source) override;

    int
    configureStorage(bsl::ostream&                      errorDescription,
                     bsl::shared_ptr<mqbi::Storage>*    out,
                     const bmqt::Uri&                   uri,
                     const mqbu::StorageKey&            queueKey,
                     int                                partitionId,
                     const bsls::Types::Int64           messageTtl,
                     const int                          maxDeliveryAttempts,
                     const mqbconfm::StorageDefinition& storageDef) override;

    /// Executed in cluster dispatcher thread.
    void processStorageEvent(const mqbevt::StorageEvent& event) override;

    /// Executed by any thread.
    void processStorageSyncRequest(const bmqp_ctrlmsg::ControlMessage& message,
                                   mqbnet::ClusterNode* source) override;

    /// Executed by any thread.
    void processPartitionSyncStateRequest(
        const bmqp_ctrlmsg::ControlMessage& message,
        mqbnet::ClusterNode*                source) override;

    /// Executed by any thread.
    void processPartitionSyncDataRequest(
        const bmqp_ctrlmsg::ControlMessage& message,
        mqbnet::ClusterNode*                source) override;

    /// Executed by any thread.
    void processPartitionSyncDataRequestStatus(
        const bmqp_ctrlmsg::ControlMessage& message,
        mqbnet::ClusterNode*                source) override;

    /// Executed in cluster dispatcher thread.
    void processRecoveryEvent(const mqbevt::RecoveryEvent& event) override;

    /// Executed in IO thread.
    void processReceiptEvent(const bmqp::Event&   event,
                             mqbnet::ClusterNode* source) override;

    /// Executed by any thread.
    void processPrimaryStatusAdvisory(
        const bmqp_ctrlmsg::PrimaryStatusAdvisory& advisory,
        mqbnet::ClusterNode*                       source) override;

    /// Executed by any thread.
    void processReplicaStatusAdvisory(
        int                             partitionId,
        mqbnet::ClusterNode*            source,
        bmqp_ctrlmsg::NodeStatus::Value status) override;

    /// Executed by any thread.
    void processShutdownEvent() override;

    /// Invoke the specified `functor` with each queue associated to the
    /// partition identified by the specified `partitionId` if that
    /// partition has been successfully opened.  The behavior is undefined
    /// unless invoked from the queue thread corresponding to `partitionId`.
    void applyForEachQueue(int                 partitionId,
                           const QueueFunctor& functor) const override;

    /// Process the specified `command`, and load the result to the
    /// specified `result`.  This function can be invoked from any thread,
    /// and will block until the potentially asynchronous operation is
    /// complete.
    void processCommand(mqbcmd::StorageResult*        result,
                        const mqbcmd::StorageCommand& command) override;

    /// GC the queues from unrecognized domains, if any.
    void gcUnrecognizedDomainQueues() override;

    /// Purge the queues on a given domain.
    int purgeQueueOnDomain(mqbcmd::StorageResult* result,
                           const bsl::string&     domainName) override;

    // ACCESSORS

    /// Return the processor handle in charge of the specified
    /// `partitionId`.  The behavior is undefined if `partitionId` does not
    /// represent a valid partition id.
    mqbi::Dispatcher::ProcessorHandle
    processorForPartition(int partitionId) const override;

    /// Return true if the queue having specified `uri` and assigned to the
    /// specified `partitionId` has no messages, false in any other case.
    /// Behavior is undefined unless this routine is invoked from cluster
    /// dispatcher thread.
    bool isStorageEmpty(const bmqt::Uri& uri, int partitionId) const override;

    /// Return partition corresponding to the specified `partitionId`.  The
    /// behavior is undefined if `partitionId` does not represent a valid
    /// partition id.
    mqbs::FileStore& fileStore(int partitionId) const override;

    /// Return a StorageManagerIterator for the specified `partitionId`.
    bslma::ManagedPtr<mqbi::StorageManagerIterator>
    getIterator(int partitionId) const override;
};

}  // close package namespace
}  // close enterprise namespace

#endif
