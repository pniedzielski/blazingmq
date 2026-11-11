// Copyright 2020-2023 Bloomberg Finance L.P.
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

#include <mqbcmd_humanprinter.h>

#include <mqbscm_version.h>
// MQB
#include <mqbcmd_messages.h>

// BMQ
#include <bmqp_queueid.h>

#include <bmqu_memoutstream.h>
#include <bmqu_outstreamformatsaver.h>
#include <bmqu_printutil.h>

// BDE
#include <bdlb_print.h>
#include <bdlb_string.h>
#include <bsl_algorithm.h>
#include <bsl_cstddef.h>
#include <bsl_iomanip.h>
#include <bsl_ios.h>
#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_string.h>
#include <bsl_vector.h>
#include <bslim_printer.h>
#include <bsls_assert.h>
#include <bsls_types.h>

namespace BloombergLP {
namespace mqbcmd {

namespace {

void printCapacityMeter(bsl::ostream&        os,
                        const CapacityMeter& capacityMeter,
                        int                  level,
                        int                  spacesPerLevel)
{
    bdlb::Print::newlineAndIndent(os, level, spacesPerLevel);

    if (capacityMeter.isDisabled()) {
        os << "[ ** DISABLED ** ]";
        return;  // RETURN
    }

    os << capacityMeter.name() << ":"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Messages: [current: "
       << bmqu::PrintUtil::prettyNumber(capacityMeter.numMessages())
       << ", limit: "
       << bmqu::PrintUtil::prettyNumber(capacityMeter.messageCapacity());
    if (capacityMeter.numMessagesReserved() != 0) {
        os << ", reserved: "
           << bmqu::PrintUtil::prettyNumber(
                  capacityMeter.numMessagesReserved());
    }
    os << "]" << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Bytes   : [current: "
       << bmqu::PrintUtil::prettyBytes(capacityMeter.numBytes()) << ", limit: "
       << bmqu::PrintUtil::prettyBytes(capacityMeter.byteCapacity());
    if (capacityMeter.numBytesReserved() != 0) {
        os << ", reserved: "
           << bmqu::PrintUtil::prettyBytes(capacityMeter.numBytesReserved());
    }
    os << "]";

    if (!capacityMeter.parent().isNull()) {
        printCapacityMeter(os,
                           capacityMeter.parent().value(),
                           level,
                           spacesPerLevel);
    }
}

void printQueueStatus(bsl::ostream&         os,
                      const StorageContent& storageContent,
                      int                   level,
                      int                   spacesPerLevel)
{
    if (storageContent.storages().empty()) {
        return;  // RETURN
    }

    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << " QueueKey    Partition    NumMsgs    NumBytes"
       << "                QueueUri";

    typedef bsl::vector<StorageQueueInfo> Storages;
    const Storages&                       storages = storageContent.storages();
    bmqu::OutStreamFormatSaver            fmtSaver(os);

    for (const auto& storage : storages) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << storage.queueKey() << " " << bsl::setfill(' ') << bsl::setw(7)
           << storage.partitionId() << "      " << bsl::setfill(' ')
           << bsl::setw(8)
           << bmqu::PrintUtil::prettyNumber(storage.numMessages()) << "  "
           << bsl::setfill(' ') << bsl::setw(10)
           << bmqu::PrintUtil::prettyBytes(storage.numBytes()) << "    "
           << storage.queueUri()
           << (storage.isPersistent() ? "" : "  (non-persistent)");
    }
}

void printDomainInfo(bsl::ostream&     os,
                     const DomainInfo& domainInfo,
                     int               level,
                     int               spacesPerLevel)
{
    bdlb::Print::indent(os, level, spacesPerLevel);
    os << "Domain:" << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-------"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Name ............: " << domainInfo.name()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Config ..........:\n"
       << domainInfo.configJson()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Cluster .........: " << domainInfo.clusterName()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Capacity Meter ..: ";
    printCapacityMeter(os,
                       domainInfo.capacityMeter(),
                       level + 2,
                       spacesPerLevel);
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Queues ..........: "
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "------------------"
       << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
       << "ActiveQueues ..: " << domainInfo.queueUris().size();
    {
        bmqu::OutStreamFormatSaver fmtSaver(os);
        for (size_t i = 0; i < domainInfo.queueUris().size(); ++i) {
            const bsl::string& queueUri = domainInfo.queueUris()[i];
            os << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
               << " " << bsl::setfill(' ') << bsl::setw(3) << i << ". "
               << queueUri;
        }
    }
    os << "\n\n";

    bdlb::Print::indent(os, level, spacesPerLevel);
    os << "Storage ......."
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Number of assigned queue-storages: "
       << domainInfo.storageContent().storages().size();

    printQueueStatus(os,
                     domainInfo.storageContent(),
                     level + 2,
                     spacesPerLevel);
}

void printClusterList(bsl::ostream& os, const ClusterList& clusterList)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);
    os << "The following clusters are active:";

