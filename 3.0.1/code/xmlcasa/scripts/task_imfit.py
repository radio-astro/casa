from taskinit import *
#from imregion import *
#from odict import odict
#from math import *

def imfit(
    imagename=None, box=None, region=None, chan=None, stokes=None,
    mask=None, includepix=None, excludepix=None, residual=None,
    model=None, estimates=None, logfile=None, append=True,
    newestimates=None
):
    casalog.origin('imfit')
    try:
        if (not ia.open(imagename)):
            raise Exception, "Cannot create image analysis (ia) tool using " + imagename
        result_dict = ia.fitcomponents(
            box, region, chan, stokes, mask, includepix, excludepix,
            residual, model, estimates, logfile, append, newestimates
        )
        ia.done()
        return result_dict
    except Exception, instance:
        ia.done()
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        return None
        
