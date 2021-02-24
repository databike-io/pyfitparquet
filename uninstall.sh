#!/usr/bin/env bash
set -e

echo
echo "=="
echo "== Uninstalling and cleaning pyfitparquet"
echo "=="
echo

pushd $(dirname "$0") > /dev/null
rm -rf cmake-build/ dist/ build/ pyfitparquet.egg-info/ 
pip uninstall --yes pyfitparquet
popd > /dev/null

pushd ${CONDA_PREFIX} > /dev/null
rm -rf  `find . -name 'pyfitparquet'`
rm -f `find . -name 'libfitsdk.dylib'`
rm -f `find . -name 'fittransformer'`
rm -f `find . -name 'fitdecoder'`
popd > /dev/null