    for (const ClusterInfo& cluster : clusterList.clusters()) {
        os << "\n    ";

        bsl::string locality(Locality::toString(cluster.locality()));
        bdlb::String::toUpper(&locality);
        os << "[" << bsl::left << bsl::setw(6) << locality << "] "
           << cluster.name() << ":";

        for (const ClusterNode& node : cluster.nodes()) {
            os << "\n        " << bsl::left << bsl::setw(20) << node.hostName()
               << bsl::setw(7) << node.nodeId() << bsl::setw(14)
               << node.dataCenter();
        }
        os << "\n";
    }
}

void printBrokerConfig(bsl::ostream& os, const BrokerConfig& brokerConfig)
{
    os << brokerConfig.asJSON();
}

void printValue(bsl::ostream& os, const Value& value)
{
    switch (value.selectionId()) {
    case Value::SELECTION_ID_THE_INTEGER: {
        os << value.theInteger();
    } break;
    case Value::SELECTION_ID_THE_BOOL: {
        os << value.theBool();
    } break;
    case Value::SELECTION_ID_THE_DOUBLE: {
        os << value.theDouble();
    } break;
    case Value::SELECTION_ID_THE_STRING: {
        os << value.theString();
    } break;
    case Value::SELECTION_ID_THE_DATE: {
        os << value.theDate();
    } break;
    case Value::SELECTION_ID_THE_TIME: {
        os << value.theTime();
    } break;
    case Value::SELECTION_ID_THE_DATETIME: {
        os << value.theDatetime();
    } break;
    default: {
        BSLS_ASSERT_SAFE(false && "Unsupported value");
        os << "Unsupported value: " << value;
    }
    }
}

void printTunable(bsl::ostream& os, const Tunable& tunable)
{
    os << tunable.name() << " = ";
    printValue(os, tunable.value());
}

void printTunableList(bsl::ostream& os, const Tunables& tunables)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);

    // Compute the longest tunable name, for pretty-alignment
    bsl::size_t longestName = 0;
    for (const Tunable& tunable : tunables.tunables()) {
        longestName = bsl::max(longestName, tunable.name().length());
    }

    // Print all tunables
    for (const Tunable& tunable : tunables.tunables()) {
        os << tunable.name()
           << bsl::setw(longestName - tunable.name().length() + 4)
           << bsl::setfill('.') << ": ";
        // NOTE: '+ 4' in order to have at least 2 '.' (and then the ": ")
        printValue(os, tunable.value());
        os << "\n    " << tunable.description() << "\n";
    }
}

void printTunableConfirmation(bsl::ostream&              os,
                              const TunableConfirmation& tunableConfirmation)
{
    os << tunableConfirmation.name() << " set to ";
    printValue(os, tunableConfirmation.newValue());
    os << " from ";
    printValue(os, tunableConfirmation.oldValue());
}

void printPurgedQueues(bsl::ostream& os, const PurgedQueues& purgedQueueList)
{
    if (purgedQueueList.queues().empty()) {
        os << "No queue purged.";
        return;  // RETURN
    }

    // const_cast so we can inline sort without the need to make a deep
    // copy of the vector
    bsl::vector<PurgeQueueResult>& queues =
        const_cast<bsl::vector<PurgeQueueResult>&>(purgedQueueList.queues());

    // Sort by number of bytes purged, in descending order.
    bsl::sort(
        queues.begin(),
        queues.end(),
        [](const PurgeQueueResult& lhs, const PurgeQueueResult& rhs) {
            return (lhs.isQueueValue() ? lhs.queue().numBytesPurged() : 0) >
                   (rhs.isQueueValue() ? rhs.queue().numBytesPurged() : 0);
        });

    bmqu::MemOutStream stream;
    bmqu::MemOutStream errorStream;
    bsls::Types::Int64 totalBytes    = 0;
    bsls::Types::Int64 totalMessages = 0;

    for (const PurgeQueueResult& result : queues) {
        if (result.isErrorValue()) {
            errorStream << "\n" << result.error().message();
            continue;  // CONTINUE
        }

        const PurgedQueueDetails& queue = result.queue();
        stream << bsl::endl
               << "  " << bsl::setfill(' ') << bsl::setw(10) << bsl::right
               << bmqu::PrintUtil::prettyBytes(queue.numBytesPurged()) << "  "
               << bsl::setfill(' ') << bsl::setw(10) << bsl::right
               << bmqu::PrintUtil::prettyNumber(queue.numMessagesPurged())
               << bsl::left << "  " << queue.appKey() << "  "
               << bsl::setfill(' ') << bsl::setw(16) << bsl::right
               << queue.appId() << bsl::left << "  " << queue.queueUri();
        totalBytes += queue.numBytesPurged();
        totalMessages += queue.numMessagesPurged();
    }

    os << "Purged " << bmqu::PrintUtil::prettyNumber(totalMessages)
       << " message(s) for a total of "
       << bmqu::PrintUtil::prettyBytes(totalBytes) << " from " << queues.size()
       << " queue(s):\n"
       << "  " << "  NumBytes" << "  " << "   NumMsgs" << "  " << "    AppKey"
       << "  " << "           AppId" << "  " << "QueueName" << stream.str();

    if (!errorStream.str().empty()) {
        os << "\nErrors encountered: " << errorStream.str();
    }
}

void printQueueContents(bsl::ostream& os, const QueueContents& queueContents)
{
    bmqu::MemOutStream msgListing;
    bsls::Types::Int64 totalBytes = 0;

    auto it = queueContents.messages().cbegin();
    for (bsls::Types::Int64 sequenceNo = queueContents.offset();
         it != queueContents.messages().cend();
         ++sequenceNo, ++it) {
        totalBytes += it->sizeBytes();
        msgListing << bsl::setw(5) << sequenceNo << ": " << bsl::setw(32)
                   << it->guid() << "  " << bsl::setw(10)
                   << bmqu::PrintUtil::prettyBytes(it->sizeBytes()) << "  "
                   << it->arrivalTimestamp() << "\n";
    }

    bsls::Types::Int64 numMessages = queueContents.messages().size();
    bsls::Types::Int64 endInterval = 0;
    if (queueContents.offset() + numMessages >=
        queueContents.totalQueueMessages()) {
        endInterval = queueContents.totalQueueMessages() - 1;
    }
    else {
        endInterval = queueContents.offset() + numMessages - 1;
    }

    os << "Printing " << numMessages << " message(s) ["
       << queueContents.offset() << "-" << endInterval << " / "
       << bmqu::PrintUtil::prettyNumber(queueContents.totalQueueMessages())
       << "] (total: " << bmqu::PrintUtil::prettyBytes(totalBytes) << ")\n"
       << "       GUID                              Size        Timestamp "
          "(UTC)\n"
       << msgListing.str();
}

