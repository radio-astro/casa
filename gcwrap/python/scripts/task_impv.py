from taskinit import *

def impv(
    imagename, outfile, mode, start, end, center, length, pa, width,
    unit, overwrite, region, chans, stokes, mask, stretch
):
    casalog.origin('impv')
    try:
        if len(outfile) == 0:
            raise Exception, "outfile must be specified."
        mymode = mode.lower()
        if mymode.startswith('c'):
            if len(start) == 0 or len(end) == 0:
                raise Exception, "When mode='coords', start and end must both be specified."
            center = ""
            length = ""
            pa = ""
        elif mymode.startswith('l'):
            if (
                len(center) == 0 
                or (
                    not isinstance(length, (int, long, float))
                    and len(length) == 0
                )
                or len(pa) == 0
            ):
                raise Exception, "When mode='length', center, length, and pa must all be specified."
            start = ""
            end = ""
        else:
            raise Exception, "Unsupported value for mode."
        myia = iatool()
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        myia.pv(
            outfile=outfile, start=start, end=end, center=center,
            length=length, pa=pa, width=width, unit=unit,
            overwrite=overwrite, region=region, chans=chans,
            stokes=stokes, mask=mask, stretch=stretch, wantreturn=False
        )
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
