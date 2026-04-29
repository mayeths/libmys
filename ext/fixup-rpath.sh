#!/usr/bin/env bash
# Post-build step: fix RPATH/install_name for relocatable deployment.
# Env: PREFIX must be set.

set -euo pipefail
: "${PREFIX:?PREFIX not set}"

echo "Fixing up RPATHs in $PREFIX ..."

if [[ "$(uname)" == "Linux" ]]; then
    PATCHELF="$PREFIX/bin/patchelf"
    if [ ! -x "$PATCHELF" ]; then
        echo "WARNING: patchelf not found at $PATCHELF, skipping Linux RPATH fixup"
        exit 0
    fi

    for bin in "$PREFIX/bin/"*; do
        [ -f "$bin" ] || continue
        if file "$bin" | grep -q "ELF"; then
            "$PATCHELF" --set-rpath '$ORIGIN/../lib' "$bin" 2>/dev/null || true
        fi
    done

    for lib in "$PREFIX/lib/"*.so*; do
        [ -f "$lib" ] || continue
        if file "$lib" | grep -q "ELF"; then
            "$PATCHELF" --set-rpath '$ORIGIN' "$lib" 2>/dev/null || true
        fi
    done

elif [[ "$(uname)" == "Darwin" ]]; then
    for lib in "$PREFIX/lib/"*.dylib; do
        [ -f "$lib" ] || continue
        install_name_tool -id "@rpath/$(basename "$lib")" "$lib" 2>/dev/null || true
    done

    for bin in "$PREFIX/bin/"*; do
        [ -f "$bin" ] || continue
        file "$bin" | grep -q "Mach-O" || continue
        install_name_tool -add_rpath "@executable_path/../lib" "$bin" 2>/dev/null || true
    done
fi

echo "RPATH fixup done."