void printMessage(bsl::ostream& os, const Message& message)
{
    os << "GUID                              Size        Timestamp (UTC)\n"
       << bsl::setw(32) << message.guid() << "  " << bsl::setw(10)
       << bmqu::PrintUtil::prettyBytes(message.sizeBytes()) << "  "
       << message.arrivalTimestamp();
}

void printFileStoreSummary(bsl::ostream&           os,
                           const FileStoreSummary& summary,
                           int                     partitionId,
                           int                     level,
                           int                     spacesPerLevel)
{
    using namespace bmqu::PrintUtil;

    os << indent(level, spacesPerLevel) << "Partition [" << partitionId
       << "]:" << newlineAndIndent(level, spacesPerLevel) << "----------------"
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Primary Node: " << summary.primaryNodeDescription()
       << ", PSN: [ primaryLeaseId = " << summary.primaryLeaseId()
       << " sequenceNumber = " << summary.sequenceNum() << "]"
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "IsAvailable: " << bsl::boolalpha << summary.isAvailable()
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Total maintained file sets: " << summary.fileSets().size() << "\n"
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Aliased payload count: ";

    typedef bsl::vector<FileSet> FileSets;
    const FileSets&              fileSets = summary.fileSets();
    for (const auto& fileSet : fileSets) {
        os << newlineAndIndent(level + 2, spacesPerLevel)
           << fileSet.dataFileName() << " : "
           << prettyNumber(static_cast<bsls::Types::Int64>(
                  fileSet.aliasedBlobBufferCount()));
    }

    const ActiveFileSet& activeFileSet = summary.activeFileSet();
    BSLS_ASSERT_SAFE(activeFileSet.dataFile().sizeBytes() > 0);
    BSLS_ASSERT_SAFE(activeFileSet.journalFile().sizeBytes() > 0);
    os << '\n'
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Active file set details: "
       << newlineAndIndent(level + 2, spacesPerLevel)
       << "Data file size    (used/total): "
       << prettyBytes(activeFileSet.dataFile().positionBytes()) << " / "
       << prettyBytes(activeFileSet.dataFile().sizeBytes()) << "  ("
       << ((activeFileSet.dataFile().positionBytes() * 100) /
           activeFileSet.dataFile().sizeBytes())
       << " %). Outstanding bytes: "
       << prettyNumber(static_cast<bsls::Types::Int64>(
              activeFileSet.dataFile().outstandingBytes()))
       << newlineAndIndent(level + 2, spacesPerLevel)
       << "Journal file size (used/total): "
       << prettyBytes(activeFileSet.journalFile().positionBytes()) << " / "
       << prettyBytes(activeFileSet.journalFile().sizeBytes()) << "  ("
       << ((activeFileSet.journalFile().positionBytes() * 100) /
           activeFileSet.journalFile().sizeBytes())
       << " %). Outstanding bytes: "
       << prettyNumber(static_cast<bsls::Types::Int64>(
              activeFileSet.journalFile().outstandingBytes()))
       << newlineAndIndent(level + 2, spacesPerLevel);

    // QList will be deprecated when CSL mode with FSM workflow is enabled.
    //
    // TODO CSL Remove this code block
    if (activeFileSet.qlistFile().sizeBytes() > 0) {
        os << "Qlist file size   (used/total): "
           << prettyBytes(activeFileSet.qlistFile().positionBytes()) << " / "
           << prettyBytes(activeFileSet.qlistFile().sizeBytes()) << "  ("
           << ((activeFileSet.qlistFile().positionBytes() * 100) /
               activeFileSet.qlistFile().sizeBytes())
           << " %). Outstanding bytes: "
           << prettyNumber(static_cast<bsls::Types::Int64>(
                  activeFileSet.qlistFile().outstandingBytes()));
    }

    os << '\n'
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Total mapped size      : "
       << prettyBytes(summary.totalMappedBytes())
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Num outstanding records: "
       << prettyNumber(
              static_cast<bsls::Types::Int64>(summary.numOutstandingRecords()))
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Num unreceipted messages: "
       << prettyNumber(static_cast<bsls::Types::Int64>(
              summary.numUnreceiptedMessages()))
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Current Nagle count: "
       << prettyNumber(
              static_cast<bsls::Types::Int64>(summary.naglePacketCount()))
       << '\n'
       << newlineAndIndent(level + 1, spacesPerLevel)
       << "Number of assigned queue-storages: "
       << prettyNumber(static_cast<bsls::Types::Int64>(
              summary.storageContent().storages().size()));

    printQueueStatus(os, summary.storageContent(), level + 2, spacesPerLevel);
}

void printClusterStorageSummary(bsl::ostream&                os,
                                const ClusterStorageSummary& summary,
                                int                          level,
                                int                          spacesPerLevel)
{
    bdlb::Print::indent(os, level, spacesPerLevel);
    os << "Storage Summary:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "----------------"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Storage location: " << summary.clusterFileStoreLocation() << "\n\n";

    typedef bsl::vector<FileStore> FileStores;
    const FileStores&              fileStores = summary.fileStores();
    for (const auto& fileStore : fileStores) {
        if (fileStore.state() == FileStoreState::CLOSED) {
            os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
               << "Partition [" << fileStore.partitionId() << "]: NOT OPEN.";
            continue;  // CONTINUE
        }
        else if (fileStore.state() == FileStoreState::STOPPING) {
            os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
               << "Partition [" << fileStore.partitionId() << "]: STOPPING.";
            continue;  // CONTINUE
        }

        printFileStoreSummary(os,
                              fileStore.summary(),
                              fileStore.partitionId(),
                              level + 1,
                              spacesPerLevel);
        os << "\n";
    }
}

