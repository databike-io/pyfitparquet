import os, re

CONFIG_FILE_NAME = 'parquet_config.yml'
REPO_ROOT_DIRECTORY = 'fit-ingest'

# Config file dictionary. Note: populates ONCE automatically
# below (on import), to re-read call loadconfig.populate_config())
CONFIG = {}

# Init CONFIG from CONFIG_FILE_NAME
def populate_config():
#{
    # Resets if needed
    CONFIG.clear()

    # Find CONFIG_FILE_NAME (if exists) in REPO_ROOT_DIRECTORY
    start_dir, config_uri, stop_path = "..", None, None
    while config_uri is None and stop_path is None:
        config_uri = _find_file(CONFIG_FILE_NAME, start_dir)
        stop_path = _find_file(REPO_ROOT_DIRECTORY, start_dir)
        start_dir += "/.."

    assert config_uri, f"ERROR: unable to find {CONFIG_FILE_NAME}"

    # Populate server with 'param : value' pairs
    with open(config_uri) as config_fhandle:
        rg_comment = r"\s*\#.*"
        rg_parameter = r"\s*(\w+)\s*:\s*(\w+).*"
        for line in config_fhandle.readlines():            
            if re.match(rg_comment, line): continue
            matchobj = re.match(rg_parameter, line)
            if matchobj: CONFIG[matchobj.group(1)] = matchobj.group(2)
#}

def _find_file(name, start_dir):
    for root, dirs, files in os.walk(start_dir):
        if name in files or name in dirs: 
            return os.path.join(root, name)

# Populate on import
populate_config()
