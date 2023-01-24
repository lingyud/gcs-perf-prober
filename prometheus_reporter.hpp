#ifndef __PROMETHEUS_REPORTER_HPP_
#define __PROMETHEUS_REPORTER_HPP_

#include <string>
#include <memory>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/gateway.h>
#include <prometheus/counter.h>

class PrometheusReporter
{
public:
    PrometheusReporter(std::string host, std::string port,
        std::string scenario, std::string grpc_version, std::string gcs_client_version);

    PrometheusReporter(const PrometheusReporter &) = delete;
    PrometheusReporter &operator=(const PrometheusReporter &) = delete;

    void RecordSuccessP50(double latency);

    void ReportSuccessfulCalls(int success_count);
    void ReportCalls(int call_count);

    bool Push();

private:
    //std::shared_ptr<prometheus::Registry> registry_;
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Gateway gateway_;
    prometheus::Labels labels_;

    prometheus::Family<prometheus::Gauge> &success_p50_gauge_family_;
    prometheus::Gauge &success_p50_gauge_;

    prometheus::Family<prometheus::Counter> &success_count_family_;
    prometheus::Counter &success_count_;

    prometheus::Family<prometheus::Counter> &request_count_family_;
    prometheus::Counter &request_count_;
};

#endif // __PROMETHEUS_REPORTER_HPP_