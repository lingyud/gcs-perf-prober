#include <stmpct/ckms_tq.hpp>

#include <chrono>
using namespace std::chrono;

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/grpc_plugin.h"

#include <iostream>
#include <vector>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/gateway.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "metrics_tracker.hpp"
#include "prometheus_reporter.hpp"
#include "perftest_config.hpp"
#include "gcs_client.hpp"

ABSL_FLAG(bool, push_to_prometheus, false, "Whether to push results");

namespace gc = ::google::cloud;
namespace gcs = gc::storage;
namespace gcs_experimental = gc::storage_experimental;

int main(int argc, char **argv)
{
    absl::ParseCommandLine(argc, argv);

    std::optional<PerftestConfig> config = PerftestConfig::LoadConfig();
    if (!config) {
        return 1;
    }

    std::unique_ptr<GcsClient> client;
    switch (config->clientAPI())
    {
    case GRPC_DIRECTPATH:
        client = GcsClient::MakeDirectpathClient(config->universe(), config->bucket());
        break;
    case JSON:
        client = GcsClient::MakeJSONClient(config->universe(), config->bucket());
    }

    MetricsTracker metrics_tracker;
    PrometheusReporter prometheus_reporter("127.0.0.1", "9091",config->scenario(), client->GRPCVersion(), client->GCSClientVersion());


    for (int i = 0; i < 100; ++i)
    {
        auto start = high_resolution_clock::now();
        bool success = client->ReadObject(config->object());
        auto stop = high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = stop - start;

        if (success)
        {
            metrics_tracker.ReportSuccess(duration.count());
            prometheus_reporter.ReportSuccessfulCalls(1);
            prometheus_reporter.ReportCalls(1);
        }
        else
        {
            metrics_tracker.ReportError(duration.count());
            prometheus_reporter.ReportCalls(1);
        }

        if (i % 25 == 0)
            std::cout << "Read " << i << " objects, " << duration.count() << "ms." << std::endl;
    }

    prometheus_reporter.RecordSuccessP50(metrics_tracker.Quantile(.5));

    if (absl::GetFlag(FLAGS_push_to_prometheus)) {
        prometheus_reporter.Push();
    }
    return 0;
}