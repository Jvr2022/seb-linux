#!/bin/bash

if command -v apt >/dev/null; then
    echo "installing dependencies"
    sudo apt-get update
    sudo apt-get install -m -y \
    build-essential \
    desktop-file-utils \
    dpkg-dev \
    libqt6svg6-dev \
    qt6-base-dev \
    qt6-tools-dev-tools \
    qt6-webengine-dev \
    libwebkit2gtk-4.1-dev \
    libgtk-3-dev \
    shared-mime-info \
    zlib1g-dev \
    libssl-dev \
    file \
    libatomic1 \
    libdeflate0 \
    libjbig0 \
    liblerc4 \
    libngtcp2-dev \
    libngtcp2-crypto-gnutls-dev \
    libqt6pdf6 \
    libqt6qmlworkerscript6 \
    libnss3 \
    libssh2-1 \
    libssl3 \
    libtiff-dev \
    libxcb-cursor0 \
    libxcb-xinput0 \
    libnghttp3-9
else
    echo "detected non-debian system, skipping dependency install"
fi
