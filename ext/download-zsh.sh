#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
wget -O "$TOP_DIR/zsh-5.9.tar.xz" https://jaist.dl.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
