## Overview

The ```conda-forge::pyfitparquet``` package provides ETL support for Garmin [FIT](https://developer.garmin.com/fit/overview/) and [TCX](https://en.wikipedia.org/wiki/Training_Center_XML) files into [Apache Parquet](https://parquet.apache.org/) columnar format. Two configuration files are used to fine-tune ETL behavior: [parquet_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/parquet_config.yml) and [mapping_config.yml](https://github.com/databike-io/pyfitparquet/blob/main/pyfitparquet/mapping_config.yml). In general, these files control, respectively, the column and row structure of parquet output files, and the mapping of XML tag names (in TCX files) to the parquet field_name column. Please see verbose comments within the configuration files themselves for greater understanding of their use.

Though the configuration files can be modified directly in-place in your conda environment (under ```$CONDA_PREFIX``` install tree), any re-installation of ```pyfitparquet``` will revert your changes to the defaults. To maintain a persistent configuration across installations, set the ```PYFIT_CONFIG_DIR``` environment variable to a directory path of your choice and place local versions of the files there. These files will not be overwritten or removed on uninstall. (Note: if ```PYFIT_CONFIG_DIR``` is set, but ```pyfitparquet``` cannot find the configuration files there, it will copy in default versions of the files from the current conda ```pyfitparquet``` installation.)

___
### Python Code Example

```python
import os, shutil, yaml, pandas, pyarrow
from pyfitparquet import transformer

# Source and output file locations
DATA_DIR, OUTPUT_DIR = './tests/fixtures', './tmp_output'
if not os.path.exists(OUTPUT_DIR): os.mkdir(OUTPUT_DIR)

# Keep local copies of configuration files
os.environ['PYFIT_CONFIG_DIR'] = OUTPUT_DIR

# Serialize all FIT/TCX files in DATA_DIR to parquet
pyfitparq = transformer.PyFitParquet()
pyfitparq.data_to_parquet(DATA_DIR, parquet_dir=OUTPUT_DIR)

# Display 
df_fit = pandas.read_parquet('tmp_output/Who_Dares_Sufferfest.parquet', engine='pyarrow')
df_tcx = pandas.read_parquet('tmp_output/45_min_Power_Zone_Ride.parquet', engine='pyarrow')
print('Displaying: tmp_output/Who_Dares_Sufferfest.parquet\n', df_fit)
print('Displaying: tmp_output/45_min_Power_Zone_Ride.parquet\n', df_tcx)

# Make a configuration file change
local_config_file = 'tmp_output/parquet_config.yml'
local_config_file_tmp = 'tmp_output/parquet_config.yml.tmp'
with open(local_config_file) as read_fhandle:
#{
    pconfig_map = yaml.safe_load(read_fhandle)
    pconfig_map['epoch_format'] = 'FIT'
    pconfig_map['source_filetype'] = True
    pconfig_map['source_filename'] = False

    with open(local_config_file_tmp, 'w') as write_fhandle:
        yaml.safe_dump(pconfig_map, write_fhandle)
#}

shutil.move(local_config_file_tmp, local_config_file)
pyfitparq.reset_from_config()

# Serialize single FIT/TCX file to parquet
pyfitparq.source_to_parquet('tests/fixtures/Who_Dares_Sufferfest.fit', parquet_dir=OUTPUT_DIR)
pyfitparq.source_to_parquet('tests/fixtures/45_min_Power_Zone_Ride.tcx', parquet_dir=OUTPUT_DIR)

# Display 
df_fit = pandas.read_parquet('tmp_output/Who_Dares_Sufferfest.parquet', engine='pyarrow')
df_tcx = pandas.read_parquet('tmp_output/45_min_Power_Zone_Ride.parquet', engine='pyarrow')
print('Displaying: tmp_output/Who_Dares_Sufferfest.parquet\n', df_fit)
print('Displaying: tmp_output/45_min_Power_Zone_Ride.parquet\n', df_tcx)
```

___
### Building from Source

The suggested method to build from source is to use ```pip```, which will build libraries implicitly and install them into a conda environment. In addition to installation of the python module, two CLI executables are installed: **fitdecoder**, which prints the contents of a FIT file to std::cout, and **fittransformer**, which performs a FIT-to-Parquet file ETL. Thus to build and install:

```
$ git clone https://github.com/databike-io/pyfitparquet.git
$ cd pyfitparquet
$ conda env create -f environment.yml
$ conda activate pyfitenv
$ pip install .
```

Please use the provided [uninstall.sh](https://github.com/databike-io/pyfitparquet/blob/main/uninstall.sh) if you want to remove the ```pyfitparquet``` package (just using ```pip uninstall pyfitenv``` is **not** sufficient to remove all components). 

___
### CLI Usage:

To use CLI executables ETL a single FIT-file to Parquet-file (does **not** support TCX files):
```
$ fittransformer <FIT_FILE_URI> <PARQUET_FILE_URI> 
```

To decode a single FIT-file to std::cout (default functionality provided by Garmin CPP FitSDK):
```
$ fitdecoder <FIT_FILE_URI> 
```

To ETL TCX (or FIT) file to Parquet-file using the Python interface:
```
python pyfitparquet/transformer.py <SOURCE_FILE_URI> [-P PARQUET_DIR] 
```
