#!/bin/bash

if [[ $# -ne 2 ]]; then
    echo "[USAGE] bash $0 host path"
    echo "        bash $0 kphuanghp ~/project/libmys/"
    exit 1
fi

host=$1
path=$2

rsync --exclude="ugly/" --exclude="lib/" -avz $host:$path .
