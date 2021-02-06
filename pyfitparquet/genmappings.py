import re, argparse
from collections import defaultdict
import defusedxml.ElementTree as ET

class GenMappings:
#{
    def __init__(self):
        self.xmltypes = None
        self.elements = None

    def __repr__(self):
        outstr = f'Elements:\n'
        for k in self.elements: outstr += f'  {k} : {self.elements[k]}\n'

        outstr += f'\nComplex Types:\n'
        for k in self.xmltypes: outstr += f'  {k} : {self.xmltypes[k]}\n'
        return outstr

    def parse_xsd_file(self, xsd_fname):
        self.elements = {}
        self.xmltypes = defaultdict(list)
        with open(xsd_fname) as f_handle:
            xmlstring = f_handle.read().lstrip()
            self.recurse_tree(ET.fromstring(xmlstring).iter())
    
    def create_mappings(self):
        print('TRAINING_CENTER_DATABASE_XSD_DEFAULT_ENDPOINT_MAPPINGS = {')
        rootelements = [k for k in self.elements if re.match(r'^(\w+)$', k)]
        for elem in rootelements: self.recurse_mappings(self.elements[elem], elem)
        print('}')

    def recurse_mappings(self, etype, compoundtag):
    #{
        if etype not in self.xmltypes:
            tags = compoundtag.split('-')
            mesg_name = tags[2] if len(tags) > 3 else tags[-2]
            units = None

            # Convert from camel-case to lowercase w/underscores
            icaps = [i for i, ltr in enumerate(tags[-1]) if ltr.isupper()] + [len(tags[-1])]
            parts = [tags[-1][icaps[j-1]:icaps[j]].lower() for j in range(1, len(icaps))]
            
            # Pull some obvious default unit types from the field name
            if parts[-1] in ['bpm', 'degrees']: units = f"'{parts[-1]}'"; parts.pop()
            elif parts[-1] in ['seconds']: units = f"'s'"; parts.pop()
            elif parts[-1] in ['meters']: units = f"'m'"; parts.pop()
            elif parts[-1] in ['calories']: units = f"'kcal'"
            elif parts[-1] in ['cadence']: units = f"'rpm'"
            elif parts[-1] in ['speed']: units = f"'m/s'"
            
            field_name = '_'.join(parts)
            if field_name == 'time': field_name = 'timestamp'
            print(f"    '{compoundtag}' : ['{mesg_name}', '{field_name}', {units}],")
            return

        # Traverse elements down to simpleTypes
        for elem in self.xmltypes[etype]:
            elemtypeless = elem.split('-')[-1]
            # Do NOT traverse down (infinite) recursive complexType schema loops
            if elemtypeless not in compoundtag.split('-'): self.recurse_mappings(
                self.elements[elem], f'{compoundtag}-{elemtypeless}')
    #}

    # Decompose Clark notation
    def striptag(self, clarkname):
        # Tcx xnode.tag generally includes xmlns, thus appears as: '{xmlns}tagname'
        matchobj = re.match(r'^(\{(?P<xmlns>\w+:.+)\})?(?P<tagname>[:\w]+)', clarkname)
        assert matchobj, f"Unable to appropriately strip element tag: '{clarkname}'"
        return matchobj.group('xmlns'), matchobj.group('tagname')

    # Recursively traverse XML element tree 
    def recurse_tree(self, xiter, complexType=None, compoundtag=None):
    #{
        xnode = next(xiter)
        schema, tagname = self.striptag(xnode.tag)

        if tagname == 'complexType':
            xmlns, comptype = self.striptag(xnode.attrib['name'])
            complexType = comptype

        if tagname == 'element' or tagname == 'attribute':
            xmlns, elemname = self.striptag(xnode.attrib['name'])
            xmlns, elemtype = self.striptag(xnode.attrib['type'])
            if not complexType: self.elements[elemname] = elemtype
            else:
                self.elements[compelemname] = elemtype
                self.xmltypes[complexType].append(compelemname)
        
        for _ in range(len(xnode)): 
            self.recurse_tree(xiter, complexType, compoundtag)
    #}
#}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('XSD_FILE', help='XML/TCX schema file')
    args = parser.parse_args()

    gm = GenMappings()
    gm.parse_xsd_file(args.XSD_FILE)
    gm.create_mappings()
