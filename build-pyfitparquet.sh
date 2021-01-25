#!/bin/bash

set -e

THIS_SCRIPT=$(basename $0)
if [ ! -f "$THIS_SCRIPT" ]; then
    echo "ERROR: must execute this script from pyfitparquet root directory"
    echo
    exit 1
fi

: ${PYFITPARQUET_ROOT:=${PWD}}
: ${ARROW_CPP:=${PYFITPARQUET_ROOT}/arrow/cpp}
: ${ARROW_INSTALL:=${ARROW_CPP}/arrow-install}

echo
echo "=="
echo "== Building FIT lib, decoder, and fittransformer"
echo "=="
echo

cd $PYFITPARQUET_ROOT/cpp
cmake -H. -Bpyfit-build \
    -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${ARROW_INSTALL}

cmake --build pyfit-build
