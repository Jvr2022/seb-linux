#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="${ROOT_DIR}"
VERSION="${1:-0.1.0}"
BUILD_DIR="${PROJECT_DIR}/build"
STAGE_DIR="${PROJECT_DIR}/dist/stage"
ARTIFACT_DIR="${PROJECT_DIR}/dist"
PACKAGE_NAME="safe-exam-browser"

mkdir -p "${BUILD_DIR}" "${STAGE_DIR}" "${ARTIFACT_DIR}" "${PROJECT_DIR}/build-tools"
rm -rf "${BUILD_DIR:?}/"* "${STAGE_DIR:?}/"*

qmake6 CONFIG+=force_qtwebengine INSTALL_ROOT=${STAGE_DIR}/AppDir
make -j$(nproc)
make install INSTALL_ROOT=${STAGE_DIR}/AppDir
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O ${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O ${PROJECT_DIR}/build-tools/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x ${PROJECT_DIR}/build-tools/*.AppImage
export QMAKE=$(which qmake6)
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt
mv ${PROJECT_DIR}/Safe_Exam_Browser-x86_64.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-qt-x86_64.AppImage

rm -rf ${STAGE_DIR}/AppDir
rm -rf ${PROJECT_DIR}/bin/safe-exam-browser
rm -rf ${PROJECT_DIR}/dist/stage
rm -rf ${PROJECT_DIR}/.moc
rm -rf ${PROJECT_DIR}/.obj
rm -rf ${PROJECT_DIR}/.rcc
rm ${PROJECT_DIR}/.qmake.stash
rm ${PROJECT_DIR}/Makefile

qmake6 CONFIG+=force_webkitgtk INSTALL_ROOT=${STAGE_DIR}/AppDir
make -j$(nproc)
make install INSTALL_ROOT=${STAGE_DIR}/AppDir
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${STAGE_DIR}/AppDir -i ${PROJECT_DIR}/assets/icons/safe-exam-browser.png -d ${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt
mv ${PROJECT_DIR}/Safe_Exam_Browser-x86_64.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-gtk-x86_64.AppImage

echo "Artifacts written to ${ARTIFACT_DIR}"
