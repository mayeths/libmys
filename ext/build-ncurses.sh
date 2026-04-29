#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/ncurses-*

./configure --prefix="$PREFIX" \
    --with-shared=no \
    --with-static=yes \
    --enable-widec \
    --with-cxx-shared=no \
    --without-debug \
    --without-ada \
    --enable-pc-files \
    --with-pkg-config-libdir="$PREFIX/lib/pkgconfig"
make -j"$JOBS"
make install

# Create non-wide symlinks so -lncurses works
cd "$PREFIX/lib"
for lib in libncursesw.a libpanelw.a libmenuw.a libformw.a; do
    target="${lib/w.a/.a}"
    [ -f "$lib" ] && [ ! -f "$target" ] && ln -s "$lib" "$target" || true
done
cd "$PREFIX/include"
[ -d ncursesw ] && [ ! -e ncurses ] && ln -s ncursesw ncurses || true
