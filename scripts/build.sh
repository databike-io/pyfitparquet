#!/bin/bash

set -e

THIS_SCRIPT=$(basename $0)
if [ ! -f "$THIS_SCRIPT" ]; then
    echo "ERROR: must execute this script from pyfitparquet scripts directory"
    echo
    exit 1
fi

pushd .. > /dev/null
: ${PYFITPARQUET_ROOT:=${PWD}}
: ${PYFIT_INSTALL:=${PYFITPARQUET_ROOT}/pyfit-install}

echo
echo "=="
echo "== Building FIT lib, fitdecoder, and fittransformer"
echo "=="
echo

cmake -Spyfitparquet/cpp -Bpyfit-build \
    -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${PYFIT_INSTALL} \
    -DINSTALL_SITE_PKGS=${PYFIT_INSTALL}
cmake --build pyfit-build
popd > /dev/null
