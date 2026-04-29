#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/libevent-*

./configure --prefix="$PREFIX" \
    --disable-shared \
    --disable-openssl \
    CPPFLAGS="$(get_cppflags)" \
    LDFLAGS="$(get_static_ldflags)"
make -j"$JOBS"
make install
