FROM ubuntu:22.10

# Curl necessary for ca-certificates
RUN apt-get -qq update
RUN apt-get install -y -q curl

WORKDIR /usr/perf-prober
COPY /target/gcs_perf_prober .
COPY run_scripts/docker_launch.sh .


CMD "/usr/perf-prober/docker_launch.sh"