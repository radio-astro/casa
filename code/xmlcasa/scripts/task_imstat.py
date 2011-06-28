from taskinit import *

def imstat(
    imagename=None, axes=None, region=None, box=None, chans=None,
    stokes=None, listit=None, robust=None, verbose=None
):
    _myia = iatool.create()
    try:
        casalog.origin('imstat')
        _myia.open(imagename)
        mycsys = _myia.coordsys()
        reg = rg.frombcs(
            mycsys.torecord(), _myia.shape(),
            box, chans, stokes, "a", region
        )
        retValue = _myia.statistics(
            axes=axes, region=reg, list=listit,
            robust=robust, verbose=verbose
        )
        _myia.done()
        return retValue
    except Exception, instance:
        _myia.done()
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        return {}
