# syntax=docker/dockerfile:1

# ubuntu dockerfile for server op
# basically client and server are the same, but in reality server has 
# grpc and protoc support installed after the docker build op

# default ubuntu version 22.04, overriden in compose file (see that)
ARG UBUNTU_VERSION=22.04

FROM ubuntu:${UBUNTU_VERSION}

# Display the selected Ubuntu version during build
# (needs to redeclared to be visible)
ARG UBUNTU_VERSION
RUN echo "Selected Ubuntu version: ${UBUNTU_VERSION}"
LABEL Description="Build environment server"

ENV HOME=/root

# define the working directory
WORKDIR /usr/src/

# Copy the Readme file into the container
COPY ./readme-ubuntu_server.txt ./

# install c/c++ dev support
RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    gdb \
    wget \
    nano \
    tree \
    valgrind \
    git \
    autoconf \
    automake \
    libtool \
    curl \
    unzip \
    libssl-dev \
    pkg-config \
    ca-certificates && \
    libcjson-dev \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Install cmake from binaries -- replaced with repo version (included in 24.04)
# RUN apt-get update && \
#     mkdir cmake-install && \
#     cd cmake-install && \
#     wget https://github.com/Kitware/CMake/releases/download/v3.23.3/cmake-3.23.3.tar.gz && \
#     tar -xf cmake-3.23.3.tar.gz && \
#     cd cmake-3.23.3 && \
#     ./bootstrap && \
#     make && \
#     make install 
#     # If source files needs to be removed, then
#     # move following 2 lines to the end of the 
#     # 'make install' line: and add '&& \'
#     # cd .. && \
#     # rm -fr cmake-install
RUN apt-get update && apt-get install -y cmake

RUN echo "Build complete for server Ubuntu ${UBUNTU_VERSION}" 

# # install protoc and grpc (space problem - commented)
# # NOTE! disp spce should not ran out during installation
# # ...and volumes space is not yet available
# # ...but space can be increased during the image build (see docs)
#
# RUN apt-get update && \
#     export MY_INSTALL_DIR=$HOME/.local && \
#     mkdir -p $MY_INSTALL_DIR && \
#     export PATH="$MY_INSTALL_DIR/bin:$PATH" && \
#     pwd && \
#     ls -la && \
#     ls ./c-projects/temp && \
#     cd ./c-projects/temp && \
#     mkdir grpc-install && \    
#     cd grpc-install && \
#     git clone --recurse-submodules -b v1.46.3 --depth 1 --shallow-submodules https://github.com/grpc/grpc && \
#     cd grpc && \
#     mkdir -p cmake/build && \
#     cd cmake/build && \
#     cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../.. && \
#     make -j && \
#     make install

# Some old stuff
# EXPOSE 8080
# CMD [ "/docker-gs-ping" ]

