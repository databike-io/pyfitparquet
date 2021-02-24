## Conda Package Build Variant Tests

This directory contains primarily dev scripts to facilate preliminary local builds and tests before conda-feedstock submission. From activated ```pyfitenv``` environment in the repo root:
```
$ uninstall.sh
$ python setup.py sdist
```

Replace <PYFIT_VERSION> below appropriately, then use openssl to calc SHA256 (for e.g.):
```
$ openssl sha256 dist/pyfitparquet-<PYFIT_VERSION>.tar.gz
SHA256(dist/pyfitparquet-1.0.tar.gz)= d3cb5a589889b498a14b5d98d0ff341be3a6de9dd7308495a6d28ff735dd5878
```

Copy calculated ```sha256``` into [recipes/meta.yaml](https://github.com/databike-io/pyfitparquet/blob/main/.scripts/recipes/meta.yaml), if needed also adjust ```pyfit_version``` and ```build number```, then:

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

Can attach to docker container with:
```
$ docker ps

# Copy conda-builder <container id>
$ docker exec -it <container id> /bin/bash
```
