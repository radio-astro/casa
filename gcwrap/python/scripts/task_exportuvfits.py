import os
from taskinit import *

def exportuvfits(vis, fitsfile, datacolumn, field, spw, antenna, time,
                 avgchan, writesyscal, multisource, combinespw, 
                 writestation, padwithflags):
	
        casalog.origin('exportuvfits')

	try:
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open( vis, lock=True )
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
                writesyscal=False #until ms syscal table defined
                #start=-1 # redundant, use spw expr
                #nchan=-1 # redundant, use spw expr
		ms.tofits(fitsfile=fitsfile,
                          column=datacolumn,
                          field=field,
                          spw=spw,
                          baseline=antenna,
                          time=time,
                          #start=start,
                          #nchan=nchan,
                          width=avgchan,
                          writesyscal=writesyscal,
                          multisource=multisource,
                          combinespw=combinespw,
                          writestation=writestation,
                          padwithflags=padwithflags)
		ms.close( )
	except Exception, instance:
		raise Exception