void printClusterQueueHelper(bsl::ostream&             os,
                             const ClusterQueueHelper& clusterQueueHelper,
                             int                       level,
                             int                       spacesPerLevel)
{
    os << "ClusterQueueHelper :\n"
       << "--------------------"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "cluster......................: " << clusterQueueHelper.clusterName()
       << " (" << Locality::toString(clusterQueueHelper.locality()) << ")"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "nbQueues.....................: " << clusterQueueHelper.numQueues()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "nbQueueKeys..................: "
       << clusterQueueHelper.numQueueKeys()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "nbPendingReopenQueueRequests.: "
       << clusterQueueHelper.numPendingReopenQueueRequests() << "\n";

    // Domains
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Domains :"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "---------";

    bsl::size_t                        maxDomainLength = 0;
    typedef bsl::vector<ClusterDomain> ClusterDomains;
    const ClusterDomains& clusterDomains = clusterQueueHelper.domains();
    for (const auto& clusterDomain : clusterDomains) {
        maxDomainLength = bsl::max(maxDomainLength,
                                   clusterDomain.name().length());
    }

    for (const auto& clusterDomain : clusterDomains) {
        os << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
           << clusterDomain.name()
           << bsl::setw(maxDomainLength - clusterDomain.name().length())
           << bsl::setfill('.')
           << ": numAssignedQueues=" << clusterDomain.numAssignedQueues();
        if (!clusterDomain.loaded()) {
            os << " (domain not yet loaded)";
        }
        os << "\n";
    }

    // Queues
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Queues :"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "--------";

    typedef bsl::vector<ClusterQueue> ClusterQueues;
    const ClusterQueues& clusterQueues = clusterQueueHelper.queues();
    for (const auto& clusterQueue : clusterQueues) {
        // Queue URI
        os << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
           << clusterQueue.uri();

        // Info
        os << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "inFlightContexts.: " << clusterQueue.numInFlightContexts()
           << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "assigned.........: " << bsl::boolalpha
           << clusterQueue.isAssigned()
           << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "primaryAvailable..: " << bsl::boolalpha
           << clusterQueue.isPrimaryAvailable()
           << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "id...............: "
           << bmqp::QueueId::QueueIdInt(clusterQueue.id())
           << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "subIds...........: ";

        typedef bsl::vector<SubId> SubIds;
        const SubIds&              subIds = clusterQueue.subIds();
        for (SubIds::const_iterator citer = subIds.cbegin();
             citer != subIds.cend();
             ++citer) {
            os << "(" << citer->subId() << ", " << citer->appId() << ")";

            SubIds::const_iterator citerCopy(citer);
            if ((++citerCopy) != subIds.end()) {
                os << ", ";
            }
        }

        os << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "partitionId......: " << clusterQueue.partitionId();
        if (!clusterQueue.primaryNodeDescription().isNull()) {
            os << clusterQueue.primaryNodeDescription().value();
        }
        os << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "key..............: " << clusterQueue.key()
           << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "queue............: "
           << (clusterQueue.isCreated() ? "created" : "*NOT* created");

        // Contexts
        os << bmqu::PrintUtil::newlineAndIndent(level + 3, spacesPerLevel)
           << "nbContext........: " << clusterQueue.contexts().size();
        for (size_t ctxId = 0; ctxId != clusterQueue.contexts().size();
             ++ctxId) {
            const Context& context = clusterQueue.contexts()[ctxId];
            os << bmqu::PrintUtil::newlineAndIndent(level + 4, spacesPerLevel)
               << "#" << (ctxId + 1) << ": "
               << "[handle parameters: " << context.queueHandleParametersJson()
               << "]";
        }
    }
}

void printResourceUsageMonitor(bsl::ostream&               os,
                               const ResourceUsageMonitor& usageMonitor,
                               int                         level,
                               int                         spacesPerLevel)
{
    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << usageMonitor.state() << " " << "[Messages ("
       << usageMonitor.messagesState()
       << "): " << bmqu::PrintUtil::prettyNumber(usageMonitor.numMessages())
       << " ("
       << bmqu::PrintUtil::prettyNumber(static_cast<bsls::Types::Int64>(
              usageMonitor.messagesLowWatermarkRatio() *
              usageMonitor.messagesCapacity()))
       << " - "
       << bmqu::PrintUtil::prettyNumber(static_cast<bsls::Types::Int64>(
              usageMonitor.messagesHighWatermarkRatio() *
              usageMonitor.messagesCapacity()))
       << " - "
       << bmqu::PrintUtil::prettyNumber(usageMonitor.messagesCapacity())
       << "), " << "Bytes (" << usageMonitor.bytesState()
       << "): " << bmqu::PrintUtil::prettyBytes(usageMonitor.numBytes())
       << " ("
       << bmqu::PrintUtil::prettyBytes(usageMonitor.bytesLowWatermarkRatio() *
                                           usageMonitor.bytesCapacity(),
                                       2)
       << " - "
       << bmqu::PrintUtil::prettyBytes(usageMonitor.bytesHighWatermarkRatio() *
                                           usageMonitor.bytesCapacity(),
                                       2)
       << " - " << bmqu::PrintUtil::prettyBytes(usageMonitor.bytesCapacity())
       << ")] ";
}

