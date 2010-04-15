import os
from taskinit import *

#def msmoments( msname, moments, mask, stokes, mask, includerow, excluderow, outfile, overwrite):
def msmoments( msname, moments, includerow, excluderow, outfile, overwrite):
    
    retValue=None
    casalog.origin('msmoments')

    try:
        if ( (not overwrite) and (len(moments) == 1) ):
            if ( os.path.exists(outfile) ):
                raise Exception( outfile+" exists." ) 
        ms.open( msname ) 
        retValue = ms.moments( moments=moments,includerow=includerow,excluderow=excluderow,outfile=outfile,overwrite=overwrite)
	ms.close()
        return retValue
    except Exception, instance:
        ia.done()
	print '*** Error ***',instance
	raise Exception, instance
	    

