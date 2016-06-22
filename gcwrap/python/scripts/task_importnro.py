import datetime
import os
import re
import shutil
from taskinit import *
import sdutil
mysdms, mycb = gentools(['sdms', 'cb'])

def importnro(infile=None, outputvis=None, overwrite=None, parallel=None):
    """
    """
    casalog.origin('importnro')
    status = True

    try:
        outputvis_temp = outputvis + '-backup-' + datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
        
        if os.path.exists(outputvis):
            if overwrite:
                os.rename(outputvis, outputvis_temp)
            else:
                raise RuntimeError('%s exists.'%(outputvis))
        
        if not is_nostar(infile):
            raise RuntimeError('%s is not a valid NOSTAR data.'%(infile))

        status = mysdms.importnro(infile, outputvis, parallel)

        if status:
            # initialize weights using cb tool
            mycb.open(outputvis, compress=False, addcorr=False, addmodel=False)
            mycb.initweights(wtmode='nyq')
            if os.path.exists(outputvis_temp):
                shutil.rmtree(outputvis_temp)
        else:
            if os.path.exists(outputvis):
                shutil.rmtree(outputvis)
            if os.path.exists(outputvis_temp):
                os.rename(outputvis_temp, outputvis)
            raise RuntimeError('import failed.')
        
        return status
    except Exception, instance:
        casalog.post('*** Error *** ' + str(instance), 'SEVERE')
        raise instance
    finally:
        if status: 
            mycb.close()


def is_nostar(filename):
    """
    Check if given data is NOSTAR or not
    """
    ret = False
    if os.path.getsize(filename) >= 15136: # size of observation header
        with open(filename, 'rb') as f:
            if f.read(8).replace('\x00', '') == 'RW':
                ret = f.read(15136-8+4)[-4:].replace('\x00', '') == 'LS'
            f.close()

    return ret
