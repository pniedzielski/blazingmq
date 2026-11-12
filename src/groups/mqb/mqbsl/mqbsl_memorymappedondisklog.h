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

#ifndef INCLUDED_MQBSL_MEMORYMAPPEDONDISKLOG
#define INCLUDED_MQBSL_MEMORYMAPPEDONDISKLOG

//@PURPOSE: Implements an on-disk log using the mmap() syscall.
//
//@CLASSES:
//  mqbsl::MemoryMappedOnDiskLogFactory: Class used to create on-disk logs.
//  mqbsl::MemoryMappedOnDiskLog:        On-disk log using the mmap() syscall.
//
//@SEE_ALSO:
//  mqbsi::Log
//  mqbsi::LogFactory
//  mqbsl::OnDiskLog
//
//@DESCRIPTION: 'mqbsl::MemoryMappedOnDiskLog' is an implementation of an
// on-disk log using the mmap() syscall.  For efficiency, it is intended to be
// used solely in an append-only fashion.  Note that this implementation
// memory-maps the log.
//
/// Thread Safety
///-------------
// This component is *NOT* thread safe.

// MQB

#include <mqbs_mappedfiledescriptor.h>
#include <mqbsi_log.h>
#include <mqbsl_ondisklog.h>

#include <bmqu_blob.h>
#include <bmqu_uniqueptr.h>

// BDE
#include <bslma_allocator.h>

namespace BloombergLP {

namespace mqbsl {

// ==================================
// class MemoryMappedOnDiskLogFactory
// ==================================

/// Factory used to create in-memory log instances.
class MemoryMappedOnDiskLogFactory final : public mqbsi::LogFactory {
  private:
    // DATA
    bslma::Allocator* d_allocator_p;

  private:
    // NOT IMPLEMENTED
    MemoryMappedOnDiskLogFactory(const MemoryMappedOnDiskLogFactory&) = delete;
    MemoryMappedOnDiskLogFactory&
    operator=(const MemoryMappedOnDiskLogFactory&) = delete;

  public:
    // CREATORS

    /// Constructor of a `mqbsl::MemoryMappedOnDiskLogFactory` object, using
    /// the specified `allocator` to supply memory.
    MemoryMappedOnDiskLogFactory(bslma::Allocator* allocator);

    /// Destructor.
    ~MemoryMappedOnDiskLogFactory() override;

    // MANIPULATORS

    /// Create a new log using the specified `config`.
    bmqu::UniquePtr<mqbsi::Log>
    create(const mqbsi::LogConfig& config) override;
};

// ===========================
// class MemoryMappedOnDiskLog
// ===========================

/// This class implements an on-disk log using the mmap() syscall.
class MemoryMappedOnDiskLog final : public OnDiskLog {
  private:
    // PRIVATE TYPES
    typedef mqbsi::LogOpResult LogOpResult;
    typedef mqbsi::LogConfig   LogConfig;

    typedef mqbs::MappedFileDescriptor MappedFileDescriptor;

  private:
    // DATA
    bool d_isOpened;  // Whether the log is opened.

    bool d_isReadOnly;  // Whether the log is in read-only
                        // mode.

    bsls::Types::Int64 d_totalNumBytes;
    // Total number of bytes in the log.

    bsls::Types::Int64 d_outstandingNumBytes;
    // Number of outstanding bytes in the
    // log.  Note that it is the onus of
    // the user to invoke
    // 'updateOutstandingNumBytes' properly
    // before overwriting an existing
    // record, since a 'write()' operation
    // will always increment this value by
    // exactly the number of bytes written,
    // regardless of whether an existing
    // record is overwritten.

    Offset d_currentOffset;
    // Current offset of the log's internal
    // write position.

    mqbsi::LogConfig d_config;  // Config of this on-disk log.

    MappedFileDescriptor d_mfd;  // File descriptor to the underlying
                                 // memory-mapped file storing the log.

  private:
    // NOT IMPLEMENTED
    MemoryMappedOnDiskLog(const MemoryMappedOnDiskLog&)            = delete;
    MemoryMappedOnDiskLog& operator=(const MemoryMappedOnDiskLog&) = delete;

  private:
    // PRIVATE MANIPULATORS
    int validateRead(int length, Offset offset) const;
    // Validate that the specified 'length' and 'offset' arguments for a
    // 'read()' or 'alias()' operation are within bounds of the log.
    // Return 0 on success or a negative value LogOpResult otherwise.

    /// Increment the log's internal write position and outstanding bytes by
    /// the specified `writeLength` and update the total number of bytes in
    /// the log if it has grown to a new max.
    void updateInternalState(int writeLength);

  public:
    // CREATORS

    /// Create an instance of memory-mapped on-disk log having the specified
    /// `config`.
    explicit MemoryMappedOnDiskLog(const mqbsi::LogConfig& config);

    /// Destructor
    ~MemoryMappedOnDiskLog() override;

    // MANIPULATORS

    /// Open the log in the mode according to the specified `flags`, and
    /// return 0 on success or a negative value LogOpResult otherwise.  The
    /// `flags` must include exactly zero or one of the following modes:
    /// e_READ_ONLY, or e_CREATE_IF_MISSING (setting both e_READ_ONLY and
    /// e_CREATE_IF_MISSING to true does not make sense).  If e_READ_ONLY is
    /// true, open the log in read-only mode.  If e_CREATE_IF_MISSING is
    /// true, create the log if it does not exist.  Else, return error if it
    /// does not exist.  Note that if e_READ_ONLY is true, `write()` and
    /// `seek()` operations will return failure.  As an additional
    /// guarantee, upon successful completion of `open()`, `currentOffset()`
    /// must point to the end of the log, while `totalNumBytes()` and
    /// `outstandingNumBytes()` must be equal to the size of the log.
    int open(int flags) override;

