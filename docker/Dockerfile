# This file is part of qt-flutter-embedder.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com> Author: Sérgio Martins <sergio.martins@kdab.com>
# SPDX-License-Identifier: GPL-3.0-only Contact KDAB at <info@kdab.com> for
# commercial licensing options.

# Docker container for building the engine and testing qt-flutter-embedder
# in CI

FROM ubuntu:24.10
MAINTAINER Sergio Martins (sergio.martins@kdab.com)

ENV TZ=Europe/Berlin
ENV LC_CTYPE=C.UTF-8
# ENV LD_LIBRARY_PATH=/Qt5/lib/:/Qt6/lib/
ENV PATH=$PATH:/home/ubuntu/depot_tools/:/home/ubuntu/flutter/bin/
ENV DISPLAY=:0
ENV FLUTTER_ENGINE_FOLDER=/home/ubuntu/engine/src/flutter

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -qq && apt install build-essential software-properties-common lld git cmake ninja-build vim curl wget gdebi-core libdbus-glib-1-2 \
        mesa-common-dev libglu1-mesa-dev libglib2.0-dev libfontconfig \
        libxkbcommon-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev \
        libxkbcommon-x11-0 libssl-dev openssl unzip clang libgtk-3-dev \
        qt6-base-dev time qt6-base-private-dev \
        -y

USER ubuntu

WORKDIR /home/ubuntu

RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN git clone https://github.com/flutter/flutter.git --branch 3.24.0

# precache linux stuff (flutter precache downloads too much stuffs)
RUN cd /tmp && flutter create foo && cd foo && flutter build linux
RUN rm -rf /tmp/foo/

RUN mkdir /home/ubuntu/engine/ && cd /home/ubuntu/engine/ && fetch flutter && \
cd /home/ubuntu/engine/src/flutter && \
git remote add sergio https://github.com/iamsergio/engine.git && \
git fetch sergio && \
git checkout sergio/main && gclient sync -D

RUN cd $FLUTTER_ENGINE_FOLDER && ./build.sh && cp ./shell/platform/embedder/embedder.h ./shell/platform/embedder/flutter_embedder.h
