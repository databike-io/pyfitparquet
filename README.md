## PyFitParquet

Perform Garmin FIT/TCX file serialization to parquet format.

___

### Installation and Build:

The suggested build method is to simply ```pip install```. The package will be built implicitly and installed into your conda environment. Upon completion the **pyfitparquet** module will be accessible for import. Please see [SerializationTest.ipynb](https://github.com/databike-io/pyfitparquet/blob/main/notebooks/SerializationTest.ipynb) for usage examples. Also installed in ```${CONDA_PREFIX}/bin``` are two C++ executables that can be run from the command-line: **fitdecoder** ([decoder.cpp](https://github.com/databike-io/pyfitparquet/blob/main/cpp/FitCppSDK_21.40.00/cpp/examples/decode.cpp) provided w/the FitSDK), which prints the contents of a FIT file to stdout, and **fittransformer** ([fittransformer.cc](https://github.com/databike-io/pyfitparquet/blob/main/cpp/fittransformer.cc)), which transforms a FIT file to parquet. Please use [uninstall.sh](https://github.com/databike-io/pyfitparquet/blob/main/uninstall.sh) if you want to remove the **pyfitparquet** package (just using ```pip uninstall pyfitenv``` is not sufficient to remove all components). Thus to install:

```
$ git clone https://github.com/databike-io/pyfitparquet.git
$ cd pyfitparquet
$ conda env create -f environment.yml
$ conda activate pyfitenv
$ pip install .
```

___
### Configuration Files:

Two (well-commented) configuration files are installed to fine-tune serialization behavior: [parquet_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/parquet_config.yml) and [mappings_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/mappings_config.yml). These files can be modified in-place directly at ```${CONDA_PREFIX}/lib/pythonX.X/site-packages/pyfitparquet```, however any re-installation of **pyfitparquet** will overwrite your changes. To maintain your own persistent copies, set the environment variable: ```PYFIT_CONFIG_DIR``` to a directory path of your choice and place local versions of the files there. These files will not be overwritten or removed on uninstall. (Note: if ```PYFIT_CONFIG_DIR``` is set, but **pyfitparquet** cannot find config files there, it will copy in default versions of the files from the current installation.)

___
### Execution:

Please see notebook [SerializationTest.ipynb](https://github.com/databike-io/pyfitparquet/blob/main/notebooks/SerializationTest.ipynb) for Python FIT/TCX-to-parquet serialization examples. To use C++ executables directly from the command-line to serialize a single FIT-file to parquet:
```
$ cd $CONDA_PREFIX/bin
$ fittransformer <FIT_FILE> <PARQUET_FILE>
```

Or to decode a single FIT-file to stdout:
```
$ cd $CONDA_PREFIX/bin
$ fitdecoder <FIT_FILE>
```

___
### TODOs:

- **X** Add Garmin XSD schemas to the repo
- **X** Create complete compound tag mapping (and generate-script) from XSD
- **X** Download and test against all TCX files in https://github.com/cjoakim/ggps/tree/master/data
- **X** Add API controllable parquet_config.yml parameters (maybe?) (**cancelled - requiring config file manipulation from USER and address w/consistent pip/conda install location, this also addresses PYTHONPATH variablity too**)
- **X** Implement pip/conda packaging and build procedure
- Add C++ cout/cerr logging redirects to python sys.stdout, sys.stderr 
- Licensing?? Need to address and add disclaimer comments to src files
- Implement a more definitive pytest sequence
- Create mkdocs page describing python and C++ API (and update repo README)
- Create conda-forge meta.yaml recipe and submit for public download 
