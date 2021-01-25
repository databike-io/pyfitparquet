## Project: PyFitParquet

Perform Garmin FIT/TCX file serialization to parquet format.

![sysdesign](https://github.com/databike-io/pyfitparquet/blob/main/sysdesign_v1.jpg)

___

### Installation and Build:

Two build scripts are provided to be run in order: first [build-arrow.sh](https://github.com/databike-io/pyfitparquet/blob/main/build-arrow.sh), which will clone Apache Arrow (if needed), then build and install it (in *./arrow/cpp/arrow-build* and *./arrow/cpp/arrow-install*). The second: [build-pyfitparquet.sh](https://github.com/databike-io/pyfitparquet/blob/main/build-pyfitparquet.sh) will build (in *./cpp/pyfit-build*) the FitSDK libary, two executables: **decoder** (provided w/FitSDK [decoder.cpp](https://github.com/databike-io/pyfitparquet/blob/main/cpp/FitCppSDK_21.40.00/cpp/examples/decode.cpp)) and **fittransformer** ([fittransformer.cc](https://github.com/databike-io/pyfitparquet/blob/main/cpp/fittransformer.cc)), and a python module: **fittransformer_so** ([fittransformer_so.cc](https://github.com/databike-io/pyfitparquet/blob/main/cpp/fittransformer_so.cc)). The **decoder** prints the contents of a FIT file to stdout, while **fittransformer** performs transformation of FIT files to parquet files, **fittransformer_so** provides the same functionality from python. You can clean the build by deleting *./cpp/pyfit-build*, *./arrow/cpp/arrow-build*, *./arrow/cpp/arrow-install*, or *./arrow* entirely.

```
$ git clone https://github.com/databike-io/pyfitparquet.git
$ cd pyfitparquet
$ conda env create -f pyfitparquet.yml
$ conda activate pyfitparquet
$ ./build-arrow.sh
$ ./build-pyfitparquet.sh
```

___
### Execution:

Please see notebook [SerializationTest.ipynb](https://github.com/databike-io/pyfitparquet/blob/main/notebooks/SerializationTest.ipynb) to test FIT/TCX-to-parquet serialization functionality. Or you can run from the command-line as below. To serialize FIT/TCX files to parquet from command-line:
```
$ cd ./python
$ python pyfitparquet.py <DATA_DIR> --parque_dir <PARQUET_DIR> 
```

(See [pyfitparquet.py](https://github.com/databike-io/pyfitparquet/blob/main/python/pyfitparquet.py) for more nuanced usage) To use C++ executables directly from the command-line to serialize a single FIT-file to parquet:
```
$ cd cpp/pyfit-build
$ fittransformer <FIT_FILE> <PARQUET_FILE>
```

Or to decode a single FIT-file to stdout:
```
$ cd cpp/pyfit-build
$ decoder <FIT_FILE>
```

___
### TODOs:

- Add C++ cout/cerr logging redirects to python sys.stdout, sys.stderr 
- Licensing?? Need to address and add disclaimer comments to src files
- Add Garmin XSD schemas to the repo
- Create complete compound tag mapping (and generate-script) from XSD
- Download and test against all TCX files in https://github.com/cjoakim/ggps/tree/master/data
- Implement a more definitive pytest sequence
- Add API controllable parquet_config.yml parameters (maybe?)
- Create mkdocs page describing python and C++ API (and update repo README)
- Finalize pip/conda packaging and build procedure
