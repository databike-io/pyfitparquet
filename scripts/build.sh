#!/usr/bin/env bash
set -e

PYFITPARQUET_ROOT=$(cd "$(dirname "$0")/.."; pwd;)
: ${PYFIT_INSTALL:=${PYFITPARQUET_ROOT}/pyfit-install}

echo
echo "=="
echo "== Building FIT lib, fitdecoder, and fittransformer"
echo "=="
echo

pushd ${PYFITPARQUET_ROOT} > /dev/null
cmake -Spyfitparquet/cpp -Bcmake-build \
    -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${PYFIT_INSTALL} \
    -DINSTALL_SITE_PKGS=${PYFIT_INSTALL}
cmake --build cmake-build
popd > /dev/null
