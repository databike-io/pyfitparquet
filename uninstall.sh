#!/bin/bash

set -e

THIS_SCRIPT=$(basename $0)
if [ ! -f "$THIS_SCRIPT" ]; then
    echo "ERROR: must execute this script from pyfitparquet root directory"
    echo
    exit 1
fi

echo
echo "=="
echo "== Uninstalling and cleaning pyfitparquet"
echo "=="
echo

rm -rf pyfit-build pyfit-install
pip uninstall --yes pyfitparquet 
rm -rf ${CONDA_PREFIX}/lib/python3.8/site-packages/pyfitparquet
rm ${CONDA_PREFIX}/lib/libfitsdk.dylib
rm ${CONDA_PREFIX}/bin/fittransformer
rm ${CONDA_PREFIX}/bin/fitdecoder
