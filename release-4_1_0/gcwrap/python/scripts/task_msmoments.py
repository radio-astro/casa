import os
from taskinit import *

def msmoments( infile, moments, antenna, field, spw, includemask, excludemask, outfile, overwrite):
    
    retms=None
    casalog.origin('msmoments')

    try:
        if ( (not overwrite) and (len(moments) == 1) ):
            if ( os.path.exists(outfile) ):
                raise Exception( outfile+" exists." ) 
        ms.open( infile ) 
        retms = ms.moments( moments=moments,antenna=antenna,field=field,spw=spw,includemask=includemask,excludemask=excludemask,outfile=outfile,overwrite=overwrite)
	ms.close()
        retms.close()
    except Exception, instance:
        ms.close()
        if retms: retms.close()
	print '*** Error ***',instance
	raise Exception, instance
	    

