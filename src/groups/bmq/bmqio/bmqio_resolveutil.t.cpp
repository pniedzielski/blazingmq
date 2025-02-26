// Copyright 2023 Bloomberg Finance L.P.
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

// bmqio_resolveutil.t.cpp                                            -*-C++-*-
#include <bmqio_resolveutil.h>

// TEST DRIVER
#include <bmqtst_testhelper.h>

// BDE
#include <bsl_string.h>
#include <bsl_vector.h>
#include <bslmf_assert.h>
#include <bsls_assert.h>

#include <ntsa_error.h>
#include <ntsa_ipaddress.h>

// CONVENIENCE
using namespace BloombergLP;
using namespace bsl;
using namespace bmqio;

// ============================================================================
//                                    TESTS
// ----------------------------------------------------------------------------

static void test1_breathingTest()
{
    bmqtst::TestHelper::printTestName("RESOLVE UTILITIES - BREATHING TEST");

    bsl::string       hostname(bmqtst::TestHelperUtil::allocator());
    ntsa::Ipv4Address hostIp;

    BMQTST_ASSERT(hostname.empty());

    {
        PVV("GET HOSTNAME");

        ntsa::Error error = bmqio::ResolveUtil::getHostname(&hostname);

        BMQTST_ASSERT_EQ(error.code(), ntsa::Error::e_OK);
        BMQTST_ASSERT(!hostname.empty());

        PVV(hostname);
    }

    {
        PVV("GET PRIMARY ADDRESS");

        ntsa::Error error = bmqio::ResolveUtil::getIpAddress(&hostIp,
                                                             hostname);

        BMQTST_ASSERT_EQ(error.code(), ntsa::Error::e_OK);
        BMQTST_ASSERT_NE(hostIp, ntsa::Ipv4Address());

        PVV(hostIp);
    }

    {
        PVV("GET DOMAIN NAME");

        bsl::string domainName(bmqtst::TestHelperUtil::allocator());
        ntsa::Error error = bmqio::ResolveUtil::getDomainName(
            &domainName,
            ntsa::IpAddress(hostIp));
        BMQTST_ASSERT_EQ(error.code(), ntsa::Error::e_OK);
        BMQTST_ASSERT(!domainName.empty());

        PVV(domainName);
    }

    {
        PVV("GET ADDRESSES")

        bsl::vector<ntsa::IpAddress> addresses(
            bmqtst::TestHelperUtil::allocator());
        ntsa::Error                  error =
            bmqio::ResolveUtil::getIpAddress(&addresses, "www.wikipedia.org");

        BMQTST_ASSERT_EQ(error.code(), ntsa::Error::e_OK);
        BMQTST_ASSERT_GT(addresses.size(), 0U);

        for (bsl::vector<ntsa::IpAddress>::const_iterator it =
                 addresses.begin();
             it != addresses.end();
             ++it) {
            PVV(*it);
        }
    }

    {
        PVV("GET LOCAL ADDRESSES")

        bsl::vector<ntsa::IpAddress> addresses(
            bmqtst::TestHelperUtil::allocator());
        ntsa::Error error = bmqio::ResolveUtil::getLocalIpAddress(&addresses);

        BMQTST_ASSERT_EQ(error.code(), ntsa::Error::e_OK);
        BMQTST_ASSERT_GT(addresses.size(), 0U);

        ntsa::IpAddress loopback("127.0.0.1");
        bool            foundLoopback = false;

        for (bsl::vector<ntsa::IpAddress>::const_iterator it =
                 addresses.begin();
             it != addresses.end();
             ++it) {
            PVV(*it);
            if (it->equals(loopback)) {
                foundLoopback = true;
            }
        }

        BMQTST_ASSERT(foundLoopback);
    }
}

// ============================================================================
//                                 MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    TEST_PROLOG(bmqtst::TestHelper::e_DEFAULT);

    switch (_testCase) {
    case 0:
    case 1: test1_breathingTest(); break;
    default: {
        cerr << "WARNING: CASE '" << _testCase << "' NOT FOUND." << endl;
        bmqtst::TestHelperUtil::testStatus() = -1;
    } break;
    }

    // Internally, 'ntsu::ResolverUtil::getIpAddress()' rountine uses vector
    // with default allocator so we can check here only for global allocations.
    TEST_EPILOG(bmqtst::TestHelper::e_CHECK_GBL_ALLOC);
}