void printQueueHandleSubStream(bsl::ostream&               os,
                               const QueueHandleSubStream& subStream,
                               int                         level,
                               int                         spacesPerLevel)
{
    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "subId .................: " << subStream.subId();
    if (!subStream.appId().isNull()) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << "appId .................: " << subStream.appId().value();
    }

    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "Stream Parameters .....: " << subStream.parametersJson()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "UnconfirmedMonitors ...:"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "--------";

    for (const auto& monitor : subStream.unconfirmedMonitors()) {
        printResourceUsageMonitor(os, monitor, 0, -1);
    }

    if (!subStream.numUnconfirmedMessages().isNull()) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << "UnconfirmedMessages ...: "
           << bmqu::PrintUtil::prettyNumber(static_cast<bsls::Types::Int64>(
                  subStream.numUnconfirmedMessages().value()));
    }

    os << '\n';
}

void printQueueHandle(bsl::ostream&      os,
                      const QueueHandle& handle,
                      int                level,
                      int                spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "Handle Parameters .....: " << handle.parametersJson()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "IsClientClusterMember .: " << bsl::boolalpha
       << handle.isClientClusterMember();

    typedef bsl::vector<QueueHandleSubStream> SubStreams;
    const SubStreams&                         subStreams = handle.subStreams();
    for (const auto& subStream : subStreams) {
        printQueueHandleSubStream(os, subStream, level, spacesPerLevel);
    }
}

void printQueueHandles(bsl::ostream&                   os,
                       const bsl::vector<QueueHandle>& handles,
                       int                             level,
                       int                             spacesPerLevel)
{
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << handles.size() << " handles:\n";
    for (unsigned int idx = 1; idx <= handles.size(); ++idx) {
        os << bmqu::PrintUtil::indent(level + 2, spacesPerLevel) << idx << ". "
           << handles[idx - 1].clientDescription() << "\n";
        printQueueHandle(os, handles[idx - 1], level + 3, spacesPerLevel);
    }
}

void printQueueState(bsl::ostream&     os,
                     const QueueState& state,
                     int               level,
                     int               spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "State:" << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "------"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "URI ...............: " << state.uri()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Handle Parameters .: " << state.handleParametersJson()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Stream Parameters .: " << state.streamParametersJson()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Id/Key ............: ";

    if (state.id() == bmqp::QueueId::k_UNASSIGNED_QUEUE_ID) {
        os << "unassigned";
    }
    else if (state.id() == bmqp::QueueId::k_PRIMARY_QUEUE_ID) {
        os << "primary";
    }
    else {
        os << state.id();
    }

    os << " ~ " << state.key()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "PartitionId .......: " << state.partitionId()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Storage ...........: ";

    if (!state.storage().isNull()) {
        const QueueStorage& storage = state.storage().value();
        os << bmqu::PrintUtil::prettyNumber(
                  static_cast<bsls::Types::Int64>(storage.numMessages()))
           << " messages, " << bmqu::PrintUtil::prettyBytes(storage.numBytes())
           << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
           << "Num virtual storages: " << storage.virtualStorages().size();

        if (storage.virtualStorages().size()) {
            typedef bsl::vector<VirtualStorage> VirtualStorages;
            const VirtualStorages& virtualStorages = storage.virtualStorages();
            for (const auto& virtualStorage : virtualStorages) {
                bdlb::Print::newlineAndIndent(os, level + 2, spacesPerLevel);
                os << virtualStorage.appId()
                   << " : [appKey: " << virtualStorage.appKey()
                   << ", numMessages: "
                   << bmqu::PrintUtil::prettyNumber(
                          static_cast<bsls::Types::Int64>(
                              virtualStorage.numMessages()))
                   << "]";
            }
        }
    }
    else {
        os << "none";
    }

    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Capacity Meter ....:";
    if (!state.capacityMeter().isNull()) {
        printCapacityMeter(os,
                           state.capacityMeter().value(),
                           level + 2,
                           spacesPerLevel);
    }
    else {
        os << "none";
    }

    os << "\n"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "QueueHandleCatalog:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-------------------";
    printQueueHandles(os, state.handles(), level + 1, spacesPerLevel);
    os << "\n";
}

void printMessageGroupIdManagerIndex(
    bsl::ostream&                     os,
    const MessageGroupIdManagerIndex& messageGroupIdManagerIndex,
    int                               level,
    int                               spacesPerLevel)
{
    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "Lru:" << "\n";

    {
        bslim::Printer printer(&os, level, spacesPerLevel);
        printer.start();

        typedef bsl::vector<LeastRecentlyUsedGroupId> LruGroupIds;
        const LruGroupIds&                            lruGroupIds =
            messageGroupIdManagerIndex.leastRecentlyUsedGroupIds();
        for (const auto& lruGroupId : lruGroupIds) {
            bsl::ostringstream key;
            bsl::ostringstream timeDelta;
            key << lruGroupId.clientDescription() << "#"
                << lruGroupId.msgGroupId();
            bmqu::PrintUtil::prettyTimeInterval(
                timeDelta,
                lruGroupId.lastSeenDeltaNanoseconds());
            printer.printAttribute(key.str().c_str(), timeDelta.str());
        }
        printer.end();
    }

    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "Handles:" << "\n";
    {
        bslim::Printer printer(&os, level, spacesPerLevel);
        printer.start();

        typedef bsl::vector<ClientMsgGroupsCount> NumMsgGroupsPerClient;
        const NumMsgGroupsPerClient&              numMsgGroupsPerClient =
            messageGroupIdManagerIndex.numMsgGroupsPerClient();
        for (const auto& clientGroups : numMsgGroupsPerClient) {
            printer.printAttribute(clientGroups.clientDescription().c_str(),
                                   clientGroups.numMsgGroupIds());
        }
        printer.end();
    }
}

