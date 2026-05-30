FROM mcr.microsoft.com/devcontainers/base:ubuntu

ARG USERNAME=vscode

ENV DEBIAN_FRONTEND=noninteractive
ENV PIPX_BIN_DIR=/usr/local/bin
ENV PIPX_HOME=/opt/pipx

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        pipx \
        python3 \
        python3-pip \
        python3-venv \
        udev \
    && pipx install platformio \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN usermod -aG dialout "${USERNAME}"

ENV PLATFORMIO_CORE_DIR=/home/${USERNAME}/.platformio

RUN mkdir -p /home/${USERNAME}/.platformio \
    && chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/.platformio
