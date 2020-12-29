#!/bin/bash

set -e

THIS_SCRIPT=$(basename $0)
if [ ! -f "$THIS_SCRIPT" ]; then
    echo "ERROR: must execute this script from fit-ingest root directory"
    echo
    exit 1
fi

: ${FIT_INGEST_ROOT:=${PWD}}
: ${ARROW_CPP:=${FIT_INGEST_ROOT}/arrow/cpp}
: ${ARROW_INSTALL:=${ARROW_CPP}/arrow-install}

if [ ! -d "$ARROW_CPP" ]; then
    echo
    echo "=="
    echo "== Cloning Apache Arrow C++ repo"
    echo "=="
    echo
    git clone https://github.com/apache/arrow.git
fi

echo
echo "=="
echo "== Building Arrow C++ library"
echo "=="
echo
cd $ARROW_CPP
if [ ! -d "$ARROW_CPP/arrow-build" ]; then
    cmake -H. -Barrow-build \
        -DARROW_BUILD_STATIC=ON \
        -DARROW_BUILD_SHARED=ON \
        -DARROW_PARQUET=ON \
        -DARROW_WITH_SNAPPY=ON \
        -DARROW_BUILD_EXAMPLES=ON \
        -DPARQUET_BUILD_EXAMPLES=ON \
        -DCMAKE_INSTALL_PREFIX=${ARROW_INSTALL}
fi

cmake --build arrow-build --target install
