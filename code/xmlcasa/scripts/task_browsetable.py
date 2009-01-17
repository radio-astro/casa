import os
from taskinit import *

def browsetable(tablename=None):
	""" Browse a table (visibility data set, calibration table, or image):

	Brings up a java brower that can open and display any CASA table

	Keyword arguments:
    	tablename -- Name of table file on disk (MS, calibration table, image)
		default: none; example: tablename='ngc5921.ms'

	"""
	#Python script
	try:
		if (type(tablename)==str):
		   if (os.path.exists(tablename)):
                        tb.open(tablename)
		   tb.browse()
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
	except Exception, instance:
		print '*** Error ***',instance
