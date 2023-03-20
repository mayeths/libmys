#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR/tmux
wget -O "$TOP_DIR/tmux/tmux-3.3a.tar.gz" https://github.com/tmux/tmux/releases/download/3.3a/tmux-3.3a.tar.gz
