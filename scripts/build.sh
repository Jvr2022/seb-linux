#!/usr/bin/env bash
set -euo pipefail

if command -v apt >/dev/null; then
    echo "Installing dependencies"
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
    libicu78 \
    libjbig0 \
    libjpeg62-turbo \
    liblerc4 \
    libnghttp3-3 \
    libngtcp2-dev \
    libngtcp2-crypto-gnutls-dev \
    libngtcp2-crypto-gnutls8 \
    libngtcp2-16 \
    libqt6pdf6 \
    libqt6qmlmeta6 \
    libqt6qmlworkerscript6 \
    libnss3 \
    libssh2-1 \
    libssl3 \
    libtiff-dev \
    libxcb-cursor0 \
    libxcb-xinput0 \
    libxml2-16 \
else
    echo "Detected non-debian system, skipping dependency install"
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BUILD_CONFIG_STAMP="${BUILD_DIR}/.last-qmake-args"
QMAKE_ARGS=("$@")
QMAKE_ARGS_FINGERPRINT="$(printf '%s\0' "${QMAKE_ARGS[@]}" | sha256sum | awk '{print $1}')"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

PREVIOUS_QMAKE_ARGS_FINGERPRINT=""
if [[ -f "${BUILD_CONFIG_STAMP}" ]]; then
  PREVIOUS_QMAKE_ARGS_FINGERPRINT="$(<"${BUILD_CONFIG_STAMP}")"
fi

if [[ -f Makefile && "${PREVIOUS_QMAKE_ARGS_FINGERPRINT}" != "${QMAKE_ARGS_FINGERPRINT}" ]]; then
  echo "Detected changed qmake configuration; cleaning ${BUILD_DIR}"
  make distclean >/dev/null 2>&1 || true
fi

echo "Configuring build with: qmake6 ../seb-linux-qt.pro ${QMAKE_ARGS[*]}"
qmake6 ../seb-linux-qt.pro "${QMAKE_ARGS[@]}"
printf '%s\n' "${QMAKE_ARGS_FINGERPRINT}" > "${BUILD_CONFIG_STAMP}"
make -j"$(nproc)"
popd >/dev/null

echo "Build output: ${BUILD_DIR}/bin/safe-exam-browser"
