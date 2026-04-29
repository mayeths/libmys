#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/tmux-*

CONFIGURE_ARGS="--prefix=$PREFIX"
if is_linux; then
    CONFIGURE_ARGS="$CONFIGURE_ARGS --enable-static"
fi
CONFIGURE_ARGS="$CONFIGURE_ARGS --enable-utf8proc"

./configure $CONFIGURE_ARGS \
    CPPFLAGS="$(get_cppflags) -I${PREFIX}/include/ncursesw" \
    LDFLAGS="$(get_static_ldflags)" \
    LIBS="-lncursesw"
make -j"$JOBS"
make install
