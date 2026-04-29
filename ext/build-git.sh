#!/usr/bin/env bash
source "$(dirname "$0")/common.sh"

download_and_extract "$URL"
cd "$BUILD_DIR"/git-*

# Uses system curl/openssl for https support.
# Disable optional components that need perl/python/tcl.
make prefix="$PREFIX" \
    NO_TCLTK=1 \
    NO_GETTEXT=1 \
    NO_PERL=1 \
    NO_PYTHON=1 \
    CPPFLAGS="$(get_cppflags)" \
    LDFLAGS="$(get_static_ldflags)" \
    -j"$JOBS"

make prefix="$PREFIX" \
    NO_TCLTK=1 \
    NO_GETTEXT=1 \
    NO_PERL=1 \
    NO_PYTHON=1 \
    install
