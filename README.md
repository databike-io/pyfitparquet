## Project: Fit-Ingest

Perform FIT file data ingest and serialization to parquet.

![sysdesign](https://github.com/breakawayfit/fit-ingest/blob/main/sysdesign_v1.jpg)

___

### Installation and Build:

Two build scripts are provided to be run in order as below: first [build-arrow.sh](https://github.com/breakawayfit/fit-ingest/blob/main/build-arrow.sh), will clone Apache Arrow (if needed), create the Arrow build system using CMake (in *./arrow/cpp/arrow-build* and *./arrow/cpp/arrow-install*) and then build and install it. The second: [build-fit-ingest.sh](https://github.com/breakawayfit/fit-ingest/blob/main/build-fit-ingest.sh) will create the Fit-Ingest build system (in *./_build*), then build the FitSDK libary and two executables: **decoder** ([decoder.cpp](https://github.com/breakawayfit/fit-ingest/blob/main/decode.cpp)) and **fitparquet** ([fitparquet.cpp](https://github.com/breakawayfit/fit-ingest/blob/main/fitparquet.cpp)) there. The **decoder** just prints the contents of a fit file to stdout, while **fitparquet** performs full serialization of fit files to parquet files. Note: the build scripts can be run multiple times without recloning or recreating build systems. You can clean to various levels by simply deleting *./_build*, *./arrow/cpp/arrow-build*, *./arrow/cpp/arrow-install*, or even *./arrow* entirely.

```
$ git clone https://github.com/breakawayfit/fit-ingest.git
$ cd fit-ingest
$ conda env create -f databike.yml
$ conda activate databike
$ ./build-arrow.sh
$ ./build-fit-ingest.sh
```
___

### Execution:

You can run [FitToParquetTest.ipynb](https://github.com/breakawayfit/fit-ingest/blob/main/FitToParquetTest.ipynb) in jupyter for an example test of current serialization functionality on a data directory of your own .fit files. You can also run from the command-line directly as below. Note: currently **fitparquet** can only link libraries dynamically, some of which must be pulled from your conda install. Consequently, you must either export/set the DYLD_LIBRARY_PATH environment variable appropriately to your databike: ${CONDA_PREFIX}/lib, which will likely screw up some other command-line functionality in that terminal, or (suggested) feed the variable in on the command-line for each run as below. 

To serialize a single .fit to .parquet file from command-line:
```
$ cd _build
$ DYLD_LIBRARY_PATH=${CONDA_PREFIX}/lib fitparquet <FIT_FILE> <PARQUET_FILE>
```

To decode a single .fit file to stdout from command-line:
```
$ cd _build
$ decoder <FIT_FILE>
```
