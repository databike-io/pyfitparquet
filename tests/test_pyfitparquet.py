import pandas as pd
import os, re, shutil, random, unittest, yaml, pyarrow
from pyfitparquet import transformer, loadconfig

class TestSerialization(unittest.TestCase):
#{
    parquet_files = []
    PARQUET_DIR = None

    @classmethod
    def setUpClass(cls):
    #{
        print("\nSetting up TestSerialization")

        DATA_DIR = os.path.join(os.getcwd(), 'fixtures')
        cls.PARQUET_DIR = os.path.join(DATA_DIR, 'parquet')
        if os.path.isdir(cls.PARQUET_DIR):
            shutil.rmtree(cls.PARQUET_DIR)
        os.mkdir(cls.PARQUET_DIR) 

        # Create sorted list of expected parquet outfiles
        for file in sorted(os.listdir(DATA_DIR)):
            matchfit = re.match('(\w+).(fit|FIT)$', file)
            matchtcx = re.match('(\w+).(tcx|TCX)$', file)      
            if matchfit: cls.parquet_files.append(os.path.join(cls.PARQUET_DIR, f"{matchfit.group(1)}.parquet"))
            elif matchtcx: cls.parquet_files.append(os.path.join(cls.PARQUET_DIR, f"{matchtcx.group(1)}.parquet"))
        #}

        # Serialize all files in DATA_DIR to parquet
        print("Serializing FIT/TCX files...")
        pyfitparq = transformer.PyFitParquet()
        pyfitparq.data_to_parquet(DATA_DIR, verbose=0)
        print("Testing Parquet file integrity...")
    #}

    @classmethod
    def tearDownClass(cls):
        cls.parquet_files.clear()
        if os.path.isdir(cls.PARQUET_DIR): shutil.rmtree(cls.PARQUET_DIR)

    def test_number_rows(self):
        numb_rows = [len(pd.read_parquet(pfile, engine='pyarrow')) 
                     for pfile in self.parquet_files]
        self.assertEqual(numb_rows, [6610,5180,8570,360356,20766,
            36399,22909,5412,11111,11506,769,8114,11506])

    def test_mean_power(self):
    #{
        mean_power, fnames = [], []
        for pfile in self.parquet_files:
            df = pd.read_parquet(pfile, engine='pyarrow')
            pseries = df[df["field_name"] == "power"]["value_float"]
            if len(pseries) > 0: 
                mean_power.append(round(pseries.mean()))
                fnames.append(os.path.basename(pfile))
                
        self.assertEqual(mean_power, [142,132,119,151,222,202,211])
        self.assertEqual(fnames, ['45_min_HIIT_Hills_Ride.parquet', 
            '45_min_Power_Zone_Ride.parquet', '60_min_Power_Zone_Endurance_Ride.parquet', 
            'Bolt_GPS.parquet', 'TeamScream.parquet', 'Who_Dares_Bolt.parquet', 
            'Who_Dares_Sufferfest.parquet'])
    #}

    def test_heart_rate(self):
    #{
        min_max_mean, fnames = [], []
        for pfile in self.parquet_files:
            df = pd.read_parquet(pfile, engine='pyarrow')
            hrseries = df[df["field_name"] == "heart_rate"]["value_float"]
            if len(hrseries) > 0:
                min_max_mean.append((int(hrseries.min()), 
                    int(hrseries.max()), round(hrseries.mean())))
                fnames.append(os.path.basename(pfile))

        self.assertEqual(min_max_mean, [(95,160,142), (90,167,139), (93,148,134), 
            (93,177,145), (81,199,178), (99,194,156), (64,194,157), (82,173,148), 
            (97,202,168),(85,170,141),(85,170,141)])
        self.assertEqual(fnames, ['45_min_HIIT_Hills_Ride.parquet', 
            '45_min_Power_Zone_Ride.parquet', '60_min_Power_Zone_Endurance_Ride.parquet', 
            'Bolt_GPS.parquet', 'TeamScream.parquet', 'Who_Dares_Bolt.parquet', 
            'Who_Dares_Sufferfest.parquet', 'Who_Dares_Whoop.parquet', 
            'activity_4564516081.parquet', 'activity_607442311.parquet', 
            'twin_cities_marathon.parquet'])
    #}

    def test_distance(self):
    #{
        dist_sum, fnames = [], []
        for pfile in self.parquet_files:
            df = pd.read_parquet(pfile, engine='pyarrow')
            dseries = df[df["field_name"] == "distance"]["value_float"]
            if len(dseries) > 0: 
                dist_sum.append(round(dseries.sum()))
                fnames.append(os.path.basename(pfile))

        self.assertEqual(dist_sum, [14031176,8773236,23598187,1479425507,
            79033557,46914931,25597485,45513934,653587,67940563,45513934])
        self.assertEqual(fnames, ['45_min_HIIT_Hills_Ride.parquet', 
            '45_min_Power_Zone_Ride.parquet', '60_min_Power_Zone_Endurance_Ride.parquet', 
            'Bolt_GPS.parquet', 'TeamScream.parquet', 'Who_Dares_Sufferfest.parquet', 
            'activity_4564516081.parquet', 'activity_607442311.parquet', 
            'dav_track_5k.parquet', 'new_river_50k.parquet', 
            'twin_cities_marathon.parquet'])
    #}
