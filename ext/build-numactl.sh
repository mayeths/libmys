#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/numactl-*

./configure --prefix="$PREFIX" \
    --disable-shared \
    LDFLAGS="$(get_static_ldflags)"
make -j"$JOBS"
make install
