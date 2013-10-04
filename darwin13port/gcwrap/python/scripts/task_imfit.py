from taskinit import *

def imfit(
    imagename=None, box=None, region=None, chans=None, stokes=None,
    mask=None, includepix=None, excludepix=None, residual=None,
    model=None, estimates=None, logfile=None, append=None,
    newestimates=None, complist=None, overwrite=None, dooff=None,
    offset=None, fixoffset=None, stretch=None
):
    casalog.origin('imfit')
    myia = iatool()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        result_dict = myia.fitcomponents(
            box=box, region=region, chans=chans, stokes=stokes,
            mask=mask, includepix=includepix, excludepix=excludepix,
            residual=residual, model=model, estimates=estimates,
            logfile=logfile, append=append, newestimates=newestimates,
            complist=complist, overwrite=overwrite, dooff=dooff,
            offset=offset, fixoffset=fixoffset, stretch=stretch
        )
        myia.done()
        return result_dict
    except Exception, instance:
        if (myia):
            myia.done()
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise instance
        
