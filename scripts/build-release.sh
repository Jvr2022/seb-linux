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
# AppImages inherit the build host glibc. Ubuntu 22.04 sets the floor at 2.35.
MAX_GLIBC_VERSION="${MAX_GLIBC_VERSION:-2.35}"
LINUXDEPLOY_VERSION="1-alpha-20250213-2"
LINUXDEPLOY_PLUGIN_QT_VERSION="1-alpha-20250213-1"
LINUXDEPLOY_SHA256="4648f278ab3ef31f819e67c30d50f462640e5365a77637d7e6f2ad9fd0b4522a"
LINUXDEPLOY_PLUGIN_QT_SHA256="15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724"
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VERSION}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_PLUGIN_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${LINUXDEPLOY_PLUGIN_QT_VERSION}/linuxdeploy-plugin-qt-x86_64.AppImage"

# Prefer extract-and-run so nested AppImages work without FUSE (CI containers).
export APPIMAGE_EXTRACT_AND_RUN="${APPIMAGE_EXTRACT_AND_RUN:-1}"

# Library basenames to bundle via linuxdeploy -l (resolved at runtime).
REQUIRED_EXTRA_LIBS=(
    libssl.so.3
    libsoftokn3.so
    libatomic.so.1
    libdeflate.so.0
    libjbig.so.0
    libjpeg.so.62
    libQt6Pdf.so.6
    libQt6QmlWorkerScript.so.6
    libsmime3.so
    libssh2.so.1
    libxcb-cursor.so.0
    libxcb-xinput.so.0
)

# Prefer newer sonames first. Jammy: libtiff.so.5 / libLerc.so.3; Noble+: .6 / .4.
ALTERNATE_EXTRA_LIBS=(
    "libtiff.so.6|libtiff.so.5"
    "libLerc.so.4|libLerc.so.3"
)

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

resolve_library_path() {
    local lib_name="$1"
    local resolved=""
    local candidate

    if command -v ldconfig >/dev/null; then
        resolved="$(ldconfig -p 2>/dev/null | awk -v name="${lib_name}" '
            $1 == name {
                for (i = 1; i <= NF; i++) {
                    if ($i ~ /^\//) {
                        print $i
                        exit
                    }
                }
            }
        ')"
    fi

    # NSS modules (e.g. libsoftokn3.so) live under .../nss/ and are often absent
    # from ldconfig -p / the top-level multiarch lib dir.
    if [[ -z "${resolved}" || ! -e "${resolved}" ]]; then
        for candidate in \
            "/usr/lib/x86_64-linux-gnu/nss/${lib_name}" \
            "/usr/lib/x86_64-linux-gnu/${lib_name}" \
            "/usr/lib64/nss/${lib_name}" \
            "/usr/lib64/${lib_name}" \
            "/usr/lib/nss/${lib_name}" \
            "/usr/lib/${lib_name}"; do
            if [[ -e "${candidate}" ]]; then
                resolved="${candidate}"
                break
            fi
        done
    fi

    if [[ -n "${resolved}" && -e "${resolved}" ]]; then
        readlink -f "${resolved}" 2>/dev/null || printf '%s\n' "${resolved}"
        return 0
    fi

    return 1
}

build_linuxdeploy_extra_args() {
    local lib_name
    local lib_path
    local alt_group
    local found

    LINUXBUILD_EXTRA_ARGS=()

    for lib_name in "${REQUIRED_EXTRA_LIBS[@]}"; do
        if ! lib_path="$(resolve_library_path "${lib_name}")"; then
            echo "Required library not found: ${lib_name}" >&2
            exit 1
        fi
        LINUXBUILD_EXTRA_ARGS+=(-l "${lib_path}")
    done

    for alt_group in "${ALTERNATE_EXTRA_LIBS[@]}"; do
        found=0
        IFS='|' read -r -a alt_libs <<< "${alt_group}"
        for lib_name in "${alt_libs[@]}"; do
            if lib_path="$(resolve_library_path "${lib_name}")"; then
                LINUXBUILD_EXTRA_ARGS+=(-l "${lib_path}")
                found=1
                break
            fi
        done
        if [[ "${found}" -eq 0 ]]; then
            echo "Warning: none of these libraries found (optional): ${alt_group}" >&2
        fi
    done
}

version_gt() {
    # Return 0 if $1 > $2 (dotted numeric versions).
    local left="$1"
    local right="$2"
    [[ "$(printf '%s\n' "${left}" "${right}" | sort -V | tail -n1)" == "${left}" && "${left}" != "${right}" ]]
}

check_glibc_symbols() {
    local app_dir="$1"
    local max_version="$2"
    local elf_file
    local symbol
    local version
    local too_new=0

    if ! command -v objdump >/dev/null; then
        echo "objdump not found; cannot verify glibc symbol versions" >&2
        exit 1
    fi

    echo "Checking glibc symbol versions in ${app_dir} (max ${max_version})"

    while IFS= read -r -d '' elf_file; do
        while IFS= read -r symbol; do
            version="${symbol#GLIBC_}"
            if version_gt "${version}" "${max_version}"; then
                echo "glibc symbol ${symbol} in ${elf_file} exceeds ${max_version}" >&2
                too_new=1
            fi
        done < <(objdump -T "${elf_file}" 2>/dev/null | grep -oE 'GLIBC_[0-9]+(\.[0-9]+)+' | sort -u || true)
    done < <(find "${app_dir}" -type f \( -perm -111 -o -name '*.so*' \) -print0)

    if [[ "${too_new}" -ne 0 ]]; then
        echo "AppDir requires glibc newer than ${max_version}; rebuild on an older base" >&2
        exit 1
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

    check_glibc_symbols "${app_dir}" "${MAX_GLIBC_VERSION}"

    appimage_path="$(find "${appimage_output_dir}" -maxdepth 1 -type f -name '*.AppImage' -print -quit)"
    if [[ -z "${appimage_path}" ]]; then
        echo "linuxdeploy did not produce an AppImage" >&2
        exit 1
    fi

    mv "${appimage_path}" "${output_path}"
}

echo "Installing dependencies"
"${ROOT_DIR}/scripts/dependencies.sh"

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

build_linuxdeploy_extra_args
echo "Resolved ${#LINUXBUILD_EXTRA_ARGS[@]} linuxdeploy extra library args"

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
