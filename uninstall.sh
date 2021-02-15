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

pushd ${CONDA_PREFIX} > /dev/null
rm -rf  `find . -name 'pyfitparquet'`
rm -f `find . -name 'libfitsdk.dylib'`
rm -f `find . -name 'fittransformer'`
rm -f `find . -name 'fitdecoder'`
popd > /dev/null
