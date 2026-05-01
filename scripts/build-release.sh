#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="${ROOT_DIR}"
VERSION="${1:-0.1.0}"
BUILD_DIR="${PROJECT_DIR}/build"
PROJECT_FILE="${PROJECT_DIR}/seb-linux-qt.pro"
ARTIFACT_DIR="${PROJECT_DIR}/dist"
TOOLS_DIR="${PROJECT_DIR}/build-tools"
PACKAGE_NAME="safe-exam-browser"
LINUXDEPLOY_VERSION="1-alpha-20250213-2"
LINUXDEPLOY_PLUGIN_QT_VERSION="1-alpha-20250213-1"
LINUXDEPLOY_SHA256="4648f278ab3ef31f819e67c30d50f462640e5365a77637d7e6f2ad9fd0b4522a"
LINUXDEPLOY_PLUGIN_QT_SHA256="15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724"
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VERSION}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_PLUGIN_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${LINUXDEPLOY_PLUGIN_QT_VERSION}/linuxdeploy-plugin-qt-x86_64.AppImage"
LINUXBUILD_EXTRA_ARGS=(
    -l /usr/lib/x86_64-linux-gnu/libssl.so.3
    -l /usr/lib/x86_64-linux-gnu/libsoftokn3.so
    -l /usr/lib/x86_64-linux-gnu/libatomic.so.1
    -l /usr/lib/x86_64-linux-gnu/libdeflate.so.0
    -l /usr/lib/x86_64-linux-gnu/libjbig.so.0
    -l /usr/lib/x86_64-linux-gnu/libjpeg.so.62
    -l /usr/lib/x86_64-linux-gnu/libLerc.so.4
    -l /usr/lib/x86_64-linux-gnu/libQt6Pdf.so.6
    -l /usr/lib/x86_64-linux-gnu/libQt6QmlWorkerScript.so.6
    -l /usr/lib/x86_64-linux-gnu/libsmime3.so
    -l /usr/lib/x86_64-linux-gnu/libssh2.so.1
    -l /usr/lib/x86_64-linux-gnu/libtiff.so.6
    -l /usr/lib/x86_64-linux-gnu/libxcb-cursor.so.0
    -l /usr/lib/x86_64-linux-gnu/libxcb-xinput.so.0
)
# -l /usr/lib/x86_64-linux-gnu/libnghttp3.so.9
# -l /usr/lib/x86_64-linux-gnu/libngtcp2.so.16 -l /usr/lib/x86_64-linux-gnu/libngtcp2_crypto_gnutls.so.8
# -l /usr/lib/x86_64-linux-gnu/libQt6QmlMeta.so.6
# -l /usr/lib/x86_64-linux-gnu/libxml2.so.16
echo "Variables set"

verify_sha256() {
    local file_path="$1"
    local expected_sha256="$2"
    local actual_sha256

    actual_sha256="$(sha256sum "${file_path}" | awk '{print $1}')"
    [[ "${actual_sha256}" == "${expected_sha256}" ]]
}

download_verified_tool() {
    local url="$1"
    local output_path="$2"
    local expected_sha256="$3"
    local tmp_path

    if [[ -f "${output_path}" ]] && verify_sha256 "${output_path}" "${expected_sha256}"; then
        chmod +x "${output_path}"
        return
    fi

    tmp_path="$(mktemp "${output_path}.XXXXXX")"
    if ! wget -q "${url}" -O "${tmp_path}"; then
        rm -f "${tmp_path}"
        echo "Failed to download ${url}" >&2
        exit 1
    fi

    if ! verify_sha256 "${tmp_path}" "${expected_sha256}"; then
        rm -f "${tmp_path}"
        echo "Checksum verification failed for ${url}" >&2
        exit 1
    fi

    mv "${tmp_path}" "${output_path}"
    chmod +x "${output_path}"
}

run_logged() {
    local log_path="$1"
    shift

    if ! "$@" > "${log_path}" 2>&1; then
        echo "Command failed: $*" >&2
        echo "Last 100 lines from ${log_path}:" >&2
        tail -n 100 "${log_path}" >&2 || true
        return 1
    fi
}

build_appimage() {
    local config="$1"
    local build_dir="$2"
    local output_path="$3"
    local app_dir="${build_dir}/AppDir"
    local appimage_output_dir="${build_dir}/appimage-output"
    local appimage_path

    rm -rf "${app_dir}" "${appimage_output_dir}"
    mkdir -p "${build_dir}" "${app_dir}" "${appimage_output_dir}"

    pushd "${build_dir}" >/dev/null
    qmake6 "${PROJECT_FILE}" "CONFIG+=${config}" "INSTALL_ROOT=${app_dir}"
    run_logged "${build_dir}/make.log" make -j"$(nproc)"
    run_logged "${build_dir}/make-install.log" make install "INSTALL_ROOT=${app_dir}"
    popd >/dev/null

    rm -f "${output_path}"
    pushd "${appimage_output_dir}" >/dev/null
    PATH="${TOOLS_DIR}:${PATH}" \
    LINUXDEPLOY_OUTPUT_VERSION="${VERSION}" \
    "${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" \
        --appdir "${app_dir}" \
        -i "${PROJECT_DIR}/assets/icons/safe-exam-browser.png" \
        -d "${PROJECT_DIR}/packaging/linux/safe-exam-browser.desktop" \
        --output appimage \
        --plugin qt \
        --verbosity=3 \
        "${LINUXBUILD_EXTRA_ARGS[@]}"
    popd >/dev/null

    appimage_path="$(find "${appimage_output_dir}" -maxdepth 1 -type f -name '*.AppImage' -print -quit)"
    if [[ -z "${appimage_path}" ]]; then
        echo "linuxdeploy did not produce an AppImage" >&2
        exit 1
    fi

    mv "${appimage_path}" "${output_path}"
}

echo "Installing dependencies"
${ROOT_DIR}/scripts/dependencies.sh

unset QML_SOURCES_PATHS
unset QMAKE
unset EXTRA_QT_MODULES
export QML_SOURCES_PATHS="$PROJECT_DIR"/src
export QMAKE=$(which qmake6)
export EXTRA_QT_MODULES="libssl;libcrypto"
echo "Environment variables set"

mkdir -p "${BUILD_DIR}" "${ARTIFACT_DIR}" "${TOOLS_DIR}"
rm -rf "${BUILD_DIR:?}/"*
echo "Made directories"

download_verified_tool \
    "${LINUXDEPLOY_URL}" \
    "${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" \
    "${LINUXDEPLOY_SHA256}"
download_verified_tool \
    "${LINUXDEPLOY_PLUGIN_QT_URL}" \
    "${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage" \
    "${LINUXDEPLOY_PLUGIN_QT_SHA256}"

echo "Building for QtWebEngine"
build_appimage \
    "force_qtwebengine" \
    "${BUILD_DIR}/release-qtwebengine" \
    "${ARTIFACT_DIR}/${PACKAGE_NAME}-qt-x86_64.AppImage"

echo "Building for WebKitGTK"
build_appimage \
    "force_webkitgtk" \
    "${BUILD_DIR}/release-webkitgtk" \
    "${ARTIFACT_DIR}/${PACKAGE_NAME}-gtk-x86_64.AppImage"

echo "Artifacts written to ${ARTIFACT_DIR}"
