
FROM debian:11-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    libncurses-dev \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    bc \
    git \
    rsync \
    kmod \
    cpio \
    dwarves \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/linux