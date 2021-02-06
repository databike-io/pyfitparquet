import os, re, time, argparse
from pyfitparquet import fittransformer_so, tcxtransformer


class PyFitParquet:
#{
    def __init__(self):
        self.fit_transformer = fittransformer_so.FitTransformer()
        self.tcx_transformer = tcxtransformer.TcxTransformer()
    
    # Serializes all fit/tcx files in data_dir, outputs into
    # parquet_dir, which defaults to subdirectory within data_dir
    def data_to_parquet(self, data_dir, parquet_dir=None, verbose=1):
    #{
        if not parquet_dir: 
            parquet_dir = f'{data_dir}/parquet'
        try: os.mkdir(parquet_dir)
        except FileExistsError: pass

        for file in os.listdir(data_dir):
        #{
            matchfit = re.match('(\w+).(fit|FIT)$', file)
            if matchfit:
                initial = time.time()
                fitfile = f"{data_dir}/{file}"
                parquetfile = f"{parquet_dir}/{matchfit.group(1)}.parquet"
                if self.fit_to_parquet(fitfile, parquetfile) == 0 and verbose > 0:
                    print(f"Serialized {fitfile} => {parquetfile} in {time.time()-initial:.3f} sec")
                continue

            matchtcx = re.match('(\w+).(tcx|TCX)$', file)            
            if matchtcx: 
                initial = time.time()
                tcxfile = f"{data_dir}/{file}"
                parquetfile = f"{parquet_dir}/{matchtcx.group(1)}.parquet"
                if self.tcx_to_parquet(tcxfile, parquetfile) == 0 and verbose > 0:
                    print(f"Serialized {tcxfile} => {parquetfile} in {time.time()-initial:.3f} sec")
        #}
    #}

    def fit_to_parquet(self, fit_fname, parquet_fname):
        return self.fit_transformer.fit_to_parquet(fit_fname, parquet_fname)
    
    def tcx_to_parquet(self, tcx_fname, parquet_fname):
        return self.tcx_transformer.tcx_to_parquet(tcx_fname, parquet_fname)

    def reset_from_config(self):
        self.fit_transformer.reset_from_config()
        self.tcx_transformer.reset_from_config()
#}


if __name__ == "__main__":
#{
    parser = argparse.ArgumentParser()
    parser.add_argument('DATA_DIR', help='source dir of .fit/.tcx files')
    parser.add_argument('-P', metavar='PARQUET_DIR', help='parquet output dir (defaults: ${DATA_DIR}/parquet')
    args = parser.parse_args()

    PARQUET_DIR = args.P if args.P else f'{args.DATA_DIR}/parquet'
    try: os.mkdir(PARQUET_DIR) 
    except FileExistsError: pass

    pyfitparq = PyFitParquet()
    pyfitparq.data_to_parquet(args.DATA_DIR, PARQUET_DIR)
#}
