#
# This file was generated using xslt from its XML file
#
# Copyright 2007, Associated Universities Inc., Washington DC
#
import os
from taskinit import *

def uvsub(vis=None,reverse=False):

        """Subtract model from the corrected visibility data

        uvsub(vis='ngc5921.ms', reverse=false)

        This function subtracts model visibility data from corrected visibility
        data leaving the residuals in the corrected data column.  If the
        parameter 'reverse' is set true, the process is reversed.

        Keyword arguments:
        vis -- Name of input visibility file (MS)
                default: none; example: vis='ngc5921.ms'
        reverse -- Reverse the operation (add rather than subtract)
                default: false; example: reverse=true

        uvsub(vis='ngc5921.ms', reverse=false)
 
        """

	#Python script
	#
	try:
		casalog.origin('uvsub')
		if ((type(vis)==str) & (os.path.exists(vis))):
			ms.open(thems=vis,nomodify=False)
		else:
			raise Exception, 'Visibility data set not found - please verify the name'
		        return
		ms.uvsub(reverse)
		ms.close()
		return
	except Exception, instance:
		print '*** Error ***',instance
		return
