import os
import re
from taskinit import *
from casac import casac

mysdms, mycb = gentools(['sdms', 'cb'])

def importasap(infile=None, outputvis=None, flagbackup=None, overwrite=None, parallel=None):
    """
    """
    casalog.origin('importasap')

    try:
        if infile is None or outputvis is None:
            raise RuntimeError('Error: infile and outputvis must be specified.')

        # default value
        if flagbackup is None:
            flagbackup = True

        if overwrite is None:
            overwrite = False

        # basic check
        if os.path.exists(outputvis) and not overwrite:
            raise RuntimeError('%s exists.'%(outputvis))
        
        if not is_scantable(infile):
            raise RuntimeError('%s is not a valid Scantable.'%(infile))

        # import
        status = mysdms.importasap(infile, outputvis, parallel)

        if status == True:
            # flagversions file must be deleted 
            flagversions = outputvis.rstrip('/') + '.flagversions'
            if os.path.exists(flagversions):
                os.system('rm -rf %s'%(flagversions))

            # initialize weights using cb tool
            mycb.open(outputvis, compress=False, addcorr=False, addmodel=False)
            mycb.initweights(wtmode='nyq')

            # create flagbackup file if user requests it
            if flagbackup == True:
                aflocal = casac.agentflagger()
                aflocal.open(outputvis)
                aflocal.saveflagversion('Original',
                                        comment='Original flags at import into CASA using importasap',
                                        merge='save')
                aflocal.done()

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
