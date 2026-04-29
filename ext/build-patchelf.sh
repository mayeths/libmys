#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/patchelf-*

./configure --prefix="$PREFIX" \
    LDFLAGS="$(get_static_ldflags)"
make -j"$JOBS"
make install
