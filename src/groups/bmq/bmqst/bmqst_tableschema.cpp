// Copyright 2022-2023 Bloomberg Finance L.P.
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

#include <bmqst_tableschema.h>

#include <bdlma_localsequentialallocator.h>
#include <bmqscm_version.h>
#include <bsl_functional.h>
#include <bslmf_allocatorargt.h>
#include <bsls_alignedbuffer.h>

namespace BloombergLP {
namespace bmqst {

namespace {

// CONSTANTS
const char DIRECT_NAME[]  = "*direct*";
const char EXPIRED_NAME[] = "*expired*";
const char UNKNOWN_NAME[] = "*unknown*";

// FUNCTIONS
void defaultIdColumn(Value*                 value,
                     const StatContext&     context,
                     int                    level,
                     StatContext::ValueType type)
{
    bdlma::LocalSequentialAllocator<128> seqAlloc;

    bsl::string storage(&seqAlloc);
    storage.reserve(2 * level + 32 + 2);

    // 1. Indentation
    storage.assign(2 * level, ' ');

    // 2. Open quote (optional)
    if (context.isDeleted()) {
        storage += '(';
    }

    // 3. Context name
    switch (type) {
    case StatContext::e_TOTAL_VALUE: [[fallthrough]];
    case StatContext::e_ACTIVE_CHILDREN_TOTAL_VALUE: {
        if (context.hasName()) {
            storage += context.name();
        }
        else {
            storage += bsl::to_string(context.id());
        }
    } break;
    case StatContext::e_DIRECT_VALUE: {
        storage += DIRECT_NAME;
    } break;
    case StatContext::e_EXPIRED_VALUE: {
        storage += EXPIRED_NAME;
    } break;
    default: {
        storage += UNKNOWN_NAME;
    } break;
    }

    // 4. Close quote (optional)
    if (context.isDeleted()) {
        storage += ')';
    }

    value->set(storage);
    value->ownValue();
}

}  // close anonymous namespace

// -----------------------
// class TableSchemaColumn
// -----------------------

// CREATORS
TableSchemaColumn::TableSchemaColumn(const bslstl::StringRef& name,
                                     const ValueFn&           fn,
                                     bslma::Allocator*        basicAllocator)
: d_name(name, basicAllocator)
, d_fn(bsl::allocator_arg, basicAllocator, fn)
{
}

TableSchemaColumn::TableSchemaColumn(const TableSchemaColumn& other,
                                     bslma::Allocator*        basicAllocator)
: d_name(other.d_name, basicAllocator)
, d_fn(bsl::allocator_arg, basicAllocator, other.d_fn)
{
}

// ACCESSORS
const bsl::string& TableSchemaColumn::name() const
{
    return d_name;
}

void TableSchemaColumn::evaluate(Value*                 value,
                                 const StatContext&     context,
                                 int                    level,
                                 StatContext::ValueType type) const
{
    return d_fn(value, context, level, type);
}

// -----------------
// class TableSchema
// -----------------

// CREATORS
TableSchema::TableSchema(bslma::Allocator* basicAllocator)
: d_columns(basicAllocator)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TableSchema::TableSchema(const TableSchema& other,
                         bslma::Allocator*  basicAllocator)
: d_columns(other.d_columns, basicAllocator)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

// MANIPULATORS
TableSchemaColumn& TableSchema::addColumn(const bslstl::StringRef& name,
                                          const Column::ValueFn&   func)
{
    d_columns.emplace_back(name, func);
    return d_columns.back();
}

void TableSchema::addDefaultIdColumn(const bslstl::StringRef& name)
{
    Column::ValueFn columnFn = [](Value*                 value,
                                  const StatContext&     context,
                                  int                    level,
                                  StatContext::ValueType type) {
        defaultIdColumn(value, context, level, type);
    };

    addColumn(name, columnFn);
}

TableSchemaColumn&
TableSchema::addColumn(const bslstl::StringRef& name,
                       int                      statIndex,
                       bsls::Types::Int64 (*func)(const StatValue&))
{
    Column::ValueFn columnFn = [statIndex, func](Value*             value,
                                                 const StatContext& context,
                                                 int,
                                                 StatContext::ValueType type) {
        value->set(func(context.value(type, statIndex)));
    };

    return addColumn(name, columnFn);
}

TableSchemaColumn& TableSchema::addColumn(
    const bslstl::StringRef& name,
    int                      statIndex,
    bsls::Types::Int64 (*func)(const StatValue&,
                               const StatValue::SnapshotLocation&),
    const StatValue::SnapshotLocation& snapshot)
{
    Column::ValueFn columnFn =
        [statIndex, func, snapshot](Value*             value,
                                    const StatContext& context,
                                    int,
                                    StatContext::ValueType type) {
            value->set(func(context.value(type, statIndex), snapshot));
        };

    return addColumn(name, columnFn);
}

TableSchemaColumn& TableSchema::addColumn(
    const bslstl::StringRef& name,
    int                      statIndex,
    bsls::Types::Int64 (*func)(const StatValue&,
                               const StatValue::SnapshotLocation&,
                               const StatValue::SnapshotLocation&),
    const StatValue::SnapshotLocation& snapshot1,
    const StatValue::SnapshotLocation& snapshot2)
{
    Column::ValueFn columnFn = [statIndex, func, snapshot1, snapshot2](
                                   Value*             value,
                                   const StatContext& context,
                                   int,
                                   StatContext::ValueType type) {
        value->set(func(context.value(type, statIndex), snapshot1, snapshot2));
    };

    return addColumn(name, columnFn);
}

TableSchemaColumn& TableSchema::addColumn(const bslstl::StringRef& name,
                                          int                      statIndex,
                                          double (*func)(const StatValue&))
{
    Column::ValueFn columnFn = [statIndex, func](Value*             value,
                                                 const StatContext& context,
                                                 int,
                                                 StatContext::ValueType type) {
        value->set(func(context.value(type, statIndex)));
    };

    return addColumn(name, columnFn);
}

TableSchemaColumn& TableSchema::addColumn(
    const bslstl::StringRef& name,
    int                      statIndex,
    double (*func)(const StatValue&, const StatValue::SnapshotLocation&),
    const StatValue::SnapshotLocation& snapshot)
{
    Column::ValueFn columnFn =
        [statIndex, func, snapshot](Value*             value,
                                    const StatContext& context,
                                    int,
                                    StatContext::ValueType type) {
            value->set(func(context.value(type, statIndex), snapshot));
        };

    return addColumn(name, columnFn);
}

TableSchemaColumn&
TableSchema::addColumn(const bslstl::StringRef& name,
                       int                      statIndex,
                       double (*func)(const StatValue&,
                                      const StatValue::SnapshotLocation&,
                                      const StatValue::SnapshotLocation&),
                       const StatValue::SnapshotLocation& snapshot1,
                       const StatValue::SnapshotLocation& snapshot2)
{
    Column::ValueFn columnFn = [statIndex, func, snapshot1, snapshot2](
                                   Value*             value,
                                   const StatContext& context,
                                   int,
                                   StatContext::ValueType type) {
        value->set(func(context.value(type, statIndex), snapshot1, snapshot2));
    };

    return addColumn(name, columnFn);
}

// ACCESSORS
int TableSchema::numColumns() const
{
    return static_cast<int>(d_columns.size());
}

const TableSchemaColumn& TableSchema::column(int index) const
{
    BSLS_ASSERT(index < numColumns());
    return d_columns[index];
}

}  // close package namespace
}  // close enterprise namespace
