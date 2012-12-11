from taskinit import *

def impv(
    imagename, outfile, start, end, halfwidth, overwrite,
    region, chans, stokes, mask, stretch, wantreturn
):
    casalog.origin('impv')
    myia = iatool()
    mypv = None
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        mypv = myia.pv(
            outfile=outfile, start=start, end=end,
            halfwidth=halfwidth, overwrite=overwrite, region=region,
            chans=chans, stokes=stokes, mask=mask, stretch=stretch,
            wantreturn=wantreturn
        )
        if (wantreturn):
            return mypv
        else:
            mypv.done()
            return False
    except Exception, instance:
        if (mypv):
            mypv.done()
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
