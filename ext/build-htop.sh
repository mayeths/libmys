#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/htop-*

CONFIGURE_ARGS="--prefix=$PREFIX --disable-unicode"
if is_linux; then
    CONFIGURE_ARGS="$CONFIGURE_ARGS --enable-static"
fi

./configure $CONFIGURE_ARGS \
    CPPFLAGS="$(get_cppflags) -I${PREFIX}/include/ncursesw" \
    LDFLAGS="$(get_static_ldflags)" \
    LIBS="-lncursesw -lm"
make -j"$JOBS"
make install
