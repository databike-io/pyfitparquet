#!/usr/bin/env bash
set -e

PYFITPARQUET_SCRIPTS=$(cd "$(dirname "$0")"; pwd;)
pushd ${PYFITPARQUET_SCRIPTS} > /dev/null
source ~/.bash_profile

rm -rf dist ; mkdir dist
cp ../dist/*.gz dist/.
docker build -t conda-builder .
rm -rf dist

echo "Building linux-64 conda packages"
docker run -it conda-builder
popd  > /dev/null
