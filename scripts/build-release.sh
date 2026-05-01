#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="${ROOT_DIR}"
VERSION="${1:-0.1.0}"
BUILD_DIR="${PROJECT_DIR}/build"
STAGE_DIR="${PROJECT_DIR}/dist/stage"
ARTIFACT_DIR="${PROJECT_DIR}/dist"
PACKAGE_NAME="safe-exam-browser"
LINUXBUILD_EXTRA_ARGS="-l /usr/lib/x86_64-linux-gnu/libssl.so.3 -l /usr/lib/x86_64-linux-gnu/libsoftokn3.so -l /usr/lib/x86_64-linux-gnu/libatomic.so.1 -l /usr/lib/x86_64-linux-gnu/libdeflate.so.0 -l /usr/lib/x86_64-linux-gnu/libjbig.so.0 -l /usr/lib/x86_64-linux-gnu/libjpeg.so.62 -l /usr/lib/x86_64-linux-gnu/libLerc.so.4 -l /usr/lib/x86_64-linux-gnu/libnghttp3.so.9 -l /usr/lib/x86_64-linux-gnu/libngtcp2.so.16 -l /usr/lib/x86_64-linux-gnu/libngtcp2_crypto_gnutls.so.8 -l /usr/lib/x86_64-linux-gnu/libQt6Pdf.so.6 -l /usr/lib/x86_64-linux-gnu/libQt6QmlMeta.so.6 -l /usr/lib/x86_64-linux-gnu/libQt6QmlWorkerScript.so.6 -l /usr/lib/x86_64-linux-gnu/libsmime3.so -l /usr/lib/x86_64-linux-gnu/libssh2.so.1 -l /usr/lib/x86_64-linux-gnu/libtiff.so.6 -l /usr/lib/x86_64-linux-gnu/libxcb-cursor.so.0 -l /usr/lib/x86_64-linux-gnu/libxcb-xinput.so.0 -l /usr/lib/x86_64-linux-gnu/libxml2.so.16"
echo "Variables set"

echo "Installing dependencies"
${ROOT_DIR}/scripts/dependencies.sh

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
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt --verbosity=3 ${LINUXBUILD_EXTRA_ARGS}
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
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt --verbosity=3 ${LINUXBUILD_EXTRA_ARGS}

echo "GTK: moving binary"
mv ${PROJECT_DIR}/*.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-gtk-x86_64.AppImage

echo "Artifacts written to ${ARTIFACT_DIR}"
