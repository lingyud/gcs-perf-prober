#include "perftest_config.hpp"

#include "absl/flags/flag.h"
#include "absl/strings/str_cat.h"

#include <iostream>
#include <optional>

ABSL_FLAG(std::optional<std::string>, universe, std::nullopt, "prod or preprod");
ABSL_FLAG(std::optional<std::string>, scenario, std::nullopt, "Name of the scenario, for example 100k-object-read");
ABSL_FLAG(std::optional<std::string>, region, std::nullopt, "Name of the scenario, for example 100k-object-read");
ABSL_FLAG(std::optional<std::string>, api, std::nullopt, "grpc, grpc-no-directparth, json");

ABSL_FLAG(std::optional<std::string>, operation, std::nullopt, "ReadObject, WriteObject, etc");
ABSL_FLAG(std::optional<std::string>, object_name, std::nullopt, "Name of object to read/write");

std::optional<PerftestConfig> PerftestConfig::LoadConfig()
{
    if (!absl::GetFlag(FLAGS_universe).has_value())
    {
        std::cerr << "You must set the 'universe' flag to 'prod' or 'preprod'" << std::endl;
        return std::nullopt;
    }
    if (!absl::GetFlag(FLAGS_api).has_value())
    {
        std::cerr << "You must set the 'api' flag to 'grpc', 'grpc-no-directpath', or 'json'" << std::endl;
        return std::nullopt;
    }
    if (!absl::GetFlag(FLAGS_scenario).has_value())
    {
        std::cerr << "Must specify 'scenario'" << std::endl;
        return std::nullopt;
    }
    if (!absl::GetFlag(FLAGS_region).has_value())
    {
        std::cerr << "Must specify 'region'" << std::endl;
        return std::nullopt;
    }
    if (!absl::GetFlag(FLAGS_operation).has_value())
    {
        std::cerr << "Must specify 'operation'" << std::endl;
        return std::nullopt;
    }
    if (!absl::GetFlag(FLAGS_object_name).has_value())
    {
        std::cerr << "Must specify 'object_name'" << std::endl;
        return std::nullopt;
    }

    ClientAPI api;
    if (absl::GetFlag(FLAGS_api) == "grpc")
    {
        api = GRPC_DIRECTPATH;
    }
    else if (absl::GetFlag(FLAGS_api) == "json")
    {
        api = JSON;
    }
    else
    {
        std::cerr << "You must set the 'api' flag to 'grpc', 'grpc-no-directpath', or 'json'" << std::endl;
        return std::nullopt;
    }

    std::string bucket_prefix;

    Universe universe;
    if (absl::GetFlag(FLAGS_universe) == "prod")
    {
        universe = PROD;
        bucket_prefix = "gcs-grpc-team-perf-testing";
    }
    else if (absl::GetFlag(FLAGS_universe) == "preprod")
    {
        universe = PREPROD;
        bucket_prefix = "gcs-grpc-team-preprod-perf";
    }
    else
    {
        std::cerr << "You must set the 'universe' flag to 'prod' or 'preprod'" << std::endl;
        return std::nullopt;
    }

    Operation operation;
    if (absl::GetFlag(FLAGS_operation) == "ReadObject")
    {
        operation = READ;
    }
    else
    {
        std::cerr << "You must set the 'operation' flag to a known operation" << std::endl;
        return std::nullopt;
    }

    std::string scenario = *absl::GetFlag(FLAGS_scenario);
    std::string region = *absl::GetFlag(FLAGS_region);
    std::string bucket = absl::StrCat(bucket_prefix, "-", region, "-", scenario);
    std::string object = *absl::GetFlag(FLAGS_object_name);

    return std::make_optional<PerftestConfig>({scenario, region,
                                               universe, api, operation, bucket, object});
}
