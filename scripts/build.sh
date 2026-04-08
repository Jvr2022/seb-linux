#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BUILD_CONFIG_STAMP="${BUILD_DIR}/.last-qmake-args"
QMAKE_ARGS=("$@")
QMAKE_ARGS_SERIALIZED="$(printf '%s\n' "${QMAKE_ARGS[@]}")"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

PREVIOUS_QMAKE_ARGS=""
if [[ -f "${BUILD_CONFIG_STAMP}" ]]; then
  PREVIOUS_QMAKE_ARGS="$(cat "${BUILD_CONFIG_STAMP}")"
fi

if [[ -f Makefile && "${PREVIOUS_QMAKE_ARGS}" != "${QMAKE_ARGS_SERIALIZED}" ]]; then
  echo "Detected changed qmake configuration; cleaning ${BUILD_DIR}"
  make distclean >/dev/null 2>&1 || true
fi

echo "Configuring build with: qmake6 ../seb-linux-qt.pro ${QMAKE_ARGS[*]}"
qmake6 ../seb-linux-qt.pro "${QMAKE_ARGS[@]}"
printf '%s' "${QMAKE_ARGS_SERIALIZED}" > "${BUILD_CONFIG_STAMP}"
make -j"$(nproc)"
popd >/dev/null

echo "Build output: ${BUILD_DIR}/bin/safe-exam-browser"
