#include <vector>
#include "google/cloud/storage/client.h"
#include "google/cloud/storage/grpc_plugin.h"

const char BUCKET_NAME[] = "gcs-grpc-team-perf-testing-us-central1-10-byte-object-read";
const char OBJECT_NAME[] = "10_bytes.txt";

namespace gc = ::google::cloud;
namespace gcs = gc::storage;
namespace gcs_experimental = gc::storage_experimental;

int main(int argc, char **argv)
{
    gc::Options options{};
    options.set<gcs_experimental::GrpcPluginOption>("media")
        .set<gc::EndpointOption>("google-c2p:///storage.googleapis.com");
    gcs::Client client = google::cloud::storage_experimental::DefaultGrpcClient(options);

    int BUFFER_SIZE = 100;
    std::vector<char> buffer(BUFFER_SIZE);
    for( long i=1;/*forever*/;i++)
    {
        auto stream = client.ReadObject(BUCKET_NAME, OBJECT_NAME);
        if (stream.bad())
        {
            std::cerr << "Error reading object: " << stream.status() << "\n";
            return 1;
        }
        do
        {
            stream.read(buffer.data(), BUFFER_SIZE);
        } while (stream);

        if (i % 25 == 0)
            std::cout << "Read " << i << " objects" << std::endl;
    }
}
