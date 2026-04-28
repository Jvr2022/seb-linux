#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="${ROOT_DIR}"
VERSION="${1:-0.1.0}"
BUILD_DIR="${PROJECT_DIR}/build"
STAGE_DIR="${PROJECT_DIR}/dist/stage"
ARTIFACT_DIR="${PROJECT_DIR}/dist"
PACKAGE_NAME="safe-exam-browser"

mkdir -p "${BUILD_DIR}" "${STAGE_DIR}" "${ARTIFACT_DIR}"
rm -rf "${BUILD_DIR:?}/"* "${STAGE_DIR:?}/"*

# TODO: Consistency
# TODO: Move appdir to build-stage, edit all paths

# Build AppImages:
qmake6 CONFIG+=force_qtwebengine INSTALL_ROOT=${PROJECT_DIR}/AppDir
make -j$(nproc)
make install INSTALL_ROOT=${PROJECT_DIR}/AppDir
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O ${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O ${PROJECT_DIR}/build-tool/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x ${PROJECT_DIR}/build-tools/*.AppImage
export QMAKE=$(which qmake6)
${PROJECT_DIR}/build-tools/linuxdeploy-x86_64.AppImage --appdir ${PROJECT_DIR}/AppDir -e "$PROJECT_DIR"/assets/icons/safe-exam-browser.png -d "$PROJECT_DIR"/packaging/linux/safe-exam-browser.desktop --output appimage --plugin qt

# TODO: Figure out dynamic name of the built package --> LDAI_OUTPUT=filename
# Move built package
mv ${PROJECT_DIR}/Safe_Exam_Browser-x86_64.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-qt-x86_64.AppImage

# Cleanup
rm -rf ${PROJECT_DIR}/AppDir
rm -rf ${PROJECT_DIR}/bin/safe-exam-browser
rm ${PROJECT_DIR}/Makefile

# qmake6 CONFIG+=force_webkitgtk INSTALL_ROOT=AppDir
# LD_DEBUG=all make -j$(nproc)
# LD_DEBUG=all make install INSTALL_ROOT=AppDir
# ./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage --plugin qt

# # Move built package
# mv ./Safe_Exam_Browser-x86_64.AppImage ${ARTIFACT_DIR}/${PACKAGE_NAME}-gtk-x86_64.AppImage

# # Cleanup
# rm -rf ./AppDir
# rm -rf ./bin/safe-exam-browser
# rm ./Makefile

# Build for debian (outdated)
# mkdir -p "${ARTIFACT_DIR}/debian/DEBIAN"
# DP_ARCH=$(dpkg --print-architecture 2>/dev/null || echo "amd64")
# cat > "${ARTIFACT_DIR}/debian/DEBIAN/control" <<EOF
# Package: ${PACKAGE_NAME}
# Version: ${VERSION}
# Section: education
# Priority: optional
# Architecture: ${DP_ARCH}
# Maintainer: SEB Linux contributors
# Depends: libqt6core6, libqt6gui6, libqt6network6, libqt6webenginecore6 | libwebkit2gtk-4.1-0, libqt6webenginewidgets6 | libwebkit2gtk-4.1-0, libgtk-3-0, shared-mime-info, pkexec
# Description: Safe Exam Browser Linux Qt port
#  Qt-based Safe Exam Browser launcher for Linux with .seb file and sebs:// support.
# EOF

# cp -a "${STAGE_DIR}/usr" "${ARTIFACT_DIR}/debian/"
# if command -v dpkg-deb >/dev/null 2>&1; then
#   dpkg-deb --build "${ARTIFACT_DIR}/debian" "${ARTIFACT_DIR}/${PACKAGE_NAME}_${VERSION}_amd64.deb"
# else
#   echo "warning: dpkg-deb not found; skipping .deb package build" >&2
# fi

# TODO: Change this accordingly

tar \
  --exclude='./dist' \
  --exclude='./build' \
  --exclude='./Makefile' \
  --exclude='./.qmake.stash' \
  --exclude='./safe-exam-browser' \
  --exclude='./*.o' \
  --transform "s|^./|${PACKAGE_NAME}-${VERSION}/|" \
  -czf "${ARTIFACT_DIR}/${PACKAGE_NAME}-${VERSION}.tar.gz" \
  -C "${PROJECT_DIR}" .

echo "Artifacts written to ${ARTIFACT_DIR}"
