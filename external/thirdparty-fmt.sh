#!/bin/bash
LIBDIR=$1
INCDIR=$2

exit_if_failed() {
    if [[ $1 -ne 0 ]]; then
        exit $1
    fi
}

PROJECT_DIR=$(realpath ./fmt-9.1.0)
BUILD_DIR=$(mktemp --directory)
echo "========================================"
echo "> Building $PROJECT_DIR (in $BUILD_DIR)"
echo "========================================"
cd $BUILD_DIR
cmake -DFMT_TEST=0 -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_INSTALL_PREFIX=./install $PROJECT_DIR
exit_if_failed $?
make -j fmt
exit_if_failed $?
make install
exit_if_failed $?
cp install/lib64/libfmt.a $LIBDIR
exit_if_failed $?
cp -r install/include/fmt $INCDIR
exit_if_failed $?
rm -rf $BUILD_DIR
exit_if_failed $?
