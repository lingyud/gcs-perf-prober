#!/bin/bash

# exit when any command fails
set -e

bazel build :gcs_perf_prober

# bazel-bin is a symlink out of the directory, so Docker doesn't like it.
rm -rf target
mkdir -p target
mv bazel-bin/gcs_perf_prober target/

# Login to Could container stuff
gcloud auth print-access-token | sudo docker login -u oauth2accesstoken --password-stdin https://us-central1-docker.pkg.dev

# Build the image locally
docker build -t us-central1-docker.pkg.dev/gcs-grpc-team-testing/my-repository/gcs-perf-prober:latest .

# Push the image to google cloud
docker push us-central1-docker.pkg.dev/gcs-grpc-team-testing/my-repository/gcs-perf-prober:latest

echo "Success. To start one of these, use kubectl apply. e.g.:  kubectl apply -f deployments/prod-us-east1-100k-object-read.yaml"
echo "To stop it, kubectl get pods to get the ID, then kubectl kill."