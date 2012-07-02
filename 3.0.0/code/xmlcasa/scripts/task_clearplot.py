import os
from taskinit import *

def clearplot():
	"""Clear the matplotlib plotter and all layers:

	"""
	try:
		print "Calling tp.clearplot()"
		ok=tp.clearplot()
	except Exception, instance:
		print '*** Error ***',instance
