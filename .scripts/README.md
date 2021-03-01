## Dev Notes - conda package build tests

Directory contains dev scripts and recipe to perform local conda build tests before conda-feedstock submission. The two tests described below run on a native OSX arch and in a LINUX docker container. Subsequent to these tests, run the conda-forge provided local test procedure described [here](https://conda-forge.org/docs/maintainer/adding_pkgs.html#staging-test-locally)

To test from a local ```sdist``` in an activated ```pyfitenv``` environment in the repo root:
```
$ uninstall.sh
$ python setup.py sdist

# Replace <PYFIT_VERSION> below appropriately:
$ openssl sha256 dist/pyfitparquet-<PYFIT_VERSION>.tar.gz
SHA256(dist/pyfitparquet-1.0.tar.gz)= d3cb5a589889b498a14b5d98d0ff341be3a6de9dd7308495a6d28ff735dd5878
```
Add a local copy of the ```pyfitparquet-feedstock``` [meta.yml](https://github.com/conda-forge/pyfitparquet-feedstock/blob/master/recipe/meta.yaml) file into the local recipes directory, copy/paste the calculated ```sha256``` appropriately into the file, and if needed adjust ```pyfit_version``` and ```build number``` before exectuting test scripts below. (Note: if running test from a GitHub release URL, adjust the ```docker_build.sh``` to use the wget options):

```
# Build (local) for osx64
$ cd .scripts
$ conda_build.sh
```
```
# Build (in container) for linux-64
$ cd .scripts
$ docker_build.sh
```

Can attach to docker container to verify build output with:
```
$ docker ps

# Copy conda-builder <container id>
$ docker exec -it <container id> /bin/bash
```

