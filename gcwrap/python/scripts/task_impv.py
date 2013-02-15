from taskinit import *

def impv(
    imagename, outfile, start, end, halfwidth, overwrite,
    region, chans, stokes, mask, stretch
):
    casalog.origin('impv')
    
    try:
        if len(outfile) == 0:
            raise Exception, "outfile must be specified."
        myia = iatool()
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        print "**** here 0"

        myia.pv(
            outfile=outfile, start=start, end=end,
            halfwidth=halfwidth, overwrite=overwrite, region=region,
            chans=chans, stokes=stokes, mask=mask, stretch=stretch,
            wantreturn=False
        )
        myia.done()
        print "**** here 1"
        return True
    except Exception, instance:
        print "**** here 2"
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
