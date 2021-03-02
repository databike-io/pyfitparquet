#!/usr/bin/env bash
set -e

echo "Cleaning pyfitparquet..."
pushd "$(dirname "$0")/.." > /dev/null
rm -rf cmake-build/ dist/ build/ pyfitparquet.egg-info/ example_output/ site/
rm -f ./*.fit 
rm -f ./*.tcx
popd > /dev/null
