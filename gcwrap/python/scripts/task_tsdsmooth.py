import numpy
import os
from taskinit import *
import sdutil
ms,sdms,tb = gentools(['ms','sdms','tb'])

def tsdsmooth(infile=None, datacolumn=None, antenna=None, 
              field=None, spw=None, timerange=None, scan=None, 
              pol=None, kernel=None, kwidth=None, outfile=None, overwrite=None):

    casalog.origin('tsdsmooth')

    try:
        casalog.post('This is tsdsmooth!')
        
        if ((os.path.exists(outfile)) and (not overwrite)):
            raise Exception(outfile+' exists.')

        sdms.open(infile)
        sdms.set_selection(spw=spw, field=field, 
                           antenna=antenna,
                           timerange=timerange, scan=scan)
        sdms.smooth(type=kernel, width=kwidth, datacolumn=datacolumn, outfile=outfile)
    except Exception, instance:
        raise Exception, instance
    finally:
        sdms.close()

