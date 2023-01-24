#ifndef __GCS_CLIENT_HPP_
#define __GCS_CLIENT_HPP_

#include <string>
#include <memory>

#include "google/cloud/storage/client.h"

#include "universes.hpp"

class GcsClient
{
public:
    static std::unique_ptr<GcsClient> MakeDirectpathClient(Universe universe, std::string bucket);
    static std::unique_ptr<GcsClient> MakeJSONClient(Universe universe, std::string bucket);

    //GcsClient(const GcsClient &) = delete;
    //GcsClient &operator=(const GcsClient &) = delete;

    bool ReadObject(std::string object);
    void WriteObject(std::string object);
    void StartResumableWrite(std::string object);

    std::string GRPCVersion();
    std::string GCSClientVersion();

private:
    GcsClient(google::cloud::storage::Client client, std::string bucket) :
        client_(client), bucket_(bucket), io_buffer_(262144) {};

    google::cloud::storage::Client client_;
    const std::string bucket_;

    std::vector<char> io_buffer_;
};

#endif // __GCS_CLIENT_HPP_