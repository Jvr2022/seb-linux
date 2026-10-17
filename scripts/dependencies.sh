#!/usr/bin/env bash
set -euo pipefail

if command -v apt-get >/dev/null; then
    echo "Detected Debian/Ubuntu-based system, installing dependencies..."
    sudo apt-get update
    sudo apt-get install -y \
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
        libjpeg62
elif command -v pacman >/dev/null; then
    echo "Detected Arch Linux-based system, installing dependencies..."
    sudo pacman -Syu --needed --noconfirm \
        gcc \
        make \
        pkgconf \
        qt6-base \
        qt6-webengine \
        webkit2gtk-4.1 \
        gtk3 \
        zlib \
        hicolor-icon-theme \
        shared-mime-info \
        desktop-file-utils \
        polkit \
        qt6-tools
else
    echo "Detected unsupported package manager. Please manually install the Qt6 and WebEngine dependencies."
    exit 1
fi
