import os, shutil, yaml, pandas, pyarrow
from pyfitparquet import transformer

# The following script is expected to to be run from the pyfitparquet root directory.
# It executes ETLs on all test FIT/TCX files, outputs them to a created ./example_output 
# directory, and prints summaries of a few of them. It then modifies one of the local
# configuration files to alter parquet column configuration, and re-runs ETLs and
# display summaries on a couple of the same files. 


# Source and output file locations
DATA_DIR, OUTPUT_DIR = './tests/fixtures', './example_output'
if not os.path.exists(OUTPUT_DIR): os.mkdir(OUTPUT_DIR)

# Keep local copies of configuration files
os.environ['PYFIT_CONFIG_DIR'] = OUTPUT_DIR

# Serialize all FIT/TCX files in DATA_DIR to parquet
print("Serializing FIT/TCX files...")
pyfitparq = transformer.PyFitParquet()
pyfitparq.data_to_parquet(DATA_DIR, parquet_dir=OUTPUT_DIR)

# Display 
df_fit = pandas.read_parquet('example_output/Who_Dares_Sufferfest.parquet', engine='pyarrow')
df_tcx = pandas.read_parquet('example_output/45_min_Power_Zone_Ride.parquet', engine='pyarrow')
df_fit_big = pandas.read_parquet('example_output/Bolt_GPS.parquet', engine='pyarrow')
print('Displaying: example_output/Bolt_GPS.parquet\n', df_fit_big)
print('Displaying: example_output/Who_Dares_Sufferfest.parquet\n', df_fit)
print('Displaying: example_output/45_min_Power_Zone_Ride.parquet\n', df_tcx)

# Make a configuration file change
print("Modifying columns in local parquet_config.yml")
local_config_file = 'example_output/parquet_config.yml'
local_config_file_tmp = 'example_output/parquet_config.yml.tmp'
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
print("Serializing FIT/TCX files...")
pyfitparq.source_to_parquet('tests/fixtures/Who_Dares_Sufferfest.fit', parquet_dir=OUTPUT_DIR)
pyfitparq.source_to_parquet('tests/fixtures/45_min_Power_Zone_Ride.tcx', parquet_dir=OUTPUT_DIR)

# Display 
df_fit = pandas.read_parquet('example_output/Who_Dares_Sufferfest.parquet', engine='pyarrow')
df_tcx = pandas.read_parquet('example_output/45_min_Power_Zone_Ride.parquet', engine='pyarrow')
print('Displaying: example_output/Who_Dares_Sufferfest.parquet\n', df_fit)
print('Displaying: example_output/45_min_Power_Zone_Ride.parquet\n', df_tcx)