#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null
qmake6 ../seb-linux-qt.pro "$@"
make -j"$(nproc)"
popd >/dev/null

echo "Build output: ${BUILD_DIR}/bin/safe-exam-browser"
