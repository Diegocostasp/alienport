FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libsdl2-dev \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    zlib1g-dev \
    zip \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -ms /bin/bash vscode && echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER root
WORKDIR /workspace
