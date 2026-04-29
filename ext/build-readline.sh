#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/readline-*

CFLAGS="${CFLAGS:+$CFLAGS }-fPIC" \
./configure --prefix="$PREFIX" \
    --disable-shared \
    --enable-multibyte \
    --with-curses \
    CPPFLAGS="$(get_cppflags) -I${PREFIX}/include/ncursesw" \
    LDFLAGS="$(get_static_ldflags)"
make -j"$JOBS"
make install
