#include "metrics_tracker.hpp"

#include <stmpct/ckms_tq.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

MetricsTracker::MetricsTracker() : success_latency_tracker_(quantiles_.begin(), quantiles_.end())
{
}

void MetricsTracker::ReportSuccess(double latency)
{
    success_latency_tracker_.insert(latency);
    success_count_ += 1;
    request_count_ += 1;
}

void MetricsTracker::ReportError(double latency)
{
    request_count_ += 1;
}

double MetricsTracker::Quantile(double p)
{
    for (stmpct::targeted_quantile q : quantiles_)
    {
        if (q.phi == p)
        {
            return success_latency_tracker_.quantile(p);
            ;
        }
    }

    std::cerr << "Request for quantile " << p << ", which is not tracked.";
    exit(1);
    return 0;
}