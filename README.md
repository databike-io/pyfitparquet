# PyFitParquet

The ```pyfitparquet``` package provides support for Garmin [FIT](https://developer.garmin.com/fit/overview/) and [TCX](https://en.wikipedia.org/wiki/Training_Center_XML) file ETL into [Apache Parquet](https://parquet.apache.org/) columnar format.
It is designed to be used within a conda environment.

## Install

> See Development - Install From Source (conda package coming soon)

## Quickstart

From python in an activated conda environment with ```pyfitparquet``` installed, the following will ETL all FIT/TCX files located in ```data_dir``` and output parquet files to a default ```f'{data_dir}/parquet'``` directory:

```python
from pyfitparquet import transformer
pyfitparq = transformer.PyFitParquet()
pyfitparq.data_to_parquet(data_dir="/path/to/dir")
```

To ETL individual FIT/TCX files:

```python
from pyfitparquet import transformer
pyfitparq = transformer.PyFitParquet()
pyfitparq.source_to_parquet("path/to/fitfile.fit", parquet_dir='.')
pyfitparq.source_to_parquet("path/to/tcxfile.tcx", parquet_dir='.')
```

For a more runnable look in the [notebooks directory](https://github.com/databike-io/pyfitparquet/tree/main/notebooks) of this repo or a more complete example that includes configuration changes and reading/display of parquet files see: [example.py](https://github.com/databike-io/pyfitparquet/blob/main/example.py)

## CLI

To use CLI executables to ETL a single FIT-file (**not** TCX) to Parquet-file:

```bash
fittransformer <FIT_FILE_URI> <PARQUET_FILE_URI> 
```

To ETL a single TCX or FIT file to Parquet-file using the Python CLI interface:

```bash
python pyfitparquet/transformer.py <SOURCE_FILE_URI> [-P PARQUET_DIR] 
```

To decode a single FIT-file to std::cout (default functionality provided by Garmin CPP FitSDK):

```bash
fitdecoder <FIT_FILE_URI> 
```

## Configuration

Two configuration files are used to fine-tune ETL behavior: [parquet_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/parquet_config.yml) and [mapping_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/mapping_config.yml).
In general, these files control, respectively, the column and row structure of parquet output files, and the mapping of TCX tag names to FIT/Parquet field_names.
Please see verbose comments within the configuration files themselves for greater understanding of their use.  

Though the configuration files can be modified directly in-place under the ```$CONDA_PREFIX``` install tree, any re-installation of ```pyfitparquet``` will revert configuration to the default. To maintain a persistent configuration across installations, set the ```PYFIT_CONFIG_DIR``` environment variable to a directory path of your choice and place local versions of the configuration files there. These files will not be overwritten or removed on uninstall.
(Note: if ```PYFIT_CONFIG_DIR``` is set, but ```pyfitparquet``` cannot find the configuration files there, it will copy in default versions of the files from the current conda ```pyfitparquet``` installation.)

## Development

### Install From Source

The suggested method to build from source is to use ```pip```, which will build libraries implicitly and install them into a conda environment.
In addition to installation of the python ```pyfitparquet``` module, two CLI executables are installed: **fitdecoder** prints the contents of a FIT file to `std::cout`, and **fittransformer** performs a FIT-to-Parquet file ETL.  

**Note:** (1) building from source requires a C++ compiler installed on your system.
(2) True clone of the repo's [FIT/TCX test data files](https://github.com/databike-io/pyfitparquet/tree/main/test/fixtures) requires [Git Large File Storage (LFS)](https://git-lfs.github.com/) is [installed](https://github.com/git-lfs/git-lfs/wiki/Installation) on your system (without LFS, only stubbed placeholder data files are cloned and execution of validation tests and examples referencing this test data will fail).

```bash
git clone https://github.com/databike-io/pyfitparquet.git
cd pyfitparquet
make dev

# And to run tests to validate install:
make test
```

## Licenses and Attributions

+ Two licenses are provided with this project:
  + All files (CPP source and TCX schemas) statically included in this repo under directory [FitCppSDK_21.47.00](https://github.com/databike-io/pyfitparquet/tree/main/pyfitparquet/FitCppSDK_21.47.00) are the property of [Garmin, LTD](https://developer.garmin.com/fit/download/), statically included solely for FitSdk lib build and version control stability, and licensed under [GARMIN_FIT_SDK_LICENSE](https://github.com/databike-io/pyfitparquet/tree/main/pyfitparquet/FitCppSDK_21.47.00/GARMIN_FIT_SDK_LICENSE).
  + All other source code in this repo is authored by AJ Donich (or other project contributors) and, to the extent legally applicable and not the purview of upstream licenses, provided as open source under [Apache License 2.0](https://github.com/databike-io/pyfitparquet/blob/main/LICENSE).
+ Five TCX [test data files](https://github.com/databike-io/pyfitparquet/tree/main/test/fixtures) were gratefully borrowed from [Chris Joakim's ggps](https://github.com/cjoakim/ggps) project.  
+ The ```pyfitparquet``` package has several upstream open source dependencies (Apache Arrow, Pybind11, Boost, to name a few). We are grateful for these open APIs and acknowledge many different licenses are employed by these projects. Please see [environment.yml](https://github.com/databike-io/pyfitparquet/blob/main/environment.yml) for PyFitParquet's direct dependencies and corresponding respective repos for specific licenses.
