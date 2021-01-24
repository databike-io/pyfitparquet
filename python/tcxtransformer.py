import os, re
import pandas as pd
import defusedxml.ElementTree as ET
import loadconfig 

class TcxTransformer:
#{
    INT_VALUE = 1
    FLOAT_VALUE = 2
    STRING_VALUE = 3

    def __init__(self):
    #{
        # TCX source file name/uri
        self.source_filename = None
        self.source_file_uri = None

        # TCX file "primary-ish" key. Note: 
        # several may remain None for TCX
        self.timestamp = None
        self.manufacturer_index = None
        self.manufacturer_name = None
        self.product_index = None
        self.product_name = None
        
        # CONFIG dependant
        self.cbuilders = None
        self.excludeflags = None
        
        # Full list of parquet column names
        self.colkeys =  ["source_filetype", "source_filename", "source_file_uri", 
                         "manufacturer_index", "manufacturer_name", "product_index", 
                         "product_name", "timestamp", "mesg_index", "mesg_name", 
                         "field_index", "field_name", "field_type", "value_string", 
                         "value_integer", "value_float", "units"]
        
        # These tags apply to entire file, thus when parsed, back-apply them to entire builder buffer
        self.file_id_tags = {'TrainingCenterDatabase-Activities-Activity-Creator-Name': self._update_manufacturer_name,
                             'TrainingCenterDatabase-Activities-Activity-Creator-ProductID': self._update_product_index }
    #}
    
    # Serialize TCX file output to parquet
    def tcx_to_parquet(self, tcx_fname, parquet_fname):
    #{
        self.source_filename = os.path.basename(tcx_fname)
        self.source_file_uri = os.path.abspath(tcx_fname)
        self.init_from_config()
        
        # Parse the TCX XML-tree recursively 
        # XML parse requires NO leading whitespace
        with open(tcx_fname) as f_handle:
            xmlstring = f_handle.read().lstrip()
            self.recurse_tree(ET.fromstring(xmlstring).iter())
        
        # Write to parquet file
        table = {ck : self.cbuilders[ck] for ck in self.colkeys if ck in self.cbuilders}
        pd.DataFrame(table).to_parquet(path=parquet_fname, engine='pyarrow')
        self.reset_state()
        return 0
    #}

    # Re-read configuration file (client responsibility when desired)
    def reset_from_config(self):
        loadconfig.populate_config() 
        self.init_from_config()

    # Load from config file
    def init_from_config(self):
        self.cbuilders = {ck : list() for ck in self.colkeys if loadconfig.CONFIG[ck] == "true"}
        self.excludeflags = {ex : loadconfig.CONFIG[ex] == "true" for ex in ["exclude_empty_values", 
                                                                             "exclude_timestamp_values"]}
    
    def reset_state(self):
        self.source_filename = None
        self.source_file_uri = None
        self.timestamp = None
        self.manufacturer_index = None
        self.manufacturer_name = None
        self.product_index = None
        self.product_name = None
        for col in self.cbuilders.keys(): 
            self.cbuilders[col].clear()
    
    # Decompose Clark notation
    def striptag(self, clarkname):
        # Tcx xnode.tag generally includes xmlns, thus appears as: '{xmlns}tagname'
        matchobj = re.match(r'^(\{(?P<xmlns>\w+:.+)\})?(?P<tagname>\w+)', clarkname)
        assert matchobj, f"Unable to appropriately strip element tag: '{clarkname}'"
        return matchobj.group('xmlns'), matchobj.group('tagname')

    # Recursively traverse XML element tree 
    def recurse_tree(self, xiter, compoundtag=None):
    #{
        xnode = next(xiter)
        schema, tagname = self.striptag(xnode.tag)
        if compoundtag is None: compoundtag = tagname
        else: compoundtag += f'-{tagname}'
        nfields = 0
        
        # Append valid attributes
        for attr in xnode.attrib.keys():
            xmlns, attrname = self.striptag(attr)
            attrcomptag = compoundtag + f'-{attrname}'
            nfields += self.append_endpoint(attrcomptag, xnode.attrib[attr])

        # Append valid XML values
        if compoundtag in loadconfig.TAG_FIELD_MAP: 
            nfields += self.append_endpoint(compoundtag, xnode.text.rstrip())

        # Traverse child tags
        for _ in range(len(xnode)): 
            nfields += self.recurse_tree(xiter, compoundtag)

        # Finalize timestamp if finishing mesg block of rows
        if tagname in loadconfig.MESG_TAGS and "timestamp" in self.cbuilders: 
            self.cbuilders["timestamp"].extend([self.timestamp]*nfields)
            return 0
            
        return nfields
    #}
    
    # Append XML comptag-endpoint to column builder arrays
    def append_endpoint(self, comptag, field_value):
    #{    
        if comptag in loadconfig.TAG_FIELD_MAP:
        #{
            # Update manufacturer/product info
            if comptag in self.file_id_tags:
                self.file_id_tags[comptag](field_value)
        
            # Update timestamp
            mesg_name, field_name, units = loadconfig.TAG_FIELD_MAP[comptag]
            if comptag in loadconfig.TIMESTAMP_TAGS: 
                self.timestamp = pd.to_datetime(field_value).tz_localize(None)

            # Append field
            if (comptag not in loadconfig.TIMESTAMP_TAGS and not
                self.excludeflags["exclude_timestamp_values"]):
                self.append_fields(mesg_name, field_name, field_value, units)
                return 1
        #}
        elif field_value and comptag not in loadconfig.TAG_FIELD_EXCLUDES: 
            print(f"WARNING: unmapped {comptag}, w/value: {field_value}")
        
        return 0
    #}
    
    def append_fields(self, mesg_name, field_name, field_value, units=None, mesg_index=None, field_index=None): 
    #{
        if "source_filetype" in self.cbuilders: self.cbuilders["source_filetype"].append("TCX")
        if "source_filename" in self.cbuilders: self.cbuilders["source_filename"].append(self.source_filename)
        if "source_file_uri" in self.cbuilders: self.cbuilders["source_file_uri"].append(self.source_file_uri)
        if "manufacturer_index" in self.cbuilders: self.cbuilders["manufacturer_index"].append(self.manufacturer_index)
        if "manufacturer_name" in self.cbuilders: self.cbuilders["manufacturer_name"].append(self.manufacturer_name)
        if "product_index" in self.cbuilders: self.cbuilders["product_index"].append(self.product_index)
        if "product_name" in self.cbuilders: self.cbuilders["product_name"].append(self.product_name)
        if "mesg_index" in self.cbuilders: self.cbuilders["mesg_index"].append(mesg_index)
        if "mesg_name" in self.cbuilders: self.cbuilders["mesg_name"].append(mesg_name)
        
        if "field_index" in self.cbuilders: self.cbuilders["field_index"].append(field_index)
        if "field_name" in self.cbuilders: self.cbuilders["field_name"].append(field_name)
        if "units" in self.cbuilders: self.cbuilders["units"].append(units)
        
        ftype, ival, fval = self.get_field_type(field_value)
        if "field_type" in self.cbuilders:
            if ftype == TcxTransformer.INT_VALUE: self.cbuilders["field_type"].append("integer")
            elif ftype == TcxTransformer.FLOAT_VALUE: self.cbuilders["field_type"].append("float")
            else: self.cbuilders["field_type"].append("string")
        
        if "value_integer" in self.cbuilders: self.cbuilders["value_integer"].append(ival)
        if "value_float" in self.cbuilders: self.cbuilders["value_float"].append(fval)
        if "value_string" in self.cbuilders: self.cbuilders["value_string"].append(field_value)
    #}
    
    def get_field_type(self, field_value): 
    #{
        try: return TcxTransformer.INT_VALUE, int(field_value), float(field_value)
        except ValueError: pass
        
        try: return TcxTransformer.FLOAT_VALUE, None, float(field_value)
        except ValueError: pass
        
        return TcxTransformer.STRING_VALUE, None, None
    #}

    # See: self.file_id_tags in __init__(self)
    def _update_manufacturer_name(self, value):
        self.manufacturer_name = value
        if self.cbuilders and "manufacturer_name" in self.cbuilders: 
            self.cbuilders["manufacturer_name"] = [value] * len(self.cbuilders["manufacturer_name"])
    
    def _update_product_index(self, value):
        self.product_index = value
        if self.cbuilders and "product_index" in self.cbuilders: 
            self.cbuilders["product_index"] = [value] * len(self.cbuilders["product_index"])
#}