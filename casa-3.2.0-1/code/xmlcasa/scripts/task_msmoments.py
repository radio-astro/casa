import os
from taskinit import *

def msmoments( msname, moments, antenna, field, spw, includemask, excludemask, outfile, overwrite):
    
    retValue=None
    casalog.origin('msmoments')

    try:
        if ( (not overwrite) and (len(moments) == 1) ):
            if ( os.path.exists(outfile) ):
                raise Exception( outfile+" exists." ) 
        ms.open( msname ) 
        retValue = ms.moments( moments=moments,antenna=antenna,field=field,spw=spw,includemask=includemask,excludemask=excludemask,outfile=outfile,overwrite=overwrite)
	ms.close()
        return retValue
    except Exception, instance:
        ia.done()
	print '*** Error ***',instance
	raise Exception, instance
	    

