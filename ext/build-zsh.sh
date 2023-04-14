#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    PREFIX="$1"
else
    PREFIX="./build"
    echo "Using default PREFIX=./build"
fi


if [[ ! -d zsh-5.9 ]]; then
    tar -xf zsh-5.9.tar.xz
fi

cd zsh-5.9
./configure --prefix=$(realpath $PREFIX)
if [[ $? -ne 0 ]]; then
    exit 1
fi

make clean
make -j
if [[ $? -ne 0 ]]; then
    exit 1
fi

make install
if [[ $? -ne 0 ]]; then
    exit 1
fi

#--- You may preppend fpath to correct functions folder
# export fpath=($(realpath ~/module/BASE/share/zsh/5.9/functions) $fpath)
#--- Copy corresponding gitstatusd like gitstatusd-linux-aarch64 to ~/.cache/gitstatus/ (not gitstatusd/)
