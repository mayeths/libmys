#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/utf8proc-*

mkdir -p build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_C_FLAGS="-fPIC" \
    -DBUILD_SHARED_LIBS=OFF \
    -DUTF8PROC_ENABLE_TESTING=OFF
make -j"$JOBS"
make install

# Generate .pc file if cmake didn't (needed by tmux's pkg-config check)
if [ ! -f "$PREFIX/lib/pkgconfig/libutf8proc.pc" ]; then
    mkdir -p "$PREFIX/lib/pkgconfig"
    cat > "$PREFIX/lib/pkgconfig/libutf8proc.pc" <<EOF
prefix=${PREFIX}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libutf8proc
Description: UTF-8 processing library
Version: ${PKG_VERSION}
Libs: -L\${libdir} -lutf8proc
Cflags: -I\${includedir}
EOF
fi
