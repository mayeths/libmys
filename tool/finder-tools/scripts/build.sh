#!/bin/bash

set -euo pipefail

script_dir=$(/usr/bin/dirname "${BASH_SOURCE[0]}")
if ! cd "${script_dir}"; then
    printf 'Cannot enter script directory: %s\n' "${script_dir}" >&2
    exit 1
fi

script_dir=$(pwd -P)
project_dir=$(/usr/bin/dirname "${script_dir}")
if ! cd "${project_dir}"; then
    printf 'Cannot enter project directory: %s\n' "${project_dir}" >&2
    exit 1
fi

if ! command -v xcodegen >/dev/null 2>&1; then
    printf '%s\n' "xcodegen is required. Install it with: brew install xcodegen" >&2
    exit 1
fi

if ! command -v xcodebuild >/dev/null 2>&1; then
    printf '%s\n' "xcodebuild is required. Install the full Xcode application first." >&2
    exit 1
fi

build_dir="${project_dir}/build"
derived_data_dir="${build_dir}/DerivedData"
log_dir="${build_dir}/logs"
app_path="${derived_data_dir}/Build/Products/Release/FinderTools.app"

/bin/mkdir -p "${log_dir}"
xcodegen generate --spec "${project_dir}/project.yml"

xcodebuild \
    -project "${project_dir}/FinderTools.xcodeproj" \
    -scheme FinderTools \
    -configuration Release \
    -derivedDataPath "${derived_data_dir}" \
    CODE_SIGN_STYLE=Manual \
    CODE_SIGN_IDENTITY=- \
    DEVELOPMENT_TEAM= \
    2>&1 | /usr/bin/tee "${log_dir}/xcodebuild.log"

if [ ! -d "${app_path}" ]; then
    printf 'Build completed without producing: %s\n' "${app_path}" >&2
    exit 1
fi

/usr/bin/codesign --verify --deep --strict "${app_path}"
printf 'Built: %s\n' "${app_path}"
