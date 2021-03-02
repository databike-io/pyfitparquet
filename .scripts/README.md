## Dev Notes - conda package build tests

Directory contains dev scripts and recipe to perform local conda build tests before conda-feedstock submission. The two tests described below run on a native OSX arch and in a LINUX docker container. Subsequent to these tests, run the conda-forge provided local test procedure described [here](https://conda-forge.org/docs/maintainer/adding_pkgs.html#staging-test-locally)

To test from a local ```sdist``` in an activated ```pyfitenv``` environment in the repo root:
```bash
$ make clean
$ make sdist
running sdist
running egg_info
...
Creating tar archive
removing 'pyfitparquet-1.0' (and everything under it)
SHA256(dist/pyfitparquet-1.0.tar.gz)= 8cd378a0204144977167bbd4687bcb0fa0de5ddd297d7b2447c9a3af5d9f9747
```
Add a local copy of the ```pyfitparquet-feedstock``` [meta.yml](https://github.com/conda-forge/pyfitparquet-feedstock/blob/master/recipe/meta.yaml) file into the local ```.scripts/recipes``` directory and copy/paste the above local tarball path and ```sha256``` appropriately into the file, as well as the ```pyfit_version``` and ```build number```, then exectute test scripts below.  

**Note:** if running test from actual GitHub release rather than a local ```sdist```, adjust the ```docker_build.sh``` to use the wget options, and calculate ```sha256``` with e.g.:

```bash
curl -sL https://github.com/databike-io/pyfitparquet/releases/download/v1.0/pyfitparquet-1.0.tar.gz | openssl sha256
```

```bash
# Build (local) for osx64
cd .scripts
conda_build.sh
```
```bash
# Build (in container) for linux-64
cd .scripts
docker_build.sh
```

Can attach to docker container to verify build output with:
```bash
docker ps

# Copy conda-builder <container id>
docker exec -it <container id> /bin/bash
```

