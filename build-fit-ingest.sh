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

echo
echo "=="
echo "== Building FIT lib, decoder, and fitparquet"
echo "=="
echo

cd $FIT_INGEST_ROOT/cpp
cmake -H. -Bfit-build \
    -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${ARROW_INSTALL}

cmake --build fit-build
