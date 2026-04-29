#!/usr/bin/env bash
# Common functions for build scripts.
# Source this at the top of each build-xxx.sh:
#   source "$(dirname "$0")/common.sh"

set -euo pipefail

: "${PREFIX:?PREFIX not set}"
: "${JOBS:=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
: "${URL:?URL not set}"
: "${PKG_VERSION:?PKG_VERSION not set}"
: "${PKG_LINK:=static}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PREFIX}/_build"
mkdir -p "$BUILD_DIR"

export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

download_and_extract() {
    local url="$1"
    local filename
    filename=$(basename "$url")

    cd "$BUILD_DIR"
    if [ ! -f "$filename" ]; then
        echo "Downloading $filename ..."
        curl -fSL -o "$filename" "$url"
    fi

    echo "Extracting $filename ..."
    case "$filename" in
        *.tar.gz|*.tgz)   tar xzf "$filename" ;;
        *.tar.xz)         tar xJf "$filename" ;;
        *.tar.bz2)        tar xjf "$filename" ;;
        *.zip)            unzip -qo "$filename" ;;
        *)                echo "Unknown archive format: $filename"; exit 1 ;;
    esac
}

get_static_ldflags() {
    local flags="-L${PREFIX}/lib"
    if [[ "$(uname)" == "Linux" ]]; then
        flags="$flags -static-libgcc -static-libstdc++"
    fi
    echo "$flags"
}

get_cppflags() {
    echo "-I${PREFIX}/include"
}

is_linux() { [[ "$(uname)" == "Linux" ]]; }
is_macos() { [[ "$(uname)" == "Darwin" ]]; }
