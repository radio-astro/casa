from taskinit import *

def imfit(
    imagename=None, box=None, region=None, chan=None, stokes=None,
    mask=None, includepix=None, excludepix=None, residual=None,
    model=None, estimates=None, logfile=None, append=True,
    newestimates=None
):
    casalog.origin('imfit')
    myia = iatool.create()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        result_dict = myia.fitcomponents(
            box, region, chan, stokes, mask, includepix, excludepix,
            residual, model, estimates, logfile, append, newestimates
        )
        myia.done()
        return result_dict
    except Exception, instance:
        if (myia):
            myia.done()
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        return None
        