void printMessageGroupIdHelper(
    bsl::ostream&               os,
    const MessageGroupIdHelper& messageGroupIdHelper,
    int                         level,
    int                         spacesPerLevel)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);

    os << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "MessageGroupIdManager:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "timeout........: " << messageGroupIdHelper.timeoutNanoseconds()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "maxMsgGroupIds.: " << messageGroupIdHelper.maxMsgGroupIds()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "rebalance......: " << bsl::boolalpha
       << messageGroupIdHelper.isRebalanceOn()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "status.........: [";

    printMessageGroupIdManagerIndex(os,
                                    messageGroupIdHelper.status(),
                                    level + 1,
                                    spacesPerLevel);

    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "]";
}

void printAppState(bsl::ostream&   os,
                   const AppState& appState,
                   int             level,
                   int             spacesPerLevel)
{
    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "Consumers .........: " << appState.numConsumers()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "RedeliveryList ....: " << appState.redeliveryListLength()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "RoundRobinRouter ..:\n";

    appState.roundRobinRouter().print(os, level + 1, spacesPerLevel);
}

void printFanoutQueueEngine(bsl::ostream&            os,
                            const FanoutQueueEngine& queueEngine,
                            int                      level,
                            int                      spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "FanoutQueueEngine (" << queueEngine.mode()
       << "):" << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-----------------";

    if (queueEngine.maxConsumers() != 0) {
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << " (maxConsumers: " << queueEngine.maxConsumers() << ")";
    }

    bsl::size_t                        maxAppIdLength = 0;
    typedef bsl::vector<ConsumerState> ConsumerStates;
    const ConsumerStates& consumerStates = queueEngine.consumerStates();
    for (const auto& consumerState : consumerStates) {
        maxAppIdLength = bsl::max(maxAppIdLength,
                                  consumerState.appId().length());
    }

    for (const auto& consumerState : consumerStates) {
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << bsl::setw(maxAppIdLength) << bsl::setfill(' ') << bsl::left
           << consumerState.appId() << ": status=" << consumerState.status();

        if (!consumerState.isAtEndOfStorage().isNull()) {
            os << ", StorageIter.atEnd=" << bsl::boolalpha
               << consumerState.isAtEndOfStorage().value();
        }

        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << bsl::setw(maxAppIdLength) << bsl::setfill('-') << "-";

        printAppState(os, consumerState.appState(), level + 2, spacesPerLevel);
    }
    os << '\n';
}

void printRelayQueueEngine(bsl::ostream&           os,
                           const RelayQueueEngine& queueEngine,
                           int                     level,
                           int                     spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "RelayQueueEngine:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-----------------";

    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Num SubStreams ....: " << queueEngine.numSubstreams();

    if (queueEngine.numSubstreams()) {
        typedef bsl::vector<RelayQueueEngineSubStream> SubStreams;
        const SubStreams& subStreams = queueEngine.subStreams();
        for (const auto& subStream : subStreams) {
            os << bmqu::PrintUtil::newlineAndIndent(level + 2, spacesPerLevel)
               << "[" << subStream.appId() << "] - [" << subStream.appKey()
               << "] : " << subStream.numMessages();
        }
    }

    typedef bsl::vector<AppState> AppStates;
    const AppStates&              appStates = queueEngine.appStates();
    for (const auto& appState : appStates) {
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "App: " << appState.appId()
           << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "-----------------";
        printAppState(os, appState, level + 2, spacesPerLevel);
    }

    os << "\n";
}

void printQueueEngine(bsl::ostream&      os,
                      const QueueEngine& queueEngine,
                      int                level,
                      int                spacesPerLevel)
{
    if (queueEngine.isFanoutValue()) {
        printFanoutQueueEngine(os,
                               queueEngine.fanout(),
                               level,
                               spacesPerLevel);
    }
    else if (queueEngine.isRelayValue()) {
        printRelayQueueEngine(os, queueEngine.relay(), level, spacesPerLevel);
    }
    else {
        os << "Unknown queue engine type: " << queueEngine;
    }
}

void printLocalQueue(bsl::ostream&     os,
                     const LocalQueue& localQueue,
                     int               level,
                     int               spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "LocalQueue:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-----------\n";
    printQueueEngine(os, localQueue.queueEngine(), level + 1, spacesPerLevel);
}

void printRemoteStream(bsl::ostream&           os,
                       const RemoteStreamInfo& info,
                       int                     level,
                       int                     spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "[Id: " << info.id() << ", state: " << info.state()
       << ", genCount: " << info.genCount() << "]\n";
}
void printRemoteQueue(bsl::ostream&      os,
                      const RemoteQueue& remoteQueue,
                      int                level,
                      int                spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "RemoteQueue:"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "------------\n";

    os << bmqu::PrintUtil::indent(level + 1, spacesPerLevel)
       << "Pending PUTs .......................: "
       << remoteQueue.numPendingPuts() << " items\n";
    os << bmqu::PrintUtil::indent(level + 1, spacesPerLevel)
       << "Pending CONFIRMs ...................: "
       << remoteQueue.numPendingConfirms() << " items\n";
    os << bmqu::PrintUtil::indent(level + 1, spacesPerLevel)
       << "Has PUSH expiration timer scheduled : "
       << (remoteQueue.isPushExpirationTimerScheduled() ? "yes" : "no")
       << "\n";

    os << bmqu::PrintUtil::indent(level + 1, spacesPerLevel)
       << "Upstream generation count ..........: "
       << remoteQueue.numUpstreamGeneration() << "\n";

    size_t num = remoteQueue.streams().size();
    os << bmqu::PrintUtil::indent(level + 1, spacesPerLevel)
       << "Streams ............................: " << "\n";

    typedef bsl::vector<RemoteStreamInfo> Streams;
    const Streams&                        streams = remoteQueue.streams();
    for (Streams::const_iterator cit = streams.begin(); cit != streams.end();
         ++cit, --num) {
        printRemoteStream(os, *cit, level + 2, spacesPerLevel);
    }

    os << "\n";
    printQueueEngine(os, remoteQueue.queueEngine(), level + 1, spacesPerLevel);
}

