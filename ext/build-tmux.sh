#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/tmux-*

CONFIGURE_ARGS="--prefix=$PREFIX"
if is_linux; then
    CONFIGURE_ARGS="$CONFIGURE_ARGS --enable-static"
fi
if is_macos; then
    CONFIGURE_ARGS="$CONFIGURE_ARGS --enable-utf8proc"
else
    CONFIGURE_ARGS="$CONFIGURE_ARGS --disable-utf8proc"
fi

EXTRA_VARS=""
if is_macos; then
    EXTRA_VARS="LIBUTF8PROC_CFLAGS=-I${PREFIX}/include LIBUTF8PROC_LIBS=-L${PREFIX}/lib\ -lutf8proc"
fi

./configure $CONFIGURE_ARGS \
    CPPFLAGS="$(get_cppflags) -I${PREFIX}/include/ncursesw" \
    LDFLAGS="$(get_static_ldflags)" \
    $EXTRA_VARS \
    LIBS="-lncursesw"
make -j"$JOBS"
make install
