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
        nlohmann-json3-dev \
        git \
        libboost-all-dev && \
rm -rf /var/lib/apt/lists/*

# Install Catch2.
RUN cd /tmp && \
    git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    git checkout v2.13.3 && \
    cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install && \
    cd .. && \
    rm /tmp/Catch2 -rf