void printQueueInternals(bsl::ostream&         os,
                         const QueueInternals& queueInternals,
                         int                   level,
                         int                   spacesPerLevel)
{
    os << "Queue Internals\n"
       << "---------------\n";

    // Print State Object
    printQueueState(os, queueInternals.state(), level + 1, spacesPerLevel);

    // Print Queue Object
    const Queue& queue = queueInternals.queue();
    if (queue.isLocalQueueValue()) {
        printLocalQueue(os, queue.localQueue(), level + 1, spacesPerLevel);
    }
    else if (queue.isRemoteQueueValue()) {
        printRemoteQueue(os, queue.remoteQueue(), level + 1, spacesPerLevel);
    }
    else {
        os << "Uninitialized queue !!\n";
    }
}

void printQueuesInfo(bsl::ostream&         os,
                     const StorageContent& queuesInfo,
                     int                   level,
                     int                   spacesPerLevel)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);

    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "Queues"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel) << "------"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel);

    typedef bsl::vector<StorageQueueInfo> QueuesInfo;
    const QueuesInfo&                     queues = queuesInfo.storages();
    os << "Num Queues: " << queues.size();
    if (!queues.empty()) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << " QueueKey    Partition    Internal QueueId          QueueUri";
    }

    for (const auto& queue : queues) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << queue.queueKey() << "       " << queue.partitionId()
           << "           " << bsl::setfill(' ') << bsl::setw(10)
           << bmqp::QueueId::QueueIdInt(queue.internalQueueId()) << "       "
           << queue.queueUri();
    }
}

void printPartitionsInfo(bsl::ostream&         os,
                         const PartitionsInfo& partitionsInfo,
                         int                   level,
                         int                   spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "Partitions"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "----------";

    const bsl::vector<PartitionInfo>& partitions = partitionsInfo.partitions();
    for (unsigned int i = 0; i < partitions.size(); ++i) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << "PartitionId: " << i
           << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "Num Queues    :" << " mapped ("
           << partitions[i].numQueuesMapped() << ")," << " active ("
           << partitions[i].numActiveQueues() << ")";

        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel);
        if (!partitions[i].primaryNode().isNull()) {
            os << "Primary Node   : " << partitions[i].primaryNode().value();
        }
        else {
            os << "Primary Node   : "
               << "[ ** NONE ** ]";
        }
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "Primary LeaseId: " << partitions[i].primaryLeaseId()
           << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "Primary Status : " << partitions[i].primaryStatus();
    }
}

void printElectorInfo(bsl::ostream&      os,
                      const ElectorInfo& electorInfo,
                      int                level,
                      int                spacesPerLevel)
{
    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "Elector"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "-------\n"
       << bmqu::PrintUtil::indent(level, spacesPerLevel)
       << "Self State            : " << electorInfo.electorState()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "Leader Node           : " << electorInfo.leaderNode()
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "LSN: [ electorTerm = "
       << electorInfo.leaderMessageSequence().electorTerm()
       << " sequenceNumber = "
       << electorInfo.leaderMessageSequence().sequenceNumber() << " ]"
       << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
       << "Leader Status         : " << electorInfo.leaderStatus();
}

void printNodeStatuses(bsl::ostream&       os,
                       const NodeStatuses& nodeStatuses,
                       int                 level,
                       int                 spacesPerLevel)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);

    os << bmqu::PrintUtil::indent(level, spacesPerLevel) << "Nodes";
    os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel) << "-----";
    typedef bsl::vector<ClusterNodeInfo> Nodes;
    const Nodes&                         nodes = nodeStatuses.nodes();
    for (const auto& node : nodes) {
        os << bmqu::PrintUtil::newlineAndIndent(level, spacesPerLevel)
           << node.description();
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel);
        if (node.isAvailable().isNull()) {
            os << "IsConnected: self";
        }
        else {
            os << "IsConnected: " << bsl::boolalpha
               << node.isAvailable().value();
        }
        os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
           << "Node Status: " << node.status();

        if (node.primaryForPartitionIds().size() > 0) {
            os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
               << "Primary for partitions: [ ";
            for (unsigned int i = 0; i < node.primaryForPartitionIds().size();
                 ++i) {
                os << node.primaryForPartitionIds()[i];
                if (i != (node.primaryForPartitionIds().size() - 1)) {
                    os << ", ";
                }
            }
            os << " ]";
        }
    }
}

void printClusterStatus(bsl::ostream&        os,
                        const ClusterStatus& clusterStatus,
                        int                  level,
                        int                  spacesPerLevel)
{
    os << "Cluster ('" << clusterStatus.name() << "') :\n";
    os << "---------";
    // Add extra '-' padding
    for (size_t i = 0; i < clusterStatus.name().size() + 5; ++i) {
        os << "-";
    }
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Cluster name  : " << clusterStatus.description();
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Cluster size  : " << clusterStatus.nodeStatuses().nodes().size()
       << " node(s)";
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Self Node     : " << clusterStatus.selfNodeDescription();
    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Is Healthy    : " << (clusterStatus.isHealthy() ? "Yes" : "No");

    os << "\n\n";
    printNodeStatuses(os,
                      clusterStatus.nodeStatuses(),
                      level + 1,
                      spacesPerLevel);
    os << "\n\n";
    printElectorInfo(os,
                     clusterStatus.electorInfo(),
                     level + 1,
                     spacesPerLevel);
    os << "\n\n";
    printPartitionsInfo(os,
                        clusterStatus.partitionsInfo(),
                        level + 1,
                        spacesPerLevel);
    os << "\n\n";
    printQueuesInfo(os, clusterStatus.queuesInfo(), level + 1, spacesPerLevel);
    os << "\n\n";
    printClusterStorageSummary(os,
                               clusterStatus.clusterStorageSummary(),
                               level + 1,
                               spacesPerLevel);
}

