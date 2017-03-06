from taskinit import *

def imdev(
    imagename, outfile, region, box, chans,
    stokes, mask, overwrite, stretch,
    grid, anchor, xlength, ylength, interp, stattype, statalg,
    zscore, maxiter
):
    _myia = iatool()
    _myrg = rgtool()
    _mycs = cstool()
    try:
        casalog.origin('imdev')
        _myia.open(imagename)
        _mycs = _myia.coordsys()
        csrec = _mycs.torecord()
        shape =  _myia.shape()
        reg = _myrg.frombcs(
            csrec, shape,
            box, chans, stokes, "a", region
        )
        zz = _myia.makestatimage(
            outfile=outfile, region=reg, mask=mask,
            overwrite=overwrite, stretch=stretch, grid=grid,
            anchor=anchor, xlength=xlength, ylength=ylength,
            interp=interp, stattype=stattype, statalg=statalg,
            zscore=zscore, maxiter=maxiter
        )
        zz.done() 
        return True
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        _myia.done()
        _myrg.done()
        _mycs.done()
