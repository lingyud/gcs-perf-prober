#ifndef metrics_tracker_hpp__
#define metrics_tracker_hpp__

#include <stmpct/ckms_tq.hpp>

class MetricsTracker {
 public:
  MetricsTracker();

  MetricsTracker(const MetricsTracker&) = delete;
  MetricsTracker& operator=(const MetricsTracker&)
      = delete;

  void ReportSuccess(double latency);
  void ReportError(double latency);
  
  double Quantile(double p);
  
 private:
  double request_count_ = 0;
  double success_count_ = 0;
  std::vector<stmpct::targeted_quantile> quantiles_ = {{0.5, 0.0001}, {.9, .0001}, {.999, .0001}};
  stmpct::ckms_tq<double> success_latency_tracker_;
};

#endif  // metrics_tracker_hpp__