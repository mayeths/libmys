#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)/gitstatus"
    echo "Download to default path $TOP_DIR"
fi

#--- The etc/omz/themes/powerlevel10k/gitstatus/install.info locked gitstatus to v1.5.1 by checking sha256sum.
#--- Copy $TOP_DIR to ~/.cache/gitstatus (not deamon name gitstatusd !)
mkdir -p $TOP_DIR
# wget -O "$TOP_DIR/gitstatusd-cygwin_nt-10.0-i686.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-cygwin_nt-10.0-i686.tar.gz
# wget -O "$TOP_DIR/gitstatusd-cygwin_nt-10.0-i686.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-cygwin_nt-10.0-i686.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-darwin-arm64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-darwin-arm64.tar.gz
# wget -O "$TOP_DIR/gitstatusd-darwin-arm64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-darwin-arm64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-darwin-x86_64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-darwin-x86_64.tar.gz
# wget -O "$TOP_DIR/gitstatusd-darwin-x86_64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-darwin-x86_64.tar.gz.asc
# wget -O "$TOP_DIR/gitstatusd-freebsd-amd64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-freebsd-amd64.tar.gz
# wget -O "$TOP_DIR/gitstatusd-freebsd-amd64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-freebsd-amd64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-aarch64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-aarch64.tar.gz
# wget -O "$TOP_DIR/gitstatusd-linux-aarch64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-aarch64.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-i686.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-i686.tar.gz
# wget -O "$TOP_DIR/gitstatusd-linux-i686.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-i686.tar.gz.asc
# wget -O "$TOP_DIR/gitstatusd-linux-ppc64le.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-ppc64le.tar.gz
# wget -O "$TOP_DIR/gitstatusd-linux-ppc64le.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-ppc64le.tar.gz.asc
wget -O "$TOP_DIR/gitstatusd-linux-x86_64.tar.gz" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-x86_64.tar.gz
# wget -O "$TOP_DIR/gitstatusd-linux-x86_64.tar.gz.asc" https://github.com/romkatv/gitstatus/releases/download/v1.5.1/gitstatusd-linux-x86_64.tar.gz.asc


# KERNEL_NAME=$(uname -s | tr '[:upper:]' '[:lower:]')
# MACHINE_NAME=$(uname -m | tr '[:upper:]' '[:lower:]')
# GITSTATUS_TAR="gitstatusd-$KERNEL_NAME-$MACHINE_NAME.tar.gz"
# tar -xf $GITSTATUS_TAR
# mkdir -p ~/.cache/gitstatus
# mv $GITSTATUS_TAR ~/.cache/gitstatus
