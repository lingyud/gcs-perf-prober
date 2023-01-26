#!/bin/bash

echo "Hi, I'm Brandon's docker startup script"

echo "You want me to run $SCENARIO in region $REGION in universe $UNIVERSE, for $OPERATION operation"

if [[ -z "${SCENARIO}" ]]; then
  echo "You must set SCENARIO variable"
  exit
fi

if [[ -z "${REGION}" ]]; then
  echo "You must set REGION variable"
  exit
fi

if [[ -z "${UNIVERSE}" ]]; then
  echo "You must set UNIVERSE variable"
  exit
fi

if [[ -z "${OPERATION}" ]]; then
  echo "You must set OPERATION variable"
  exit
fi

if [[ "preprod" == "$UNIVERSE" ]]; then
	BUCKET_PREFIX="gcs-grpc-team-preprod-perf"
else
	BUCKET_PREFIX="gcs-grpc-team-perf-testing"
fi

export BUCKET="${BUCKET_PREFIX}-${REGION}-${SCENARIO}"
echo "Running on bucket '$BUCKET'"

./gcs_perf_prober --universe=$UNIVERSE --api=json --scenario=$SCENARIO --region=$REGION --operation=$OPERATION --object_name=100K --push_to_prometheus --run_duration=1m &
./gcs_perf_prober --universe=$UNIVERSE --api=grpc --scenario=$SCENARIO --region=$REGION --operation=$OPERATION --object_name=100K --push_to_prometheus --run_duration=1m

# And start JSON as well.