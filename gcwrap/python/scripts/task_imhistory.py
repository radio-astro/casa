from taskinit import *

def imhistory(
    imagename, mode, verbose, origin, message
):
    _myia = iatool()
    try:
        casalog.origin('imhistory')
        _myia.open(imagename)
        if mode.startswith("l") or mode.startswith("L"):
            return _myia.history(verbose)
        elif mode.startswith("a") or mode.startswith("A"):
            return _myia.sethistory(origin=origin, history=message)
        raise Exception("Unsopported mode " + mode)
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        _myia.done()
