#!/usr/bin/env bash
set -e

echo "Uninstalling pyfitparquet..."
pushd $(dirname "$0") > /dev/null
pip uninstall --yes pyfitparquet
pushd ${CONDA_PREFIX} > /dev/null
rm -rf  `find . -name 'pyfitparquet'`
rm -f `find . -name 'libfitsdk.dylib'`
rm -f `find . -name 'fittransformer'`
rm -f `find . -name 'fitdecoder'`
popd > /dev/null
popd > /dev/null
