import os
from taskinit import *

def exportuvfits(vis,fitsfile,datacolumn,field,spw,antenna,time,nchan,start,width,writesyscal,multisource,combinespw,writestation):
	
        casalog.origin('exportuvfits')

	try:
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open( vis, lock=True )
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		ms.tofits(fitsfile=fitsfile,column=datacolumn,field=field,spw=spw, baseline=antenna, time=time,start=start,nchan=nchan,width=width,writesyscal=writesyscal,multisource=multisource,combinespw=combinespw,writestation=writestation)
		ms.close( )
	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance


