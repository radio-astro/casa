from taskinit import *

def imstat(
    imagename, axes, region, box, chans,
    stokes, listit, verbose, mask, stretch,
    logfile, append, algorithm, fence
):
    _myia = iatool()
    _myrg = rgtool()
    _mycs = cstool()
    try:
        casalog.origin('imstat')
        _myia.open(imagename)
        _mycs = _myia.coordsys()
        csrec = _mycs.torecord()
        shape =  _myia.shape()
        reg = _myrg.frombcs(
            csrec, shape,
            box, chans, stokes, "a", region
        )
        return _myia.statistics(
            axes=axes, region=reg, list=listit,
            verbose=verbose, robust=True, mask=mask,
            stretch=stretch, logfile=logfile, append=append,
            algorithm=algorithm, fence=fence
        )
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        _myia.done()
        _myrg.done()
        _mycs.done()
