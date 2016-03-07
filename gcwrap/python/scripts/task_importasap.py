import os
import re
from taskinit import *
import sdutil
mysdms, mycb = gentools(['sdms', 'cb'])

def importasap(infile, outputvis, overwrite):
    """
    """
    casalog.origin('importasap')

    try:
        if os.path.exists(outputvis) and not overwrite:
            raise RuntimeError('%s exists.'%(outputvis))
        
        if not is_scantable(infile):
            raise RuntimeError('%s is not a valid Scantable.'%(infile))

        status = mysdms.importasap(infile, outputvis)
        
        # initialize weights using cb tool
        mycb.open(outputvis, compress=False, addcorr=False, addmodel=False)
        mycb.initweights(wtmode='nyq')

        return status
    except Exception, instance:
        casalog.post('*** Error *** ' + str(instance), 'SEVERE')
        raise instance
    finally:
        mycb.close()


def is_scantable(filename):
    """
    Check if given data is Scantable or not
    """
    ret = False
    if os.path.isdir(filename) and os.path.exists(filename+'/table.info') \
        and os.path.exists(filename+'/table.dat'):
        with open(filename+'/table.info') as f:
            l=f.readline()
            f.close()
        match_pattern = '^Type = (Scantable)? *$'
        ret = re.match(match_pattern, l) is not None
    return ret
