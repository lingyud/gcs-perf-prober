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

    // GcsClient(const GcsClient &) = delete;
    // GcsClient &operator=(const GcsClient &) = delete;

    bool ReadObject(std::string object);
    bool ResumablyWriteObject(std::string object, unsigned long bytes);
    bool OneShotWriteObject(std::string object, unsigned long bytes);
    void StartResumableWrite(std::string object);

    static std::string GRPCVersion();
    static std::string GCSClientVersion();

private:
    GcsClient(google::cloud::storage::Client client, std::string bucket);

    google::cloud::storage::Client client_;
    const std::string bucket_;
    unsigned long random_write_buffer_len_;
    char *random_write_buffer_;

    std::vector<char> io_buffer_;
};

#endif // __GCS_CLIENT_HPP_