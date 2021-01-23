import os, re

CONFIG_FILE_NAME = 'parquet_config.yml'
REPO_ROOT_DIRECTORY = 'fit-ingest'

# Config file dictionary. Note: populates ONCE automatically
# below (on import), to re-read call loadconfig.populate_config())
CONFIG = {}

# Tags defined (by convention) comparable to FIT mesgs
MESG_TAGS = {'Activity', 'Creator', 'Lap', 'Trackpoint'}

# Tags endpoints used to overwrite current timestamp column value
TIMESTAMP_TAGS = {'TrainingCenterDatabase-Activities-Activity-Id',
                  'TrainingCenterDatabase-Activities-Activity-Lap-StartTime',
                  'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-Time'}

# Tag endpoints (likely w/a valid field_value) to exclude/skip/ignore 
TAG_FIELD_EXCLUDES = {'TrainingCenterDatabase-schemaLocation',
                      'TrainingCenterDatabase-noNamespaceSchemaLocation',
                      'TrainingCenterDatabase-Activities-Activity-Creator-type'}

# Tag endpoint mappings : [mesg_name, field_name, units]
TAG_FIELD_MAP = {
    'TrainingCenterDatabase-Activities-Activity-Sport': ['Activity', 'sport', None],
    'TrainingCenterDatabase-Activities-Activity-Id': ['Activity', 'id', None],

    'TrainingCenterDatabase-Activities-Activity-Creator-Name': ['Creator', 'name', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-ProductID': ['Creator', 'product_id', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-UnitId': ['Creator', 'unit_id', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-Version-BuildMajor': ['Creator', 'build_major', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-Version-BuildMinor': ['Creator', 'build_minor', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-Version-VersionMajor': ['Creator', 'version_major', None],
    'TrainingCenterDatabase-Activities-Activity-Creator-Version-VersionMinor': ['Creator', 'version_minor', None],

    'TrainingCenterDatabase-Activities-Activity-Lap-StartTime': ['Lap', 'start_time', 's'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Calories': ['Lap', 'calories', 'kcal'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Intensity': ['Lap', 'intensity', None],
    'TrainingCenterDatabase-Activities-Activity-Lap-TriggerMethod': ['Lap', 'trigger_method', None],
    'TrainingCenterDatabase-Activities-Activity-Lap-MaximumSpeed': ['Lap', 'maximum_speed', 'm/s'],
    'TrainingCenterDatabase-Activities-Activity-Lap-DistanceMeters': ['Lap', 'distance', 'm'],
    'TrainingCenterDatabase-Activities-Activity-Lap-TotalTimeSeconds': ['Lap', 'total_time', 's'],
    'TrainingCenterDatabase-Activities-Activity-Lap-AverageHeartRateBpm-Value': ['Lap', 'average_heart_rate', 'bpm'],
    'TrainingCenterDatabase-Activities-Activity-Lap-MaximumHeartRateBpm-Value': ['Lap', 'maximum_heart_rate', 'bpm'],

    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-Cadence': ['Trackpoint', 'cadence', 'rpm'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-DistanceMeters': ['Trackpoint', 'distance', 'm'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-Extensions-TPX-Speed': ['Trackpoint', 'Speed', 'm/s'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-Extensions-TPX-Watts': ['Trackpoint', 'power', 'watts'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-HeartRateBpm-Value': ['Trackpoint', 'heart_rate', 'bmp'],
    'TrainingCenterDatabase-Activities-Activity-Lap-Track-Trackpoint-Time': ['Trackpoint', 'timestamp', 's']}


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
        start_dir += "/..";

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
