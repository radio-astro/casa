import os
from taskinit import *

def listhistory(vis=None):
	"""List the processing history of a dataset:
	The list of all task processing steps will be
	given in the logger.

	Keyword arguments:
	vis -- Name of input visibility file (MS)
		default: none; example: vis='ngc5921.ms'

	"""
	#Python script
	try:
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		ms.listhistory()
		ms.close()
	except Exception, instance:
		print '*** Error ***',instance