void printClusterProxyStatus(bsl::ostream&             os,
                             const ClusterProxyStatus& clusterProxyStatus,
                             int                       level,
                             int                       spacesPerLevel)
{
    bmqu::OutStreamFormatSaver fmtSaver(os);

    os << "Cluster\n"
       << "-------"
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Cluster name: " << clusterProxyStatus.description()
       << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Active node : ";

    if (!clusterProxyStatus.activeNodeDescription().isNull()) {
        os << clusterProxyStatus.activeNodeDescription().value();
    }
    else {
        os << "* none *";
    }

    os << bmqu::PrintUtil::newlineAndIndent(level + 1, spacesPerLevel)
       << "Is Healthy  : " << (clusterProxyStatus.isHealthy() ? "Yes" : "No")
       << "\n";

    // Print nodes status
    os << "\n";
    printNodeStatuses(os,
                      clusterProxyStatus.nodeStatuses(),
                      level,
                      spacesPerLevel);

    // Print queue status
    os << "\n";
    os << "\n";
    printQueuesInfo(os,
                    clusterProxyStatus.queuesInfo(),
                    level,
                    spacesPerLevel);
}

void printHelpCommands(bsl::ostream& os, const Help& helpCommands)
{
    typedef bsl::vector<CommandSpec> Commands;
    const Commands&                  commands = helpCommands.commands();
    bsl::string                      separator1;
    bsl::string                      separator2;

    if (helpCommands.isPlumbing()) {
        separator1 = "";
        separator2 = " - ";
    }
    else {
        separator1 = "    ";
        separator2 = "\n        ";
        os << "This process responds to the following CMD subcommands:\n";
    }

    for (const auto& command : commands) {
        os << separator1 << command.command() << separator2
           << command.description() << "\n";
    }
}

}  // close anonymous namespace

// -------------------
// struct HumanPrinter
// -------------------

bsl::ostream& HumanPrinter::print(bsl::ostream& os,
                                  const Result& result,
                                  int           level,
                                  int           spacesPerLevel)
{
    if (result.isErrorValue()) {
        os << result.error().message();
    }
    else if (result.isSuccessValue()) {
        os << "SUCCESS";
    }
    else if (result.isHelpValue()) {
        printHelpCommands(os, result.help());
    }
    else if (result.isDomainInfoValue()) {
        printDomainInfo(os, result.domainInfo(), level, spacesPerLevel);
    }
    else if (result.isClusterListValue()) {
        printClusterList(os, result.clusterList());
    }
    else if (result.isBrokerConfigValue()) {
        printBrokerConfig(os, result.brokerConfig());
    }
    else if (result.isTunableValue()) {
        printTunable(os, result.tunable());
    }
    else if (result.isTunablesValue()) {
        printTunableList(os, result.tunables());
    }
    else if (result.isTunableConfirmationValue()) {
        printTunableConfirmation(os, result.tunableConfirmation());
    }
    else if (result.isStatsValue()) {
        os << result.stats();
    }
    else if (result.isPurgedQueuesValue()) {
        printPurgedQueues(os, result.purgedQueues());
    }
    else if (result.isQueueContentsValue()) {
        printQueueContents(os, result.queueContents());
    }
    else if (result.isMessageValue()) {
        printMessage(os, result.message());
    }
    else if (result.isClusterStorageSummaryValue()) {
        printClusterStorageSummary(os,
                                   result.clusterStorageSummary(),
                                   level,
                                   spacesPerLevel);
    }
    else if (result.isStorageContentValue()) {
        printQueueStatus(os, result.storageContent(), level, spacesPerLevel);
    }
    else if (result.isClusterQueueHelperValue()) {
        printClusterQueueHelper(os,
                                result.clusterQueueHelper(),
                                level,
                                spacesPerLevel);
    }
    else if (result.isQueueInternalsValue()) {
        printQueueInternals(os,
                            result.queueInternals(),
                            level,
                            spacesPerLevel);
    }
    else if (result.isMessageGroupIdHelperValue()) {
        printMessageGroupIdHelper(os,
                                  result.messageGroupIdHelper(),
                                  level,
                                  spacesPerLevel);
    }
    else if (result.isClusterStatusValue()) {
        printClusterStatus(os, result.clusterStatus(), level, spacesPerLevel);
    }
    else if (result.isClusterProxyStatusValue()) {
        printClusterProxyStatus(os,
                                result.clusterProxyStatus(),
                                level,
                                spacesPerLevel);
    }
    else if (result.isNodeStatusesValue()) {
        printNodeStatuses(os, result.nodeStatuses(), level, spacesPerLevel);
    }
    else if (result.isElectorInfoValue()) {
        printElectorInfo(os, result.electorInfo(), level, spacesPerLevel);
    }
    else if (result.isPartitionsInfoValue()) {
        printPartitionsInfo(os,
                            result.partitionsInfo(),
                            level,
                            spacesPerLevel);
    }
    else {
        BSLS_ASSERT_SAFE(false && "Unsupported result");
    }
    return os;
}

bsl::ostream&
HumanPrinter::printResponses(bsl::ostream&            os,
                             const RouteResponseList& responseList)
{
    typedef bsl::vector<BloombergLP::mqbcmd::RouteResponse>
        RouteResponseVector;

    RouteResponseVector responses = responseList.responses();

    for (const auto& routeResponse : responses) {
        os << "Response from node \"" << routeResponse.sourceNodeDescription()
           << "\":";
        os << bsl::endl;
        os << routeResponse.response();
        os << bsl::endl;
    }
    return os;
}

}  // close package namespace
}  // close enterprise namespace
