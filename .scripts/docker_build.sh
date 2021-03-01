#!/usr/bin/env bash
set -e

PYFITPARQUET_SCRIPTS=$(cd "$(dirname "$0")"; pwd;)
pushd ${PYFITPARQUET_SCRIPTS} > /dev/null
source ~/.bash_profile
rm -rf dist ; mkdir dist 

# For local sdist
cp ../dist/*.gz dist/.

# For GitHub release download
# wget https://github.com/databike-io/pyfitparquet/releases/download/v1.0/pyfitparquet-1.0.tar.gz
# mv *.gz dist/.

docker build -t conda-builder .
rm -rf dist

echo "Building linux-64 conda packages"
docker run -it conda-builder
popd  > /dev/null
