import os, re, time, argparse
from pyfitparquet import fittransformer_so, tcxtransformer


class PyFitParquet:
#{
    def __init__(self):
        self.fit_transformer = fittransformer_so.FitTransformer()
        self.tcx_transformer = tcxtransformer.TcxTransformer()
        self.reset_from_config()
    
    def reset_from_config(self):
        self.fit_transformer.reset_from_config()
        self.tcx_transformer.reset_from_config()

    # Serializes all fit/tcx files in data_dir, outputs into
    # parquet_dir, which defaults to subdirectory within data_dir
    def data_to_parquet(self, data_dir, parquet_dir=None, verbose=1):
    #{
        if parquet_dir is None: parquet_dir = os.path.join(data_dir, 'parquet')
        assert os.path.isdir(parquet_dir) or not os.path.exists(parquet_dir), f'ERROR: {parquet_dir}' 
        if not os.path.exists(parquet_dir): os.mkdir(parquet_dir)

        for file in os.listdir(data_dir):
            initial, source_uri = time.time(), os.path.join(data_dir, file)
            parquet_uri = self.source_to_parquet(source_uri, parquet_dir)
            if parquet_uri and verbose > 0: print(f"Serialized {source_uri} =>",
                f"{parquet_uri} in {time.time()-initial:.3f} sec")
    #}

    # Serializes a single source file at source_uri to parquet
    def source_to_parquet(self, source_uri, parquet_dir=None):
        source_fname = os.path.basename(source_uri)
        if re.match('(\w+).(fit|FIT)$', source_fname):
            return self.fit_to_parquet(source_uri, parquet_dir)
        elif re.match('(\w+).(tcx|TCX)$', source_fname):
            return self.tcx_to_parquet(source_uri, parquet_dir)
        else: return None

    # Serializes a single FIT file at fit_uri to parquet
    def fit_to_parquet(self, fit_uri, parquet_dir=None):
        parquet_uri = self.create_parquet_uri(fit_uri, parquet_dir)
        status = self.fit_transformer.fit_to_parquet(fit_uri, parquet_uri)
        return parquet_uri if status == 0 else None

    # Serializes a single TCX file at tcx_uri to parquet    
    def tcx_to_parquet(self, tcx_uri, parquet_dir=None):
        parquet_uri = self.create_parquet_uri(tcx_uri, parquet_dir)
        status = self.tcx_transformer.tcx_to_parquet(tcx_uri, parquet_uri)
        return parquet_uri if status == 0 else None

    # Returns name like source_fname but extension replaced with .parquet
    # If parquet_dir is None, directory path of source_uri is used
    def create_parquet_uri(self, source_uri, parquet_dir=None):
        sfroot, ext = os.path.splitext(os.path.basename(source_uri))
        if parquet_dir is None: parquet_dir = os.path.dirname(source_uri)
        return os.path.join(parquet_dir, f'{sfroot}.parquet')
#}

if __name__ == "__main__":
#{
    parser = argparse.ArgumentParser()
    parser.add_argument('SOURCE_FILE', help='source .fit or .tcx file')
    parser.add_argument('-P', metavar='PARQUET_DIR', help='parquet output dir (defaults: ${DATA_DIR}/parquet')
    args = parser.parse_args()

    PyFitParquet().source_to_parquet(args.SOURCE_FILE, args.P if args.P else None)
#}
