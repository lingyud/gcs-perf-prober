#include "gcs_client.hpp"

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/grpc_plugin.h"
#include <grpc++/grpc++.h>

#include <algorithm>

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

GcsClient::GcsClient(google::cloud::storage::Client client, std::string bucket) : client_(client), bucket_(bucket), io_buffer_(104857600)
{
    random_write_buffer_len_ = 104857600;
    // Probably faster/better to read from /dev/urandom?
    random_write_buffer_ = new char[random_write_buffer_len_];
    for (unsigned long i = 0; i < random_write_buffer_len_; i++)
    {
        random_write_buffer_[i] = (char)rand();
    }
};

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
    if (stream.bad())
    {
        std::cerr << "Error closing read object: " << stream.status() << "\n";
        return false;
    }

    return true;
}

bool GcsClient::ResumablyWriteObject(std::string object, unsigned long bytes)
{
    gcs::ObjectWriteStream stream = client_.WriteObject(bucket_, object, gcs::UseResumableUploadSession(gcs::NewResumableUploadSession()));
    if (stream.bad())
    {
        std::cerr << "Error starting resumable uploads: " << stream.metadata().status() << "\n";
        return false;
    }
    //std::cerr << "ResumablyWriteObject write buffer len: " << random_write_buffer_len_;
    unsigned long written = 0;
    while (written < bytes)
    {
        unsigned long to_write = std::min(random_write_buffer_len_, bytes - written);
        stream.write(random_write_buffer_, to_write);
        written += to_write;

        if (stream.bad())
        {
            std::cerr << "Error writing to the stream: " << stream.metadata().status() << "\n";
            return false;
        }
    }
    stream.Close();
    if (stream.bad())
    {
        std::cerr << "Error Finishing resumable uploads: " << stream.metadata().status() << "\n";
        return false;
    }
    return true;
}

bool GcsClient::OneShotWriteObject(std::string object, unsigned long bytes) 
{ 
    std::string str;
    str.assign(random_write_buffer_, random_write_buffer_ + bytes);
    //std::cerr << "OneShotWriteObject write buffer len: " << str.length();
    gc::StatusOr<gcs::ObjectMetadata> insertResult = client_.InsertObject(bucket_, object, str);
    if (!insertResult.ok())
    {
        std::cerr << "Error doing a non-resumable uploads: " << insertResult.status() << "\n";
        return false;
    }
    return true;
}
void GcsClient::StartResumableWrite(std::string object) {}

std::string GcsClient::GRPCVersion()
{
    return grpc::Version();
}

std::string GcsClient::GCSClientVersion()
{
    return gc::version_string();
}
