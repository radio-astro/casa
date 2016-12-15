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
        if avgchan != 1:
            casalog.post(
                "WARNING: The avgchan parameter has been deprecated. It no longer "
                + "functions (and it is questionable if it actually ever did) and will "
                + "be removed in a future version. Run mstransform prior to exportuvfits "
                + "to select and average data before writing it to UVFITS format", 'WARN'
            )
        res = myms.tofits(
            fitsfile=fitsfile,
            column=datacolumn,
            field=field, spw=spw,
            baseline=antenna, time=time,
            # width=avgchan,
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

