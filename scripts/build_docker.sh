#!/bin/bash
set -e
export DEBIAN_FRONTEND=noninteractive
export TZ=Etc/UTC

# Enable APT retries and timeouts for reliable package downloads
echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries
echo 'Acquire::http::Timeout "30";' >> /etc/apt/apt.conf.d/80-retries

# Use Azure mirror for amd64 packages (fast & stable in GitHub Actions)
echo "deb [arch=amd64] http://azure.archive.ubuntu.com/ubuntu/ focal main restricted universe multiverse" > /etc/apt/sources.list
echo "deb [arch=amd64] http://azure.archive.ubuntu.com/ubuntu/ focal-updates main restricted universe multiverse" >> /etc/apt/sources.list
echo "deb [arch=amd64] http://azure.archive.ubuntu.com/ubuntu/ focal-security main restricted universe multiverse" >> /etc/apt/sources.list

# Official ports for ARM64 cross-compilation target
echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ focal main restricted universe multiverse" >> /etc/apt/sources.list
echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ focal-updates main restricted universe multiverse" >> /etc/apt/sources.list
echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ focal-security main restricted universe multiverse" >> /etc/apt/sources.list

dpkg --add-architecture arm64
apt-get update

apt-get install -y --no-install-recommends --fix-missing \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  make \
  pkg-config \
  dpkg-dev \
  cmake \
  libsdl2-dev:arm64 \
  libgles2-mesa-dev:arm64 \
  libegl1-mesa-dev:arm64 || \
(apt-get update && apt-get install -y --no-install-recommends --fix-missing \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  make \
  pkg-config \
  dpkg-dev \
  cmake \
  libsdl2-dev:arm64 \
  libgles2-mesa-dev:arm64 \
  libegl1-mesa-dev:arm64)

mkdir -p build && cd build
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig

cmake .. \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DPKG_CONFIG_EXECUTABLE=/usr/bin/aarch64-linux-gnu-pkg-config

cmake --build . --config Release -j$(nproc)

aarch64-linux-gnu-strip alienport
chmod -R 777 /work/build
