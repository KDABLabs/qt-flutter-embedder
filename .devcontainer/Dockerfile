#
# This file is part of qt-flutter-embedder.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sérgio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only

# Instructions:
# docker build -t qt-flutter-embedder .
# docker run -it -v /data/sources/flutter/qt-flutter-embedder/:/qt-flutter-embedder/ -e XDG_RUNTIME_DIR=/tmp -e DISPLAY=$DISPLAY --device=/dev/dri -v /tmp/.X11-unix:/tmp/.X11-unix:rw -u $(id -u):$(id -g) qt-flutter-embedder
# cd /qt-flutter-embedder/ && cmake --preset=dev
# cd build-dev && ninja

# To publish in docker hub:
# docker build -t qt-flutter-embedder .
# docker run -it qt-flutter-embedder # and exit
# docker ps # to check sha
# docker export -o mycontainer.tar <container_sha>
# docker import mycontainer.tar
# docker images # to check sha
# docker tag <image_sha> iamsergio/ci-qt-flutter-embedder
# docker push iamsergio/ci-qt-flutter-embedder

FROM ubuntu:24.04

ENV TZ=Europe/Berlin
ENV LC_CTYPE=C.UTF-8
ENV PATH=$PATH:/qt-flutter-embedder/3rdparty/flutter/bin/:/qt-flutter-embedder/3rdparty/flutter/bin/cache/dart-sdk/bin/

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -qq && apt install -y vim build-essential software-properties-common lld git cmake ninja-build python3 \
    python3-dev clang llvm libclang-15-dev libxslt-dev mesa-common-dev libglu1-mesa-dev libglib2.0-dev libfontconfig \
    libxkbcommon-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev nlohmann-json3-dev clang-format-15 wget curl \
    file unzip zip which xz-utils libgtk-3-dev libxcb-icccm4 libxcb-cursor-dev libx11-xcb1 libxcb-shape0-dev libxcb-keysyms1

RUN ln -sf /usr/lib/x86_64-linux-gnu/libasan.so.8 /usr/lib/libasan.so
RUN ln -sf /usr/bin/qmake6 /usr/bin/qmake

RUN apt install sudo python3-pip python3.12-venv -y

RUN mkdir -p -m 755 /etc/apt/keyrings \
    && curl -s https://cli.github.com/packages/githubcli-archive-keyring.gpg | tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null \
    && chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
    && apt-get update \
    && apt-get install gh -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN echo 'ALL ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

WORKDIR /home/ubuntu/
USER ubuntu
RUN git config --global advice.detachedHead false

RUN pip install aqtinstall --break-system-packages && \
    ~/.local/bin/aqt install-qt linux desktop 6.6.0 gcc_64 -O /home/ubuntu/Qt/

WORKDIR /qt-flutter-embedder

ENV PATH=$PATH:/home/ubuntu/Qt/6.6.0/gcc_64/bin/
