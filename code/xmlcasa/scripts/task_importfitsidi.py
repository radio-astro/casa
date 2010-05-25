import os
from taskinit import *

def importfitsidi(fitsidifile,vis):
	"""Convert FITS-IDI visibility file into a CASA visibility file (MS).

	Keyword arguments:
	fitsidifile -- Name of input FITS IDI file
		default: None; example='3C273XC1.IDI'
	vis -- Name of output visibility file (MS)
		default: None; example: vis='3C273XC1.ms'
		
	"""

	#Python script
	try:
		casalog.origin('importfitsidi')
		casalog.post("")
		ms.fromfitsidi(vis,fitsidifile)
		ms.close()
	        # write history
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open(vis,nomodify=False)
                else:
                        raise Exception, 'Output visibility file not created; cannot write to its history table.'
        	ms.writehistory(message='taskname     = importfitsidi',origin='importfitsidi')
        	ms.writehistory(message='fitsidifile  = "'+str(fitsidifile)+'"',origin='importfitsidi')
        	ms.writehistory(message='vis          = "'+str(vis)+'"',origin='importfitsidi')
        	ms.close()

	except Exception, instance: 
		print '*** Error ***',instance
		raise Exception, instance


