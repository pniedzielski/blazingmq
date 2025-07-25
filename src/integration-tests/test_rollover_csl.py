# Copyright 2025 Bloomberg Finance L.P.
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Testing rollover of CSL file.
"""

import blazingmq.dev.it.testconstants as tc
from blazingmq.dev.it.fixtures import (  # pylint: disable=unused-import
    Cluster,
    cluster,
    order,
    test_logger,
    tweak,
)
from blazingmq.dev.it.util import wait_until
import glob

pytestmark = order(4)

default_app_ids = ["foo", "bar", "baz"]
timeout = 5


class TestRolloverCSL:
    @tweak.cluster.partition_config.max_cslfile_size(2000)
    def test_csl_cleanup(self, cluster: Cluster, domain_urls: tc.DomainUrls):
        """
        Test that rolling over CSL cleans up the old file.
        """
        leader = cluster.last_known_leader
        proxy = next(cluster.proxy_cycle())
        domain_priority = domain_urls.domain_priority

        producer = proxy.create_client("producer")

        cluster._logger.info("Start to write to clients")

        csl_files_before_rollover = glob.glob(
            str(cluster.work_dir.joinpath(leader.name, "storage")) + "/*csl*"
        )
        assert len(csl_files_before_rollover) == 1

        # opening 10 queues would cause a rollover
        for i in range(0, 10):
            producer.open(
                f"bmq://{domain_priority}/q{i}", flags=["write,ack"], succeed=True
            )
            producer.close(f"bmq://{domain_priority}/q{i}", succeed=True)

        csl_files_after_rollover = glob.glob(
            str(cluster.work_dir.joinpath(leader.name, "storage")) + "/*csl*"
        )

        assert leader.outputs_regex(r"Log '.*' closed and cleaned up.", 10)

        assert len(csl_files_after_rollover) == 1
        assert csl_files_before_rollover[0] != csl_files_after_rollover[0]

    @tweak.cluster.partition_config.max_cslfile_size(2000)
    @tweak.cluster.queue_operations.keepalive_duration_ms(1000)
    def test_rollover_queue_assignments(
        self, cluster: Cluster, domain_urls: tc.DomainUrls
    ):
        """
        Test that queue and appId information are preserved across rollover of
        CSL file, even after cluster restart.

        1. Producer opens a fanout queue and posts a message.
        2. By opening and GC'ing queues, cause the CSL file to rollover.
        3. Verify that queue and appId information are preserved, after cluster
           restart, by opening a consumer for each appId of the fanout queue
           and trying to read the message.
        """
        du = domain_urls
        leader = cluster.last_known_leader
        proxy = next(cluster.proxy_cycle())
        self.producer = proxy.create_client("producer")
        self.producer.open(
            f"bmq://{du.domain_fanout}/q0", flags=["write,ack"], succeed=True
        )
        self.producer.post(
            f"bmq://{du.domain_fanout}/q0", ["msg1"], succeed=True, wait_ack=True
        )

        # Cause three QueueAssignmentAdvisories and QueueUnassignedAdvisories to be written to the CSL.  These records will be erased during rollover.
        for i in range(0, 3):
            self.producer.open(
                f"bmq://{du.domain_priority}/q{i}", flags=["write,ack"], succeed=True
            )
            self.producer.close(f"bmq://{du.domain_priority}/q{i}", succeed=True)

        # Assigning these two queues will cause rollover
        self.producer.open(
            f"bmq://{du.domain_priority}/q_last", flags=["write,ack"], succeed=True
        )
        self.producer.open(
            f"bmq://{du.domain_priority}/q_last_2", flags=["write,ack"], succeed=True
        )

        # do not wait for success, otherwise the following capture will fail
        leader.force_gc_queues(block=False)

        assert leader.outputs_regex(r"Rolling over from log with logId", timeout)

        # Rollover and queueUnAssignmentAdvisory interleave
        assert leader.outputs_regex(r"queueUnAssignmentAdvisory", timeout)

        cluster.restart_nodes()
        # For a standard cluster, states have already been restored as part of
        # leader re-election.
        if cluster.is_single_node:
            self.producer.wait_state_restored()

        consumers = {}

        for app_id in default_app_ids:
            consumer = next(cluster.proxy_cycle()).create_client(app_id)
            consumers[app_id] = consumer
            consumer.open(
                f"bmq://{du.domain_fanout}/q0?id={app_id}",
                flags=["read"],
                succeed=True,
            )

        for app_id in default_app_ids:
            test_logger.info(f"Check if {app_id} still has 1 message")
            assert wait_until(
                lambda: len(
                    consumers[app_id].list(
                        f"bmq://{du.domain_fanout}/q0?id={app_id}", block=True
                    )
                )
                == 1,
                3,
            )
