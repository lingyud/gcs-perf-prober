#ifndef __PERF_SCENARIO_HPP_
#define __PERF_SCENARIO_HPP_

#include <string>
#include <optional>

#include "universes.hpp"

enum Operation
{
    READ,
    RESUMABLE_WRITE,
    ONESHOT_WRITE,
    QUERY_WRITE_STATUS,
};

enum ClientAPI
{
    GRPC_DIRECTPATH,
    JSON
};

class PerftestConfig
{
public:
    static std::optional<PerftestConfig> LoadConfig();

    PerftestConfig &operator=(const PerftestConfig &) = delete;

    std::string scenario() { return scenario_; }
    std::string region() { return region_; }
    Universe universe() { return universe_; }
    std::string universe_str();
    ClientAPI clientAPI() { return clientAPI_; }
    Operation operation() { return operation_; }
    std::string bucket() { return bucket_; }
    std::string object() { return object_; }

    long write_length() { return write_length_; }

private:
    PerftestConfig(std::string scenario, std::string region,
                   Universe universe, ClientAPI clientAPI, Operation operation, std::string bucket, std::string object, long write_length) : scenario_(scenario),
                                                                                                                                             region_(region),
                                                                                                                                             universe_(universe),
                                                                                                                                             clientAPI_(clientAPI),
                                                                                                                                             operation_(operation),
                                                                                                                                             bucket_(bucket),
                                                                                                                                             object_(object),
                                                                                                                                             write_length_(write_length){};

    const std::string scenario_;
    const std::string region_;
    const Universe universe_;
    const ClientAPI clientAPI_;
    const Operation operation_;
    const std::string bucket_;
    const std::string object_;
    const double write_length_;
};

#endif