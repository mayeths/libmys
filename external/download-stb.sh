#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
wget -O "$TOP_DIR/stb_image.h" https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

echo "NOTE: Legacy file structure of stb in this directory is:"
echo "$TOP_DIR"
echo "  stb-2.27/"
echo "    include/"
echo "      stb_image.h"
echo "    src/"
echo "      stb.c"
echo ""
echo "The content in stb.c is:"
echo "#define STB_IMAGE_IMPLEMENTATION"
echo "#include \"stb_image.h\""
