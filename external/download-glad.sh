#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
wget -O "$TOP_DIR/glad.zip" https://glad.dav1d.de/generated/tmphvjicpppglad/glad.zip