#!/usr/bin/env bash
set -e

PYFITPARQUET_SCRIPTS=$(cd "$(dirname "$0")"; pwd;)
pushd ${PYFITPARQUET_SCRIPTS} > /dev/null

source ~/.bash_profile
conda remove --yes --name cbuilderenv --all
conda env create -f cbuilderenv.yml
conda activate cbuilderenv

echo "Building osx64 conda packages"
conda build -c conda-forge recipes/
popd  > /dev/null
