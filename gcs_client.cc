#include "gcs_client.hpp"

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/grpc_plugin.h"
#include <grpc++/grpc++.h>


namespace gc = ::google::cloud;
namespace gcs = gc::storage;
namespace gcs_experimental = gc::storage_experimental;

std::map<Universe, std::string> directpath_endpoint_map = {
    {PROD, "google-c2p:///storage.googleapis.com"},
    {PREPROD, "google-c2p-experimental:///storage-preprod-test-grpc.googleusercontent.com"},
    {HP_PREPROD, "google-c2p:///gcshp-central1-storage-preprod-test-grpc.googleusercontent.com"},
};

std::map<Universe, std::string> json_endpoint_map = {
    {PROD, "https://storage.googleapis.com"},
    {PREPROD, "https://storage-preprod-test-unified.googleusercontent.com"},
    {HP_PREPROD, "https://gcshp-central1-storage-preprod-test-unified.googleusercontent.com"} // path: /storage/v1_preprod
};

std::map<Universe, std::string> json_version_map = {
    {PROD, "v1"},
    {PREPROD, "v1_preprod"},
    {HP_PREPROD, "v1_preprod"}};

std::unique_ptr<GcsClient> GcsClient::MakeDirectpathClient(Universe universe, std::string bucket)
{
    gc::Options options{};
    options.set<gcs_experimental::GrpcPluginOption>("media")
        .set<gc::EndpointOption>(directpath_endpoint_map[universe]);
    gcs::Client client = google::cloud::storage_experimental::DefaultGrpcClient(options);

    return std::unique_ptr<GcsClient>(new GcsClient(client, bucket));
}

std::unique_ptr<GcsClient> GcsClient::MakeJSONClient(Universe universe, std::string bucket)
{
    gc::Options options{};
    options.set<gcs_experimental::GrpcPluginOption>("none")
        .set<google::cloud::storage::RestEndpointOption>(json_endpoint_map[universe])
        .set<google::cloud::storage::internal::TargetApiVersionOption>(json_version_map[universe]);
    gcs::Client client = google::cloud::storage_experimental::DefaultGrpcClient(options);

    return std::unique_ptr<GcsClient>(new GcsClient(client, bucket));
}

bool GcsClient::ReadObject(std::string object)
{
    auto stream = client_.ReadObject(bucket_, object);
    if (stream.bad())
    {
        std::cerr << "Error reading object: " << stream.status() << "\n";
        return false;
    }
    do
    {
        stream.read(io_buffer_.data(), io_buffer_.size());
        if (stream.bad())
        {
            std::cerr << "Error reading object: " << stream.status() << "\n";
            return false;
        }
    } while (stream);
    stream.Close();

    return true;
}

void GcsClient::WriteObject(std::string object) {}
void GcsClient::StartResumableWrite(std::string object) {}

std::string GcsClient::GRPCVersion() {
    return grpc::Version();
}

std::string GcsClient::GCSClientVersion() {
    return gc::version_string();
}
