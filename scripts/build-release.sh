#!/usr/bin/env bash
set -euo pipefail

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
    libnghttp3-3 \
    libngtcp2-dev \
    libngtcp2-crypto-gnutls-dev \
    libqt6pdf6 \
    libqt6qmlworkerscript6 \
    libnss3 \
    libssh2-1 \
    libssl3 \
    libtiff-dev \
    libxcb-cursor0 \
    libxcb-xinput0
else
    echo "detected non-debian system, skipping dependency install"
fi


ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="${ROOT_DIR}"
VERSION="${1:-0.1.0}"
BUILD_DIR="${PROJECT_DIR}/build"
STAGE_DIR="${PROJECT_DIR}/dist/stage"
ARTIFACT_DIR="${PROJECT_DIR}/dist"
PACKAGE_NAME="safe-exam-browser"
echo "Variables set"

unset QML_SOURCES_PATHS
unset QMAKE
unset EXTRA_QT_MODULES
export QML_SOURCES_PATHS="$PROJECT_DIR"/src
export QMAKE=$(which qmake6)
export EXTRA_QT_MODULES="libssl;libcrypto"
echo "Environment variables set"

mkdir -p "${BUILD_DIR}" "${STAGE_DIR}" "${ARTIFACT_DIR}" "${PROJECT_DIR}/build-tools"
rm -rf "${BUILD_DIR:?}/"* "${STAGE_DIR:?}/"*
cd "${PROJECT_DIR}"
echo "Made directories"

echo "Building for QtWebEngine"
qmake6 CONFIG+=force_qtwebengine INSTALL_ROOT=${STAGE_DIR}/AppDir
make -j$(nproc) >> /dev/null
make install INSTALL_ROOT=${STAGE_DIR}/AppDir
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O ${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage -q
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O ${PROJECT_DIR}/build-tools/linuxdeploy-plugin-qt-x86_64.AppImage -q
chmod +x ${PROJECT_DIR}/build-tools/*.AppImage
echo "QT: Running Linuxdeploy"
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt --verbosity=3 -l libssl.so.3
echo "QT: moving binary"
mv ${PROJECT_DIR}/*.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-qt-x86_64.AppImage

echo "QT: cleanup"
rm -rf ${STAGE_DIR}/AppDir
rm -rf ${PROJECT_DIR}/bin/safe-exam-browser
rm -rf ${PROJECT_DIR}/.moc
rm -rf ${PROJECT_DIR}/.obj
rm -rf ${PROJECT_DIR}/.rcc
rm -f ${PROJECT_DIR}/.qmake.stash
rm -f ${PROJECT_DIR}/Makefile

echo "Building for WebKitGTK"
qmake6 CONFIG+=force_webkitgtk INSTALL_ROOT=${STAGE_DIR}/AppDir
make -j$(nproc) >> /dev/null
make install INSTALL_ROOT=${STAGE_DIR}/AppDir >> /dev/null
echo "GTK: running Linuxdeploy"
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt --verbosity=3 -l libssl.so.3
echo "GTK: moving binary"
mv ${PROJECT_DIR}/*.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-gtk-x86_64.AppImage

echo "Artifacts written to ${ARTIFACT_DIR}"
