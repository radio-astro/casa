from taskinit import *

def imfit(
    imagename, box, region, chans, stokes,
    mask, includepix, excludepix, residual,
    model, estimates, logfile, append,
    newestimates, complist, overwrite, dooff,
    offset, fixoffset, stretch, rms
):
    casalog.origin('imfit')
    myia = iatool()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        result_dict = myia.fitcomponents(
            box=box, region=region, chans=chans, stokes=stokes,
            mask=mask, includepix=includepix,
            excludepix=excludepix, residual=residual,
            model=model, estimates=estimates, logfile=logfile,
            append=append, newestimates=newestimates,
            complist=complist, overwrite=overwrite, dooff=dooff,
            offset=offset, fixoffset=fixoffset, stretch=stretch, rms=rms
        )
        return result_dict
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise instance
    finally:
        myia.done()
        
