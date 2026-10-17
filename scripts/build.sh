#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BUILD_CONFIG_STAMP="${BUILD_DIR}/.last-qmake-args"
QMAKE_ARGS=("$@")
QMAKE_ARGS_FINGERPRINT="$(printf '%s\0' "${QMAKE_ARGS[@]}" | sha256sum | awk '{print $1}')"

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}" >/dev/null

PREVIOUS_QMAKE_ARGS_FINGERPRINT=""
if [[ -f "${BUILD_CONFIG_STAMP}" ]]; then
  PREVIOUS_QMAKE_ARGS_FINGERPRINT="$(<"${BUILD_CONFIG_STAMP}")"
fi

if [[ -f Makefile && "${PREVIOUS_QMAKE_ARGS_FINGERPRINT}" != "${QMAKE_ARGS_FINGERPRINT}" ]]; then
  echo "Detected changed qmake configuration; cleaning ${BUILD_DIR}"
  make distclean >/dev/null 2>&1 || true
fi

echo "Configuring build with: qmake6 ../seb-linux-qt.pro ${QMAKE_ARGS[*]}"
qmake6 ../seb-linux-qt.pro "${QMAKE_ARGS[@]}"
printf '%s\n' "${QMAKE_ARGS_FINGERPRINT}" > "${BUILD_CONFIG_STAMP}"

# --- Interactive Progress Bar Logic ---

is_tty=0
if [[ -t 1 ]]; then
  is_tty=1
fi

# Temporarily disable exit-on-error and pipefail to handle grep exiting with 1 on no matches
set +eo pipefail
total_steps=$(make -n | grep -E '\-o\s+' | grep -Ei '(g\+\+|clang\+\+|moc|rcc|uic)' | wc -l)
set -eo pipefail

if [[ "${total_steps}" -eq 0 ]]; then
  total_steps=1
fi

nproc=$(nproc 2>/dev/null || echo 1)
if [[ "${is_tty}" -eq 1 ]]; then
  echo "Starting parallel build with ${nproc} jobs..."
else
  echo "Starting non-interactive build with ${nproc} jobs (CI/redirect detected)..."
fi

extract_filename() {
  local line="$1"
  local parts=($line)
  local part
  for (( i=${#parts[@]}-1; i>=0; i-- )); do
    part="${parts[i]}"
    part="${part//[\"\']/}"
    if [[ "${part}" =~ \.(cpp|cc|c|h|qrc)$ ]]; then
      echo "${part##*/}"
      return
    fi
  done
  
  for (( i=0; i<${#parts[@]}; i++ )); do
    if [[ "${parts[i]}" == "-o" && $((i+1)) -lt ${#parts[@]} ]]; then
      local outfile="${parts[i+1]}"
      echo "${outfile##*/}"
      return
    fi
  done
}

last_percent=-1

print_progress() {
  local current=$1
  local total=$2
  local file=$3
  
  local percent=$(( current * 100 / total ))
  if [[ "${percent}" -gt 100 ]]; then
    percent=100
  fi
  
  if [[ "${is_tty}" -eq 1 ]]; then
    local bar_length=30
    local filled_length=$(( bar_length * current / total ))
    if [[ "${filled_length}" -gt "${bar_length}" ]]; then
      filled_length="${bar_length}"
    fi
    
    local bar=""
    local i
    for ((i=0; i<filled_length; i++)); do
      bar="${bar}="
    done
    if [[ "${filled_length}" -lt "${bar_length}" ]]; then
      bar="${bar}>"
      local remaining=$(( bar_length - filled_length - 1 ))
      for ((i=0; i<remaining; i++)); do
        bar="${bar} "
      done
    fi
    
    local columns=$(tput cols 2>/dev/null || echo 80)
    local action="Building..."
    if [[ -n "${file}" ]]; then
      action="Compiling: ${file}"
    fi
    
    local status_line="[${bar}] ${percent}% (${current}/${total}) ${action}"
    local max_len=$(( columns - 4 ))
    if [[ "${#status_line}" -gt "${max_len}" ]]; then
      status_line="${status_line:0:max_len}..."
    fi
    
    printf "\r%s\e[K" "${status_line}"
  else
    if [[ "${percent}" -ge $(( last_percent + 10 )) || "${current}" -eq "${total}" || "${current}" -eq 1 ]]; then
      last_percent=$(( percent / 10 * 10 ))
      local action="Building..."
      if [[ -n "${file}" ]]; then
        action="Compiling: ${file}"
      fi
      printf "[%3d%%] (%d/%d) %s\n" "${percent}" "${current}" "${total}" "${action}"
    fi
  fi
}

log_file=$(mktemp)
trap 'rm -f "${log_file}"' EXIT

completed_steps=0
make_exit_code=0

# Temporarily disable exit-on-error to capture and handle make errors manually
set +e
while read -r line; do
  if [[ "${line}" == "EXIT_STATUS:"* ]]; then
    make_exit_code="${line#EXIT_STATUS:}"
    continue
  fi

  echo "${line}" >> "${log_file}"
  
  line_lower=$(echo "${line}" | tr '[:upper:]' '[:lower:]')
  if [[ "${line}" == *"-o "* ]] && ( [[ "${line_lower}" == *"g++"* ]] || [[ "${line_lower}" == *"clang++"* ]] || [[ "${line_lower}" == *"moc"* ]] || [[ "${line_lower}" == *"rcc"* ]] || [[ "${line_lower}" == *"uic"* ]] ); then
    completed_steps=$(( completed_steps + 1 ))
    if [[ "${completed_steps}" -gt "${total_steps}" ]]; then
      total_steps="${completed_steps}"
    fi
    filename=$(extract_filename "${line}")
    print_progress "${completed_steps}" "${total_steps}" "${filename}"
  fi
done < <(make -j"${nproc}" 2>&1; echo "EXIT_STATUS:$?")
set -e

if [[ "${make_exit_code}" -ne 0 ]]; then
  echo -e "\n\nBuild failed! Compiler Output:"
  echo "============================================================"
  cat "${log_file}"
  echo "============================================================"
  exit "${make_exit_code}"
else
  print_progress "${total_steps}" "${total_steps}" "Finished successfully!"
  echo -e "\n\nBuild succeeded!"
fi

popd >/dev/null

target_name="safe-exam-browser"
for arg in "${QMAKE_ARGS[@]}"; do
  if [[ "${arg}" == *"dev_bypass"* ]]; then
    target_name="safe-exam-browser_dev"
    break
  fi
done

echo "Build output: ${BUILD_DIR}/bin/${target_name}"
