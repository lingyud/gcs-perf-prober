#ifndef __TEST_RUNNER_HPP__
#define __TEST_RUNNER_HPP__

#include "perftest_config.hpp"
#include "gcs_client.hpp"
#include "prometheus_reporter.hpp"

#include <memory>

class TestRunner
{
public:
    TestRunner(PerftestConfig config);

    void Run(PrometheusReporter *reporter);

private:
    PerftestConfig config_;
    std::unique_ptr<GcsClient> CreateClient();
};

#endif // __TEST_RUNNER_HPP__