#}

class TestConfiguration(unittest.TestCase):
#{
    fittcx_files = []
    PARQUET_DIR = None
    NCOLUMN_TRIALS = 25
    parquet_config_local = None
    mapping_config_local = None
    conda_config_install = None
    conda_prefix = None

    @classmethod
    def setUpClass(cls):
    #{
        print("\nSetting up TestConfiguration")

        DATA_DIR = os.path.join(os.getcwd(), 'fixtures')
        cls.PARQUET_DIR = os.path.join(DATA_DIR, 'parquet')
        if os.path.isdir(cls.PARQUET_DIR): shutil.rmtree(cls.PARQUET_DIR)
        os.mkdir(cls.PARQUET_DIR) 

        # Create list of FIT/TCX datafiles
        for file in sorted(os.listdir(DATA_DIR)):
            matchfit = re.match('(\w+).(fit|FIT)$', file)
            matchtcx = re.match('(\w+).(tcx|TCX)$', file)            
            if matchfit or matchtcx: cls.fittcx_files.append(os.path.join(DATA_DIR, file))

        # Setup starting environment variable settings        
        assert 'CONDA_PREFIX' in os.environ, "ERROR: CONDA_PREFIX must be set"
        assert os.getenv('CONDA_DEFAULT_ENV') == 'pyfitenv', "ERROR: conda pyfitenv must be activated"
        parquet_config_base = loadconfig.find_file("parquet_config.yml", os.environ['CONDA_PREFIX'])
        mapping_config_base = loadconfig.find_file("mapping_config.yml", os.environ['CONDA_PREFIX'])
        assert parquet_config_base and mapping_config_base, "ERROR: no config in pyfitparquet install" 

        cls.conda_prefix = os.environ['CONDA_PREFIX']
        cls.conda_config_install = os.path.dirname(parquet_config_base)
        if 'PYFIT_CONFIG_DIR' in os.environ: del os.environ['CONDA_PREFIX']

        # Setup starting local config file settings
        cls.parquet_config_local = os.path.join(os.getcwd(), 'parquet_config.yml')
        cls.mapping_config_local = os.path.join(os.getcwd(), 'mapping_config.yml')
        if os.path.isfile(cls.parquet_config_local): os.remove(cls.parquet_config_local)
        if os.path.isfile(cls.mapping_config_local): os.remove(cls.mapping_config_local)
    #}

    @classmethod
    def tearDownClass(cls):
        if os.path.isdir(cls.PARQUET_DIR): shutil.rmtree(cls.PARQUET_DIR)
        if os.path.isfile(cls.parquet_config_local): os.remove(cls.parquet_config_local)
        if os.path.isfile(cls.mapping_config_local): os.remove(cls.mapping_config_local)
        local_yml_dir = os.path.dirname(cls.parquet_config_local)
        for ymlfile in os.listdir(local_yml_dir):
            if re.match(r'^parquet_config\.yml\.\d+$', ymlfile):
                os.remove(os.path.join(local_yml_dir, ymlfile))

        if 'PYFIT_CONFIG_DIR' in os.environ: del os.environ['PYFIT_CONFIG_DIR']
        os.environ['CONDA_PREFIX'] = cls.conda_prefix
    
    def test_env_variable_config(self):
    #{
        # (1) Input: base case, CONDA_PREFIX set, PYFIT_CONFIG_DIR not set, no local files
        # Expected result: loadconfig.config_dir == pyfitparquet dir of CONDA install
        if 'PYFIT_CONFIG_DIR' in os.environ: del os.environ['PYFIT_CONFIG_DIR']
        if os.path.isfile(self.parquet_config_local): os.remove(self.parquet_config_local)
        if os.path.isfile(self.mapping_config_local): os.remove(self.mapping_config_local)
        loadconfig.populate_config() 

        self.assertEqual(loadconfig.config_dir, self.conda_config_install)

        # (2) Input: CONDA_PREFIX not set, PYFIT_CONFIG_DIR not set, no local files
        # Expected result: assertion error, can't find config_files
        del os.environ['CONDA_PREFIX']
        with self.assertRaises(AssertionError):
            loadconfig.populate_config()
        
        self.assertIsNone(loadconfig.config_dir)
        self.assertFalse(os.path.isfile(self.parquet_config_local))
        self.assertFalse(os.path.isfile(self.mapping_config_local))

        # (3) Input: CONDA_PREFIX not set, PYFIT_CONFIG_DIR is set, no local files
        # Expected result: assertion error, can't find config_files
        os.environ['PYFIT_CONFIG_DIR'] = os.getcwd()
        with self.assertRaises(AssertionError):
            loadconfig.populate_config()
        
        self.assertIsNone(loadconfig.config_dir)
        self.assertFalse(os.path.isfile(self.parquet_config_local))
        self.assertFalse(os.path.isfile(self.mapping_config_local))

        # (4) Input: CONDA_PREFIX set, PYFIT_CONFIG_DIR set, no local files
        # Expected result: loadconfig.config_dir == PYFIT_CONFIG_DIR, files copied in
        os.environ['CONDA_PREFIX'] = self.conda_prefix
        loadconfig.populate_config()

        self.assertEqual(loadconfig.config_dir, os.environ['PYFIT_CONFIG_DIR'])
        self.assertTrue(os.path.isfile(self.parquet_config_local))
        self.assertTrue(os.path.isfile(self.mapping_config_local))

        # (5) Input: CONDA_PREFIX not set, PYFIT_CONFIG_DIR set, local files exist
        # Expected result: loadconfig.config_dir == PYFIT_CONFIG_DIR
        del os.environ['CONDA_PREFIX']
        loadconfig.populate_config()
        self.assertEqual(loadconfig.config_dir, os.environ['PYFIT_CONFIG_DIR'])        
    #}

    def _read_parquet_config(self, parquet_config):
        with open(parquet_config) as pconfig_fhandle:
            pconfig_map = yaml.safe_load(pconfig_fhandle)
            return pconfig_map

    def _write_parquet_config(self, pconfig_map, parquet_config, i):
        shutil.move(parquet_config, f'{parquet_config}.{i}')
        with open('parquet_config.yml.tmp', 'w') as write_fhandle:
            yaml.safe_dump(pconfig_map, write_fhandle)
        shutil.move('parquet_config.yml.tmp', parquet_config)

    def _randomize_pconfig_map(self, pconfig_map, columns):
        rtb = random.getrandbits(len(columns))
        rcol_config = {columns[i]: True if rtb & (1 << i) else False for i in range(len(columns))}
        return {k: rcol_config[k] if k in rcol_config else pconfig_map[k] for k in pconfig_map.keys()}

    def test_column_config(self):
    #{
        # random.seed(2)
        print(f"Testing {self.NCOLUMN_TRIALS} randomized column configurations...")
        columns = ['source_filetype', 'source_filename', 'source_file_uri', 'manufacturer_index',
           'manufacturer_name', 'product_index', 'product_name', 'timestamp', 'mesg_index',
           'mesg_name', 'field_index', 'field_name', 'field_type', 'value_string',
           'value_integer', 'value_float', 'units']

        colset = set(columns)
        if os.path.isfile(self.parquet_config_local): os.remove(self.parquet_config_local)
        if os.path.isfile(self.mapping_config_local): os.remove(self.mapping_config_local)
        os.environ['PYFIT_CONFIG_DIR'] = os.getcwd()
        pyfitparq = transformer.PyFitParquet()

        for i in range(self.NCOLUMN_TRIALS):
        #{
            # Read current parquet_config.yml and generate expected parquet columns
            pconfig_map = self._read_parquet_config(self.parquet_config_local)
            expected = sorted([k for k in pconfig_map.keys() if k in colset and pconfig_map[k]])

            # Serialize a randomly chosen FIT/TCX file from fixtures
            source_uri = random.choice(self.fittcx_files)
            parquet_uri = pyfitparq.source_to_parquet(source_uri, self.PARQUET_DIR)

            # Read and verify columns in the new parquet file
            df = pd.read_parquet(parquet_uri, engine='pyarrow')
            shutil.move(parquet_uri, f'{parquet_uri}.{i}')
            self.assertEqual(sorted(df.columns), expected)
            self.assertTrue(len(df) > 0)

            # Randomly reconfigure parquet_config.yml and reset state
            pconfig_map = self._randomize_pconfig_map(pconfig_map, columns)
            self._write_parquet_config(pconfig_map, self.parquet_config_local, i)
            pyfitparq.reset_from_config()
        #}
    #}
#}

if __name__ == '__main__':
    unittest.main()