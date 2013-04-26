from taskinit import *

def impv(
    imagename, outfile, start, end, width, unit, overwrite,
    region, chans, stokes, mask, stretch
):
    casalog.origin('impv')
    try:
        if len(outfile) == 0:
            raise Exception, "outfile must be specified."
        myia = iatool()
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        myia.pv(
            outfile=outfile, start=start, end=end,
            width=width, unit=unit, overwrite=overwrite, region=region,
            chans=chans, stokes=stokes, mask=mask, stretch=stretch,
            wantreturn=False
        )
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
