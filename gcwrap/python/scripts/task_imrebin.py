from taskinit import *

def imrebin(
    imagename, outfile, factor, region, box, chans, stokes, mask,
    dropdeg, overwrite, stretch
):
    casalog.origin('imrebin')
    valid = True
    # because there is a bug in the tasking layer that allows float
    # arrays through when the spec is for intArray
    for x in factor:
        if x != int(x):
            valid = False
            break
    if not valid:
        for i in range(len(factor)):
            factor[i] = int(factor[i])   
        casalog.post(
            "factor is not an int array, it will be adjusted to "
                + str(factor),
            'WARN'
        )
    myia = iatool()
    outia = None
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        if (len(outfile) == 0):
            raise Exception, "outfile must be specified."
        if (type(region) != type({})):
            region = rg.frombcs(
                csys=myia.coordsys().torecord(), shape=myia.shape(), box=box,
                chans=chans, stokes=stokes, stokescontrol="a", region=region
            )
        outia = myia.rebin(
            outfile=outfile, bin=factor, region=region, mask=mask, dropdeg=dropdeg,
            overwrite=overwrite, stretch=stretch
        )
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if myia:
            myia.done()
        if outia:
            outia.done()
        
