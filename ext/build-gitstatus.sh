#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

# gitstatus provides prebuilt binaries per platform, no need to compile.
# Download the correct binary for current platform.

mkdir -p "$PREFIX/bin"

SYSTEM=$(uname -s | tr '[:upper:]' '[:lower:]')
MACHINE=$(uname -m)

case "${SYSTEM}-${MACHINE}" in
    linux-x86_64)   ASSET="gitstatusd-linux-x86_64.tar.gz" ;;
    linux-aarch64)  ASSET="gitstatusd-linux-aarch64.tar.gz" ;;
    darwin-arm64)   ASSET="gitstatusd-darwin-arm64.tar.gz" ;;
    darwin-x86_64)  ASSET="gitstatusd-darwin-x86_64.tar.gz" ;;
    *)              echo "ERROR: unsupported platform ${SYSTEM}-${MACHINE}"; exit 1 ;;
esac

BASE_URL="https://github.com/romkatv/gitstatus/releases/download/v${PKG_VERSION}"

cd "$BUILD_DIR"
if [ ! -f "$ASSET" ]; then
    echo "Downloading $ASSET ..."
    curl -fSL -o "$ASSET" "${BASE_URL}/${ASSET}"
fi

mkdir -p _gitstatus && cd _gitstatus
tar xzf "../$ASSET"
# Find the extracted binary (name varies by platform)
BIN=$(find . -type f -name 'gitstatusd*' ! -name '*.tar.gz' | head -1)
if [ -z "$BIN" ]; then
    echo "ERROR: could not find gitstatusd binary after extraction"
    exit 1
fi
cp "$BIN" "$PREFIX/bin/gitstatusd"
chmod 755 "$PREFIX/bin/gitstatusd"
