## Project: Fit-Ingest

Perform FIT file data ingest and serialization to parquet.

![sysdesign](https://github.com/breakawayfit/fit-ingest/blob/main/sysdesign_v1.jpg)

___

### Installation and Build:

```
$ git clone https://github.com/breakawayfit/fit-ingest.git
$ git clone https://github.com/apache/arrow.git
$ cd arrow/cpp
$ cmake -H. -Barrow-build -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=ON -DARROW_PARQUET=ON -DARROW_WITH_SNAPPY=ON -DARROW_BUILD_EXAMPLES=ON -DPARQUET_BUILD_EXAMPLES=ON -DCMAKE_INSTALL_PREFIX="`pwd`/arrow-install"
$ cmake --build arrow-build --target install
$ cd ../../fit-ingest/
$ cmake -H. -B_build -DCMAKE_PREFIX_PATH=$CONDA_PREFIX -DCMAKE_INSTALL_PREFIX="`pwd`/../arrow/cpp/arrow-install" -DARROW_LINK_SHARED=ON
$ cmake --build _build 
```

```
export DYLD_LIBRARY_PATH=${CONDA_PREFIX}/lib
```
___

### FIT File Decode Test:

```
$ decoder <FIT_FILE>
```

