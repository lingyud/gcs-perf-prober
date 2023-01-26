
#include "google/cloud/storage/client.h"
#include "google/cloud/storage/grpc_plugin.h"

#include <iostream>
#include <vector>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/gateway.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "prometheus_reporter.hpp"
#include "perftest_config.hpp"
#include "gcs_client.hpp"
#include "test_runner.hpp"

ABSL_FLAG(bool, push_to_prometheus, false, "Whether to push results");
ABSL_FLAG(std::string, prometheus_host, "34.173.12.152", "Prometheus Push Host");
ABSL_FLAG(std::string, prometheus_port, "9091", "Prometheus Push Port");
ABSL_FLAG(bool, run_once, false, "Whether to exit after first successful run, for testing, etc");

namespace gc = ::google::cloud;
namespace gcs = gc::storage;
namespace gcs_experimental = gc::storage_experimental;

int main(int argc, char **argv)
{
    absl::ParseCommandLine(argc, argv);

    std::optional<PerftestConfig> config = PerftestConfig::LoadConfig();
    if (!config)
    {
        return 1;
    }

    PrometheusReporter prometheus_reporter(absl::GetFlag(FLAGS_prometheus_host), absl::GetFlag(FLAGS_prometheus_port), config->scenario(), config->universe_str(), GcsClient::GRPCVersion(), GcsClient::GCSClientVersion());

    do
    {
        TestRunner testRunner(*config);
        testRunner.Run(&prometheus_reporter);

        prometheus_reporter.Summarize();
        if (absl::GetFlag(FLAGS_push_to_prometheus))
        {
            prometheus_reporter.Push();
        }
    } while (!absl::GetFlag(FLAGS_run_once));

    return 0;
}