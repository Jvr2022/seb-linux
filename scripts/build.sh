#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BUILD_CONFIG_STAMP="${BUILD_DIR}/.last-cmake-args"
CMAKE_ARGS=("$@")

GENERATOR_ARGS=()
if command -v ninja >/dev/null 2>&1; then
  GENERATOR_ARGS=(-G Ninja)
fi

CMAKE_ARGS_FINGERPRINT="$(printf '%s\0' ${CMAKE_ARGS[@]+"${CMAKE_ARGS[@]}"} | sha256sum | awk '{print $1}')"

PREVIOUS_FINGERPRINT=""
if [[ -f "${BUILD_CONFIG_STAMP}" ]]; then
  PREVIOUS_FINGERPRINT="$(<"${BUILD_CONFIG_STAMP}")"
fi

if [[ -f "${BUILD_DIR}/CMakeCache.txt" && "${PREVIOUS_FINGERPRINT}" != "${CMAKE_ARGS_FINGERPRINT}" ]]; then
  echo "Detected changed CMake configuration; cleaning ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

echo "Configuring build with: cmake -S ${ROOT_DIR} -B ${BUILD_DIR} ${CMAKE_ARGS[*]:-}"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" ${GENERATOR_ARGS[@]+"${GENERATOR_ARGS[@]}"} -DCMAKE_BUILD_TYPE=Release ${CMAKE_ARGS[@]+"${CMAKE_ARGS[@]}"}
printf '%s\n' "${CMAKE_ARGS_FINGERPRINT}" > "${BUILD_CONFIG_STAMP}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "Build output: ${BUILD_DIR}/bin/safe-exam-browser"
