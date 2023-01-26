#include "prometheus_reporter.hpp"

#include <string>
#include <iostream>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/gateway.h>
#include <prometheus/counter.h>

PrometheusReporter::PrometheusReporter(std::string host, std::string port,
                                       std::string scenario, std::string universe, std::string grpc_version, std::string gcs_client_version) : registry_(std::make_shared<prometheus::Registry>()),
                                                                                                                         gateway_(host, port, "gcs-perf-prober", {}),
                                                                                                                         labels_({{"testname", scenario},
                                                                                                                            {"grpc_client_version", grpc_version},
                                                                                                                            {"universe", universe},
                                                                                                                            {"gcs_client_version", gcs_client_version}}),
                                                                                                                         success_p50_gauge_family_(prometheus::BuildGauge().Name("success_p50").Register(*registry_)),
                                                                                                                         success_p50_gauge_(success_p50_gauge_family_.Add(labels_)),
                                                                                                                         success_p90_gauge_family_(prometheus::BuildGauge().Name("success_p90").Register(*registry_)),
                                                                                                                         success_p90_gauge_(success_p90_gauge_family_.Add(labels_)),
                                                                                                                         success_count_family_(prometheus::BuildGauge().Name("success_count").Register(*registry_)),
                                                                                                                         success_count_(success_count_family_.Add(labels_)),
                                                                                                                         request_count_family_(prometheus::BuildGauge().Name("request_count").Register(*registry_)),
                                                                                                                         request_count_(request_count_family_.Add(labels_))
{
    gateway_.RegisterCollectable(registry_);
}

void PrometheusReporter::RecordSuccessP50(double latency)
{
    success_p50_gauge_.Set(latency);
}

void PrometheusReporter::RecordSuccessP90(double latency)
{
    success_p90_gauge_.Set(latency);
}

void PrometheusReporter::ReportSuccesses(long count)
{
    success_count_.Set(count);
}

void PrometheusReporter::ReportCalls(long count)
{
    request_count_.Set(count);
}

void PrometheusReporter::Summarize()
{
    std::cerr << "------------" << std::endl
              << "Summary:" << std::endl
              << "  p50: " << success_p50_gauge_.Value() << std::endl
              << "  p90: " << success_p90_gauge_.Value() << std::endl
              << "  Successful requests: " << success_count_.Value() << std::endl
              << "  Errors: " << (request_count_.Value() - success_count_.Value()) << std::endl;
}

bool PrometheusReporter::Push()
{
    int code = gateway_.Push();
    if (code != 200)
    {
        std::cerr << "Prometheus push failed with status " << code << std::endl;
        return false;
    }
    else
    {
        std::cerr << "Metrics pushed." << std::endl;
        return true;
    }
    
}
