#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
wget -O "$TOP_DIR/glm-0.9.9.8.zip" https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip