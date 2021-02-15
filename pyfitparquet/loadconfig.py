import os, shutil, re, yaml

# Config file sets/dictionaries. Populates ONCE automatically
# below (on import), to re-read config call populate_config())
CONFIG = {}
MESG_TAGS = set()
TIMESTAMP_TAGS = set()
TAG_FIELD_EXCLUDES = set()
TAG_FIELD_MAP = {}
config_dir = None

# Init global sets/dicts 
def populate_config():
#{
    global CONFIG
    global MESG_TAGS
    global TIMESTAMP_TAGS
    global TAG_FIELD_EXCLUDES
    global TAG_FIELD_MAP
    global config_dir

    # Reset
    CONFIG.clear()
    MESG_TAGS.clear()
    TIMESTAMP_TAGS.clear()
    TAG_FIELD_EXCLUDES.clear()
    TAG_FIELD_MAP.clear()
    config_dir = None

    # Get possible config directory locations
    conda_prefix_env = os.getenv("CONDA_PREFIX")
    pyfit_config_env = os.getenv("PYFIT_CONFIG_DIR")
    parquet_config, mapping_config = None, None
    
    # Find base site-packages config files 
    # or local copies in PYFIT_CONFIG_DIR 
    if pyfit_config_env:
    #{
        parquet_config = os.path.join(pyfit_config_env, 'parquet_config.yml')
        mapping_config = os.path.join(pyfit_config_env, 'mapping_config.yml')
        parquet_config_base = find_file("parquet_config.yml", conda_prefix_env)
        mapping_config_base = find_file("mapping_config.yml", conda_prefix_env)

        if parquet_config_base and not os.path.isfile(parquet_config):
            shutil.copyfile(parquet_config_base, parquet_config)

        if mapping_config_base and not os.path.isfile(mapping_config):
            shutil.copyfile(mapping_config_base, mapping_config)
    #}
    elif conda_prefix_env:
        parquet_config = find_file("parquet_config.yml", conda_prefix_env)
        mapping_config = find_file("mapping_config.yml", conda_prefix_env)

    assert parquet_config and mapping_config and \
        os.path.isfile(parquet_config) and os.path.isfile(parquet_config), \
        f"ERROR: unable to find parquet_config.yml and/or mapping_config.yml"
    config_dir = os.path.dirname(parquet_config)

    # Populate server with 'param : value' pairs
    with open(parquet_config) as config_fhandle:
        rg_comment = r"\s*\#.*"
        rg_parameter = r"\s*(\w+)\s*:\s*(\w+).*"
        for line in config_fhandle.readlines():            
            if re.match(rg_comment, line): continue
            matchobj = re.match(rg_parameter, line)
            if matchobj: CONFIG[matchobj.group(1)] = matchobj.group(2)
    
    # Reset all TAG map/set global members
    with open(mapping_config) as mapping_fhandle:
        try:
            mappings = yaml.safe_load(mapping_fhandle)
            MESG_TAGS = set(mappings['MESG_TAGS'])
            TIMESTAMP_TAGS = set(mappings['TIMESTAMP_TAGS'])
            TAG_FIELD_EXCLUDES = set(mappings['TAG_FIELD_EXCLUDES'])
            TAG_FIELD_MAP = mappings['TAG_FIELD_MAP']  
            for endpoint, triplet in TAG_FIELD_MAP.items():
                triplet = [None if item == 'None' else item for item in triplet]
                TAG_FIELD_MAP[endpoint] = triplet
        except yaml.YAMLError as exc: print(exc)
#}

def find_file(name, start_dir):
    if start_dir is None: return None
    for root, dirs, files in os.walk(start_dir):
        if name in files or name in dirs: 
            return os.path.join(root, name)

# Populate on import
populate_config()
