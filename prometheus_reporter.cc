#include "prometheus_reporter.hpp"

#include <string>
#include <iostream>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/gateway.h>
#include <prometheus/counter.h>

PrometheusReporter::PrometheusReporter(std::string host, std::string port,
        std::string scenario, std::string grpc_version, std::string gcs_client_version) :
    registry_(std::make_shared<prometheus::Registry>()),
    gateway_("127.0.0.1", "9091", "sample_client", {}),
    labels_({ {"operation", scenario}, {"grpc_version", grpc_version}, {"gcs_client_version", gcs_client_version}}),
    success_p50_gauge_family_(prometheus::BuildGauge().Name("success_p50").Register(*registry_)),
    success_p50_gauge_(success_p50_gauge_family_.Add(labels_)),
    success_count_family_(prometheus::BuildCounter().Name("success_count").Register(*registry_)),
    success_count_(success_count_family_.Add(labels_)),
    request_count_family_(prometheus::BuildCounter().Name("success_count").Register(*registry_)),
    request_count_(request_count_family_.Add(labels_))
{
    gateway_.RegisterCollectable(registry_);
}

void PrometheusReporter::RecordSuccessP50(double latency) {
    success_p50_gauge_.Set(latency);
}


void PrometheusReporter::ReportSuccessfulCalls(int success_count) {
    success_count_.Increment(success_count);
}

void PrometheusReporter::ReportCalls(int call_count) {
    request_count_.Increment(call_count);
}

bool PrometheusReporter::Push()
{
    int code = gateway_.Push();
    if (code != 200) {
        std::cerr << "Prometheus push failed with status " << code << std::endl;
        return false;
    } else {
        std::cerr << "Pushed metrics.";
    }
    return true;
}
