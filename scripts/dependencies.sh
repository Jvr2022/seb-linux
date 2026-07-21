#!/bin/bash
set -euo pipefail

# Release AppImage builds are expected on Debian/Ubuntu (preferably Ubuntu 22.04).
# Non-Debian systems skip install; use a Debian/Ubuntu container for packaging.

if ! command -v apt-get >/dev/null; then
    echo "detected non-debian system, skipping dependency install"
    exit 0
fi

run_as_root() {
    if [[ "${EUID}" -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

echo "installing dependencies"

run_as_root apt-get update

PACKAGES=(
    binutils
    build-essential
    desktop-file-utils
    dpkg-dev
    libqt6svg6-dev
    qt6-base-dev
    qt6-tools-dev-tools
    qt6-webengine-dev
    libwebkit2gtk-4.1-dev
    libgtk-3-dev
    shared-mime-info
    zlib1g-dev
    libssl-dev
    file
    libatomic1
    libdeflate0
    libjbig0
    libngtcp2-dev
    libngtcp2-crypto-gnutls-dev
    libqt6pdf6
    libqt6qmlworkerscript6
    libnss3
    libssh2-1
    libssl3
    libxcb-cursor0
    libxcb-xinput0
    libjpeg62
    wget
)

# Jammy: libtiff5 / liblerc3; Noble+: libtiff6 / liblerc4.
if apt-cache show libtiff6 >/dev/null 2>&1; then
    PACKAGES+=(libtiff6)
elif apt-cache show libtiff5 >/dev/null 2>&1; then
    PACKAGES+=(libtiff5)
fi
PACKAGES+=(libtiff-dev)

if apt-cache show liblerc4 >/dev/null 2>&1; then
    PACKAGES+=(liblerc4)
elif apt-cache show liblerc3 >/dev/null 2>&1; then
    PACKAGES+=(liblerc3)
else
    echo "optional package not available, skipping: liblerc*"
fi

run_as_root apt-get install -y "${PACKAGES[@]}"
