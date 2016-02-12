import os
from taskinit import *

def exportuvfits(
    vis, fitsfile, datacolumn, field, spw, antenna, time,
    avgchan, writesyscal, multisource, combinespw, 
    writestation, padwithflags, overwrite
):
    casalog.origin('exportuvfits')
    try:
        myms = mstool()
        if ((type(vis)==str) & (os.path.exists(vis))):
            myms.open( vis, lock=True )
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        writesyscal=False #until ms syscal table defined
        #start=-1 # redundant, use spw expr
        #nchan=-1 # redundant, use spw expr
        res = myms.tofits(
            fitsfile=fitsfile,
            column=datacolumn,
            field=field, spw=spw,
            baseline=antenna, time=time,
            #start=start,
            #nchan=nchan,
            width=avgchan,
            writesyscal=writesyscal,
            multisource=multisource,
            combinespw=combinespw,
            writestation=writestation,
            padwithflags=padwithflags,
            overwrite=overwrite
        )
        if res:
            return True
        else:
            raise Exception("exportuvfits failed")
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        if myms:
            myms.done()

