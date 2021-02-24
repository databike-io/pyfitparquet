#!/usr/bin/env bash
set -e

PYFITPARQUET_ROOT=$(cd "$(dirname "$0")/.."; pwd;)
: ${ARROW_CPP:=${PYFITPARQUET_ROOT}/arrow/cpp}
: ${ARROW_INSTALL:=${ARROW_CPP}/arrow-install}
: ${ARROW_PYTHON:=${PYFITPARQUET_ROOT}/arrow/python}

pushd ${PYFITPARQUET_ROOT} > /dev/null
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
pushd $ARROW_CPP > /dev/null
cmake -S. -Barrow-build \
    -DARROW_BUILD_STATIC=OFF \
    -DARROW_BUILD_SHARED=ON \
    -DARROW_PARQUET=ON \
    -DARROW_PYTHON=OFF \
    -DARROW_WITH_SNAPPY=ON \
    -DARROW_BUILD_EXAMPLES=OFF \
    -DPARQUET_BUILD_EXAMPLES=OFF \
    -DCMAKE_INSTALL_PREFIX=${ARROW_INSTALL}

cmake --build arrow-build --target install
popd  > /dev/null
popd  > /dev/null