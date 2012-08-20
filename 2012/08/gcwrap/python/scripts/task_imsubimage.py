from taskinit import *

def imsubimage(
    imagename=None, outfile=None, region=None,
    mask=None, dropdeg=None, overwrite=None, verbose=None,
    stretch=None, wantreturn=None
):
    casalog.origin('imsubimage')
    myia = iatool()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        outia = myia.subimage(
            outfile=outfile, region=region, mask=mask, dropdeg=dropdeg,
            overwrite=overwrite, list=verbose, stretch=stretch
        )
        myia.done()
        if (wantreturn):
            return outia
        else:
            outia.done()
            return True
    except Exception, instance:
        if (myia):
            myia.done()
        if (outia):
            outia.done()
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        return False
        
