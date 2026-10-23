FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        cppcheck \
        curl \
        gcc-arm-none-eabi \
        gdb-multiarch \
        ninja-build \
        python3 \
        python3-pip \
        stlink-tools \
        openocd \
    && rm -rf /var/lib/apt/lists/*

COPY tools/install_renode.sh /tmp/install_renode.sh
RUN sh /tmp/install_renode.sh /opt/renode && rm /tmp/install_renode.sh

ENV PATH="/root/.local/bin:${PATH}"
WORKDIR /workspace
