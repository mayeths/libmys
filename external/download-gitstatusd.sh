#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)/gitstatusd"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
wget -O "$TOP_DIR/gitstatusd-cygwin_nt-10.0-i686.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-cygwin_nt-10.0-i686.tar.gz
wget -O "$TOP_DIR/gitstatusd-cygwin_nt-10.0-i686.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-cygwin_nt-10.0-i686.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-darwin-arm64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-darwin-arm64.tar.gz
wget -O "$TOP_DIR/gitstatusd-darwin-arm64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-darwin-arm64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-darwin-x86_64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-darwin-x86_64.tar.gz
wget -O "$TOP_DIR/gitstatusd-darwin-x86_64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-darwin-x86_64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-freebsd-amd64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-freebsd-amd64.tar.gz
wget -O "$TOP_DIR/gitstatusd-freebsd-amd64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-freebsd-amd64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-aarch64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-aarch64.tar.gz
wget -O "$TOP_DIR/gitstatusd-linux-aarch64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-aarch64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-i686.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-i686.tar.gz
wget -O "$TOP_DIR/gitstatusd-linux-i686.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-i686.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-ppc64le.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-ppc64le.tar.gz
wget -O "$TOP_DIR/gitstatusd-linux-ppc64le.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-ppc64le.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-x86_64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-x86_64.tar.gz
wget -O "$TOP_DIR/gitstatusd-linux-x86_64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd-linux-x86_64.tar.gz.asc