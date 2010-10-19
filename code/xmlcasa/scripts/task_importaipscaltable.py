import os
from taskinit import *

def importaipscaltable(fitsfile,caltable,extension,whichhdu):
	"""Convert a AIPS calibration table (FITS format) to a CASA calibration table:

          Keyword arguments:
          fitsfile -- Name of input AIPS calibration table file (FITS format)
                  default: none; example='n09m1.tasav.FITS'
	  caltable -- Name of output calibration table file
	          default: none; example='n09m1.tasav.gcal'
	  extension -- Name of the extension table to use inside the AIPS calibration file (without the 'AIPS ')
	          default: 'CL'
  	  whichhdu -- If there is more than one extension of the same name, use this index to indentify it.
	          default: '' = take the last one; example='3'

	"""

	#Python script
	try:
		casalog.origin('importaipscaltable')
		casalog.post("")
		cb.fromaipscal(caltable,fitsfile,extension,whichhdu)
	        # write history
                if ((type(caltable)==str) & (os.path.exists(caltable))):
                        tb.open(caltable+"/CAL_HISTORY",nomodify=False)
                        tb.close()
                else:
                        raise Exception, 'Calibration table was not created.'
		# put code to write history here

	except Exception, instance: 
		print '*** Error ***',instance
		raise Exception, instance


