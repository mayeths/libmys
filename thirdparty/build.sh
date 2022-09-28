#!/bin/bash
THIRDPARTY_DIR=$(realpath $(dirname $0))
MAYETHS_DIR=$(dirname $THIRDPARTY_DIR)
TARGET_LIB=$MAYETHS_DIR/lib
TARGET_INC=$MAYETHS_DIR/include/mys-thirdparty
LIBRARY_NAME=libmys.a

mkdir -p $TARGET_LIB $TARGET_INC
ARCHIVE_DIR=$(mktemp --directory)
mkdir -p $ARCHIVE_DIR/archive $ARCHIVE_DIR/include
for installer in thirdparty-*.sh; do
    BUILD_DIR=$(mktemp --directory)
    bash $installer $ARCHIVE_DIR/archive $ARCHIVE_DIR/include
    ret=$?
    if [[ $ret -ne 0 ]]; then
        echo ""
        echo "========================================"
        echo "> ERROR: Running $installer failed, check result above."
        echo ">        No operation was applied."
        echo "========================================"
        exit $ret
    fi
    echo ""
done

### archive
echo "========================================"
echo "> Archiving libraries"
echo "========================================"
cd $ARCHIVE_DIR/archive
for libname in *.a; do
    echo "--- $libname"
    EXTRACT_DIR=$libname.extract
    mkdir -p $EXTRACT_DIR
    cd $EXTRACT_DIR
    ar -vx ../$libname
    cd ..
done
ar -crs $LIBRARY_NAME ./*.extract/*.o
cp $LIBRARY_NAME $TARGET_LIB

cd $ARCHIVE_DIR/include
cp -r * $TARGET_INC
echo ""
echo "========================================"
echo "> Library saved to $TARGET_LIB/$LIBRARY_NAME"
echo "> Headers saved to $TARGET_INC"
echo "========================================"

rm -rf $ARCHIVE_DIR
