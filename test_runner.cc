#include "test_runner.hpp"

#include <chrono>
#include <cstdlib>

#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "absl/time/clock.h"
#include "absl/flags/flag.h"

#include "perftest_config.hpp"
#include "metrics_tracker.hpp"

using namespace std::chrono;

ABSL_FLAG(absl::Duration, run_duration, absl::Minutes(1), "Length of interval used to calculate p50, etc");

TestRunner::TestRunner(PerftestConfig config) : config_(config)
{
}

std::unique_ptr<GcsClient> TestRunner::CreateClient()
{
    std::unique_ptr<GcsClient> client;
    switch (config_.clientAPI())
    {
    case GRPC_DIRECTPATH:
        client = GcsClient::MakeDirectpathClient(config_.universe(), config_.bucket());
        break;
    case JSON:
        client = GcsClient::MakeJSONClient(config_.universe(), config_.bucket());
    }
    return client;
}

void TestRunner::Run(PrometheusReporter *reporter)
{
    MetricsTracker metrics_tracker;
    std::unique_ptr<GcsClient> client = CreateClient();

    auto run_end_time = absl::Now() + absl::GetFlag(FLAGS_run_duration);

    long success_count = 0;
    long call_count = 0;
    while (absl::Now() < run_end_time)
    {

        auto start = high_resolution_clock::now();
        bool success = false;
        switch (config_.operation())
        {
        case READ:
            success = client->ReadObject(config_.object());
            break;
        case ONESHOT_WRITE:
            success = client->OneShotWriteObject(absl::StrCat(config_.object(), "_", rand()), config_.write_length());
            break;
        case RESUMABLE_WRITE:
            success = client->ResumablyWriteObject(absl::StrCat(config_.object(), "_", rand()), config_.write_length());
            break;
        case QUERY_WRITE_STATUS:
            // TODO
            break;
        }

        auto stop = high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = stop - start;
        std::cerr << "duration: " << duration;

        if (success)
        {
            success_count++;
            metrics_tracker.ReportSuccess(duration);
        }
        call_count++;
    }
    reporter->ReportSuccesses(success_count);
    reporter->ReportCalls(call_count);
    reporter->RecordSuccessP50(metrics_tracker.Quantile(.5));
    reporter->RecordSuccessP90(metrics_tracker.Quantile(.9));
    return;
}