FROM ubuntu:20.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        software-properties-common \
        clang-format \
        clang \
        ninja-build \
        build-essential \
        gdb \
        vim \
        cmake \
        libboost-all-dev && \
rm -rf /var/lib/apt/lists/*

