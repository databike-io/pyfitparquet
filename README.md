## Project: Fit-Ingest

Perform FIT file data ingest and serialization to parquet.

![sysdesign](https://github.com/databike-io/fit-ingest/blob/main/sysdesign_v1.jpg)

___

### Installation and Build:

Two build scripts are provided to be run in order: first [build-arrow.sh](https://github.com/databike-io/fit-ingest/blob/main/build-arrow.sh), which will clone Apache Arrow (if needed), then build and install it (in *./arrow/cpp/arrow-build* and *./arrow/cpp/arrow-install*). The second: [build-fit-ingest.sh](https://github.com/databike-io/fit-ingest/blob/main/build-fit-ingest.sh) will build (in *./cpp/fit-build*) the FitSDK libary, two executables: **decoder** (provided w/FitSDK [decoder.cpp](https://github.com/databike-io/fit-ingest/blob/main/cpp/FitCppSDK_21.40.00/cpp/examples/decode.cpp)) and **fitparquet** ([fitparquet.cc](https://github.com/databike-io/fit-ingest/blob/main/cpp/fitparquet.cc)), and a python module: **pyfitparquet_so** ([pyfitparquet.cc](https://github.com/databike-io/fit-ingest/blob/main/cpp/pyfitparquet.cc)). The **decoder**  prints the contents of a FIT file to stdout, while **fitparquet** performs transformation of FIT files to parquet files, **pyfitparquet_so** provides the same functionality from python. You can clean the build by deleting *./cpp/fit-build*, *./arrow/cpp/arrow-build*, *./arrow/cpp/arrow-install*, or *./arrow* entirely.

```
$ git clone https://github.com/databike-io/fit-ingest.git
$ cd fit-ingest
$ conda env create -f databike.yml
$ conda activate databike
$ ./build-arrow.sh
$ ./build-fit-ingest.sh
```
___

### Execution:

You can run notebook [FitToParquetTest.ipynb](https://github.com/databike-io/fit-ingest/blob/main/notebooks/FitToParquetTest.ipynb) (which uses **pyfitparquet_so**) to test serialization functionality on a data directory of your own FIT files. Or you can run from the command-line (using **fitparquet** directly) as below.  

To serialize a single FIT-file to parquet-file from command-line:
```
$ cd cpp/fit-build
$ fitparquet <FIT_FILE> <PARQUET_FILE>
```

To decode a single FIT-file to stdout from command-line:
```
$ cd cpp/fit-build
$ decoder <FIT_FILE>
```

### TODOs:

- Add C++ cout/cerr logging redirects to python sys.stdout, sys.stderr 
- Licensing?? Need to address and add disclaimer comments to src files 


