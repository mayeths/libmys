#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/fmt-*

mkdir -p build && cd build

BUILD_SHARED="OFF"
if [[ "$PKG_LINK" == "shared" ]]; then
    BUILD_SHARED="ON"
fi

cmake .. \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_SHARED_LIBS="$BUILD_SHARED" \
    -DFMT_TEST=OFF \
    -DFMT_DOC=OFF
make -j"$JOBS"
make install
