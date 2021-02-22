## PyFitParquet

The ```pyfitparquet``` package provides support for Garmin [FIT](https://developer.garmin.com/fit/overview/) and [TCX](https://en.wikipedia.org/wiki/Training_Center_XML) file ETL into [Apache Parquet](https://parquet.apache.org/) columnar format. It is designed to be used within a conda environment. Two configuration files are used to fine-tune ETL behavior: [parquet_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/parquet_config.yml) and [mapping_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/mapping_config.yml). In general, these files control, respectively, the column and row structure of parquet output files, and the mapping of TCX tag names to FIT/Parquet field_names. Please see verbose comments within the configuration files themselves for greater understanding of their use.  

Though the configuration files can be modified directly in-place under the ```$CONDA_PREFIX``` install tree, any re-installation of ```pyfitparquet``` will revert configuration to the default. To maintain a persistent configuration across installations, set the ```PYFIT_CONFIG_DIR``` environment variable to a directory path of your choice and place local versions of the configuration files there. These files will not be overwritten or removed on uninstall. (Note: if ```PYFIT_CONFIG_DIR``` is set, but ```pyfitparquet``` cannot find the configuration files there, it will copy in default versions of the files from the current conda ```pyfitparquet``` installation.)

___
### Build and Install from Source

The suggested method to build from source is to use ```pip```, which will build libraries implicitly and install them into a conda environment. In addition to installation of the python ```pyfitparquet``` module, two CLI executables are installed: **fitdecoder**, which prints the contents of a FIT file to std::cout, and **fittransformer**, which performs a FIT-to-Parquet file ETL. (Note: build requires C++ compiler). Thus, to build and install:

```
$ git clone https://github.com/databike-io/pyfitparquet.git
$ cd pyfitparquet
$ conda env create -f environment.yml
$ conda activate pyfitenv
$ pip install .
```

Please execute the provided [uninstall.sh](https://github.com/databike-io/pyfitparquet/blob/main/uninstall.sh) to remove the ```pyfitparquet``` package (```pip uninstall pyfitparquet``` alone is **not** sufficient to remove all installed components). 

___
### Execution Quickstart
 
From python in an activated conda environment with ```pyfitparquet``` installed, the following will ETL all FIT/TCX files located in ```data_dir``` and output parquet files to a default ```f'{data_dir}/parquet'``` directory:
```python
>>> from pyfitparquet import transformer
>>> pyfitparq = transformer.PyFitParquet()
>>> pyfitparq.data_to_parquet(data_dir='./test/fixtures')
```

To ETL individual FIT/TCX files:
```python
>>> from pyfitparquet import transformer
>>> pyfitparq = transformer.PyFitParquet()
>>> pyfitparq.source_to_parquet('./test/fixtures/Who_Dares_Sufferfest.fit', parquet_dir='.')
>>> pyfitparq.source_to_parquet('./test/fixtures/45_min_Power_Zone_Ride.tcx', parquet_dir='.')
```

For a more complete example that includes configuration changes and reading/display of parquet files see: [example.py](https://github.com/databike-io/pyfitparquet/blob/main/example.py)

___
### CLI Usage

To use CLI executables to ETL a single FIT-file (**not** TCX) to Parquet-file:
```
$ fittransformer <FIT_FILE_URI> <PARQUET_FILE_URI> 
```

To ETL a single TCX or FIT file to Parquet-file using the Python CLI interface:
```
$ python pyfitparquet/transformer.py <SOURCE_FILE_URI> [-P PARQUET_DIR] 
```

To decode a single FIT-file to std::cout (default functionality provided by Garmin CPP FitSDK):
```
$ fitdecoder <FIT_FILE_URI> 
```
___
### Licenses and Attributions

+ Two licenses are provided with this project: 
    + All files (CPP source and TCX schemas) statically included in this repo under directory [FitCppSDK_21.47.00](https://github.com/databike-io/pyfitparquet/tree/main/pyfitparquet/FitCppSDK_21.47.00) are the property of [Garmin, LTD](https://developer.garmin.com/fit/download/), statically included solely for FitSdk lib build and version control stability, and licensed under [GARMIN_FIT_SDK_LICENSE](https://github.com/databike-io/pyfitparquet/tree/main/pyfitparquet/FitCppSDK_21.47.00/GARMIN_FIT_SDK_LICENSE).  
    + All other source code in this repo is authored by AJ Donich (or other project contributors) and, to the extent legally applicable and not the purview of upstream licenses, provided as open source under [Apache License 2.0](https://github.com/databike-io/pyfitparquet/blob/main/LICENSE).
+ Five TCX [test data files](https://github.com/databike-io/pyfitparquet/tree/main/test/fixtures) were gratefully borrowed from [Chris Joakim's ggps](https://github.com/cjoakim/ggps) project.  
+ The ```pyfitparquet``` package has several upstream open source dependencies (Apache Arrow, Pybind11, Boost, to name a few). We are grateful for these open APIs and acknowledge many different licenses are employed by these projects. Please see [environment.yml](https://github.com/databike-io/pyfitparquet/blob/main/environment.yml) for PyFitParquet's direct dependencies and corresponding respective repos for specific licenses.