import os
from taskinit import *

from parameter_check import *

def clearstat():
	"""Clear all read/write locks on tables. This can be used if a task has
	indicated that it is trying to get a lock on a file.

	"""
	try:
		tb.clearlocks()
	except Exception, instance:
		print '*** Error ***',instance