    /// Close the log, and return 0 on success, or a negative value
    /// LogOpResult on error.
    int close() override;

    /// Move the log's internal write position to the specified `offset`,
    /// and return 0 on success, or a negative value LogOpResult on error.
    /// Note that depending upon a log's implementation, repeatedly using
    /// `seek` to carry out random write operations may incur severe
    /// penalty.  Effort must be made to write sequentially to the log.
    /// Also note that it is the onus of the user of this component to
    /// update the number of outstanding bytes before seeking and
    /// overwriting existing bytes.
    int seek(Offset offset) override;

    /// Increment the number of outstanding bytes in the log by the
    /// specified `value` (can be negative).
    void updateOutstandingNumBytes(bsls::Types::Int64 value) override;

    /// Update the number of outstanding bytes in the log to the specified
    /// `value`.
    void setOutstandingNumBytes(bsls::Types::Int64 value) override;

    Offset write(const void* entry, int offset, int length) override;

    /// Write the specified `length` bytes starting at the specified
    /// `offset` of the specified `entry` into the log's internal write
    /// position.  Return the offset at which the `entry` was written on
    /// success, or a negative value LogOpResult on error.  Note the number
    /// of outstanding bytes in the log will be incremented by exactly
    /// `length` bytes, regardless of whether an existing record is
    /// overwritten.  Therefore, it is the onus of the user to invoke
    /// `updateOutstandingNumBytes` properly before overwriting an existing
    /// record.
    Offset write(const bdlbb::Blob&        entry,
                 const bmqu::BlobPosition& offset,
                 int                       length) override;

    /// Write the specified `section` of the specified `entry` into the
    /// log's internal write position.   Return the offset at which the
    /// `entry` was written on success, or a negative value LogOpResult on
    /// error.  The number of outstanding bytes in the log will be
    /// incremented by exactly the number of bytes in the `section`,
    /// regardless of whether an existing record is overwritten.  Therefore,
    /// it is the onus of the user to invoke `updateOutstandingNumBytes`
    /// properly before overwriting an existing record.
    Offset write(const bdlbb::Blob&       entry,
                 const bmqu::BlobSection& section) override;

    /// Flush any cached data up to the optionally specified `offset` to the
    /// underlying storing mechanism, and return 0 on success, or a negative
    /// value `mqbsi::LogOpResult` on error.  If `offset` is not specified,
    /// all data is flushed.
    int flush(Offset offset = 0) override;

    // ACCESSORS
    int read(void* entry, int length, Offset offset) const override;

    /// Copy the specified `length` bytes starting at the specified `offset`
    /// of the log into the specified `entry`, and return 0 on success, or a
    /// negative value LogOpResult on error.  Behavior is undefined unless
    /// `entry` has space for at least `length` bytes.
    int read(bdlbb::Blob* entry, int length, Offset offset) const override;

    int alias(void** entry, int length, Offset offset) const override;

    /// Load into the specified `entry a reference to the specified `length'
    /// bytes starting at the specified `offset` of the log, and return 0 on
    /// success, or a negative value LogOpResult on error.  Behavior is
    /// undefined unless aliasing is supported.
    int alias(bdlbb::Blob* entry, int length, Offset offset) const override;

    /// Return true if this log is opened, false otherwise.
    bool isOpened() const override;

    /// Return the total number of bytes in the log.
    bsls::Types::Int64 totalNumBytes() const override;

    /// Return the number of outstanding bytes in the log.
    bsls::Types::Int64 outstandingNumBytes() const override;

    /// Return the current offset of the log's internal write position.
    Offset currentOffset() const override;

    /// Return the config of the log.
    const LogConfig& logConfig() const override;

    /// Return true if the log supports aliasing, false otherwise.
    bool supportsAliasing() const override;

    /// Return the config of this on-disk log
    const mqbsi::LogConfig& config() const override;
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

// ---------------------------
// class MemoryMappedOnDiskLog
// ---------------------------

// MANIPULATORS
inline void
MemoryMappedOnDiskLog::updateOutstandingNumBytes(bsls::Types::Int64 value)
{
    d_outstandingNumBytes += value;
}

inline void
MemoryMappedOnDiskLog::setOutstandingNumBytes(bsls::Types::Int64 value)
{
    // PRECONDITIONS
    BSLS_ASSERT_SAFE(value >= 0);

    d_outstandingNumBytes = value;
}

// ACCESSORS
inline bool MemoryMappedOnDiskLog::isOpened() const
{
    return d_isOpened;
}

inline bsls::Types::Int64 MemoryMappedOnDiskLog::totalNumBytes() const
{
    return d_totalNumBytes;
}

inline bsls::Types::Int64 MemoryMappedOnDiskLog::outstandingNumBytes() const
{
    return d_outstandingNumBytes;
}

inline mqbsi::Log::Offset MemoryMappedOnDiskLog::currentOffset() const
{
    return d_currentOffset;
}

inline const mqbsi::LogConfig& MemoryMappedOnDiskLog::logConfig() const
{
    return d_config;
}

inline bool MemoryMappedOnDiskLog::supportsAliasing() const
{
    return true;
}

inline const mqbsi::LogConfig& MemoryMappedOnDiskLog::config() const
{
    return d_config;
}

}  // close package namespace
}  // close enterprise namespace

#endif
