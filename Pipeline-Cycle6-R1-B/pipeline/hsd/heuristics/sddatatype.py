import os
import re
import pipeline.infrastructure.api as api

def is_ms(filename):
    table_info = os.path.join(filename, 'table.info')
    if os.path.exists(filename) and os.path.exists(table_info):
        with open(table_info, 'r') as f:
            line = f.readline()
        return re.match('^Type ?= ?Measurement ?Set *$', line) is not None
    return False

class DataTypeHeuristics(api.Heuristic):
    """
    Examine type of the input data. Data types that are recognizable
    by this heuristics are:

       'MS2'  -- Measurement Set
       'ASDM' -- ASDM
       'FITS' -- ATNF SDFITS 
       'NRO'  -- NRO data format (both NEWSTAR and NOSTAR)

    Otherwise, the heuristics will return 'UNKNOWN'.
    """
    def calculate(self, filename):
        """
        Return data type of the file.

        filename -- name of the file on disk
        """
        datatype = 'UNKNOWN'
        abspath = os.path.realpath(os.path.expanduser(os.path.expandvars(filename)))
        if is_ms(abspath):
            datatype = 'MS2'
        if os.path.isdir(abspath):
            if os.path.exists(os.path.join(abspath,'ASDM.xml')):
                datatype = 'ASDM'
        else:
            # regular file, so examine its contents
            with open(abspath, 'r') as f:
                head = f.read(8)
                if head.upper() == 'SIMPLE  ':
                    datatype = 'FITS'
                elif head.upper() == 'XTENSION':
                    f.seek(10)
                    head = f.read(8)
                    if head.upper() == 'BINTABLE':
                        # it may be NRO FITS data
                        datatype = 'NRO'
                elif head[:2].upper() == 'RW':
                    # it may be NRO OTF data
                    datatype = 'NRO'
        return datatype
                
